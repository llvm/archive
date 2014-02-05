#include <dlfcn.h>

#include "vmkit/system.h"
#include "vmkit/compiler.h"
#include "vmkit/thread.h"
#include "vmkit/vmkit.h"
#include "vmkit/safepoint.h"
#include "vmkit/inliner.h"

#include "llvm/LinkAllPasses.h"
#include "llvm/PassManager.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/JIT.h"

#include "llvm/Bitcode/ReaderWriter.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "llvm/Target/TargetOptions.h"

using namespace vmkit;

void* Symbol::getSymbolAddress() {
	Thread::get()->vm()->internalError("implement me: getSymbolAddress");
}

uint64_t Symbol::inlineWeight() {
	if(cachedWeight)
		return cachedWeight;

	if(llvmFunction()->size() > 4)
		return cachedWeight = (uint64_t)-1;

	uint64_t weight = 0;

	for (llvm::Function::iterator bit=llvmFunction()->begin(); bit!=llvmFunction()->end(); bit++)
		weight += bit->size();

	weight = weight * 256;

	return cachedWeight = weight;
}

void Symbol::markAsNeverInline() {
	if(llvmFunction())
		llvmFunction()->addFnAttr(llvm::Attribute::NoInline);
}

void* CompilationUnit::operator new(size_t n, BumpAllocator* allocator) {
	return allocator->allocate(n);
}

void  CompilationUnit::operator delete(void* self) {
}

CompilationUnit::CompilationUnit(BumpAllocator* allocator, const char* id, bool runInlinePass, bool onlyAlwaysInline) :
	_symbolTable(vmkit::Util::char_less, allocator) {
	_allocator = allocator;
	pthread_mutex_init(&_mutexSymbolTable, 0);

	std::string err;

	llvm::TargetOptions opt;
	opt.NoFramePointerElim = 1;

	_ee = llvm::EngineBuilder(new llvm::Module(id, Thread::get()->vm()->llvmContext()))
		.setUseMCJIT(1)
		.setMCJITMemoryManager(this)
		.setOptLevel(llvm::CodeGenOpt::None) /* Aggressive */
		.setTargetOptions(opt)
		.setErrorStr(&err)
		.create();

	if (!ee())
		Thread::get()->vm()->internalError("Error while creating execution engine: %s\n", err.c_str());

	ee()->finalizeObject();

	pm = new llvm::PassManager();
	//pm->add(new llvm::TargetData(*ee->getTargetData()));

#if 0
	pm->add(llvm::createBasicAliasAnalysisPass());
#endif

	if(runInlinePass || onlyAlwaysInline)
		pm->add(vmkit::createFunctionInlinerPass(this, onlyAlwaysInline));

#if 0
	pm->add(llvm::createCFGSimplificationPass());      // Clean up disgusting code

	pm->add(llvm::createPromoteMemoryToRegisterPass());// Kill useless allocas
	pm->add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
	pm->add(llvm::createScalarReplAggregatesPass()); // Break up aggregate allocas
	pm->add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
	pm->add(llvm::createJumpThreadingPass());        // Thread jumps.
	pm->add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
	pm->add(llvm::createInstructionCombiningPass()); // Combine silly seq's
	pm->add(llvm::createCFGSimplificationPass());    // Merge & remove BBs

	pm->add(llvm::createReassociatePass());          // Reassociate expressions
	pm->add(llvm::createLoopRotatePass());           // Rotate loops.
	pm->add(llvm::createLICMPass());                 // Hoist loop invariants
	pm->add(llvm::createLoopUnswitchPass());         // Unswitch loops.
	pm->add(llvm::createInstructionCombiningPass());
	pm->add(llvm::createIndVarSimplifyPass());       // Canonicalize indvars
	pm->add(llvm::createLoopDeletionPass());         // Delete dead loops
	pm->add(llvm::createLoopUnrollPass());           // Unroll small loops*/
	pm->add(llvm::createInstructionCombiningPass()); // Clean up after the unroller
	pm->add(llvm::createGVNPass());                  // Remove redundancies
	pm->add(llvm::createMemCpyOptPass());            // Remove memcpy / form memset
	pm->add(llvm::createSCCPPass());                 // Constant prop with SCCP

	// Run instcombine after redundancy elimination to exploit opportunities
	// opened up by them.
	pm->add(llvm::createInstructionCombiningPass());
	pm->add(llvm::createJumpThreadingPass());         // Thread jumps
	pm->add(llvm::createDeadStoreEliminationPass());  // Delete dead stores
	pm->add(llvm::createAggressiveDCEPass());         // Delete dead instructions
	pm->add(llvm::createCFGSimplificationPass());     // Merge & remove BBs
#endif
}

CompilationUnit::~CompilationUnit() {
	delete pm;
	delete _ee;
}

void CompilationUnit::destroy(CompilationUnit* unit) {
	delete unit;
	BumpAllocator::destroy(unit->allocator());
}

void CompilationUnit::addSymbol(const char* id, vmkit::Symbol* symbol) {
	pthread_mutex_lock(&_mutexSymbolTable);
	_symbolTable[id] = symbol;
	pthread_mutex_unlock(&_mutexSymbolTable);
}

Symbol* CompilationUnit::getSymbol(const char* id, bool error) {
	pthread_mutex_lock(&_mutexSymbolTable);

	std::map<const char*, vmkit::Symbol*>::iterator it = _symbolTable.find(id);
	vmkit::Symbol* res = 0;

	if(it == _symbolTable.end()) {
		uint8_t* addr = (uint8_t*)dlsym(SELF_HANDLE, id);
		//fprintf(stderr, "lookup: %s => %p\n", id, addr);
		if(!addr) {
			if(error)
				Thread::get()->vm()->internalError("unable to resolve native symbol: %s", id);
		} else {
			res = new(allocator()) vmkit::NativeSymbol(0, addr);
			size_t len = strlen(id);
			char* buf = (char*)allocator()->allocate(len+1);
			memcpy(buf, id, len+1);
			_symbolTable[buf] = res;
		}
	} else
		res = it->second;

	pthread_mutex_unlock(&_mutexSymbolTable);

	return res;
}

uint64_t CompilationUnit::getSymbolAddress(const std::string &Name) {
	return (uint64_t)(uintptr_t)getSymbol(System::mcjitSymbol(Name.c_str()))->getSymbolAddress();
}

void CompilationUnit::prepareModule(llvm::Module* module) {
	pm->run(*module);
}

void CompilationUnit::compileModule(llvm::Module* module) {
	ee()->addModule(module);
	ee()->finalizeObject();

	vmkit::Safepoint* sf = Safepoint::get(this, module);
	VMKit* vm = Thread::get()->vm();

	if(!sf)
		vm->internalError("unable to find safepoints");
		
	while(sf->addr()) {
		sf->setUnit(this);
		vm->addSafepoint(sf);

		//Thread::get()->vm()->getSafepoint(sf->addr())->dump();
		sf = sf->getNext();
	}
}
