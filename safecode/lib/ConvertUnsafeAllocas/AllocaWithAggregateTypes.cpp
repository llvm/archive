#include "ConvertUnsafeAllocas.h"
#include "SCUtils.h"

#include "llvm/Pass.h"
#include "llvm/BasicBlock.h"
#include "llvm/Type.h"
#include "llvm/Function.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Constants.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/VectorExtras.h"

#include <iostream>

using namespace llvm;

static Statistic<> InitAllocas ("convallocai", "Allocas Initialized");

#ifdef LLVA_KERNEL
static const unsigned meminitvalue = 0x00;
#else
static const unsigned meminitvalue = 0xcc;
#endif

  // Create the command line option for the pass
  //  RegisterOpt<MallocPass> X ("malloc", "Alloca to Malloc Pass");

  inline bool MallocPass::TypeContainsPointer(const Type *Ty) {
    if (Ty->getTypeID() == Type::PointerTyID)
      return true;
    else if (Ty->getTypeID() == Type::StructTyID) {
      const StructType * structTy = cast<StructType>(Ty);
      unsigned numE = structTy->getNumElements();
      for (unsigned index = 0; index < numE; index++) {
	if (TypeContainsPointer(structTy->getElementType(index)))
	  return true;
      }
    } else if (Ty->getTypeID() == Type::ArrayTyID) {
      const ArrayType *arrayTy = cast<ArrayType>(Ty);
      if (TypeContainsPointer(arrayTy->getElementType()))
	return true;
    }
    return false;
  }

  inline bool
  MallocPass::changeType (Instruction * Inst)
  {
    //
    // Check to see if the instruction is an alloca.
    //
    if (Inst->getOpcode() == Instruction::Alloca) {
      AllocationInst * AllocInst = cast<AllocationInst>(Inst);
      
      //
      // Get the type of object allocated.
      //
      const Type * TypeCreated = AllocInst->getAllocatedType ();
      
      if (TypeContainsPointer(TypeCreated))
	return true;
      
    }
    return false;
  }

  bool
  MallocPass::runOnFunction (Function &F)
  {
    bool modified = false;
    Type * VoidPtrType = PointerType::get(Type::SByteTy);

    TargetData &TD = getAnalysis<TargetData>();
    Module *theM = F.getParent();  
    Function *memsetF = 
      theM->getOrInsertFunction("memset", Type::VoidTy, 
				PointerType::get(Type::SByteTy), Type::IntTy , 
				Type::UIntTy, NULL);
    for (Function::iterator I = F.begin(), E = F.end(); I != E; ++I) {
      for (BasicBlock::iterator IAddrBegin=I->begin(), IAddrEnd = I->end();
           IAddrBegin != IAddrEnd;
           ++IAddrBegin) {
        //
        // Determine if the instruction needs to be changed.
        //
        if (changeType (IAddrBegin)) {
          AllocationInst * AllocInst = cast<AllocationInst>(IAddrBegin);
#if 0
          //
          // Get the type of object allocated.
          //
          const Type * TypeCreated = AllocInst->getAllocatedType ();

          MallocInst * TheMalloc = 
          new MallocInst(TypeCreated, 0, AllocInst->getName(), 
          IAddrBegin);
          std::cerr << "Found alloca that is struct or array" << std::endl;

          //
          // Remove old uses of the old instructions.
          //
          AllocInst->replaceAllUsesWith (TheMalloc);

          //
          // Remove the old instruction.
          //
          AllocInst->getParent()->getInstList().erase(AllocInst);
          modified = true;
#endif
          // Insert object registration at the end of allocas.
          Instruction *iptI = AllocInst->getNext();
          if (AllocInst->getParent() == (&(AllocInst->getParent()->getParent()->getEntryBlock()))) {
            BasicBlock::iterator InsertPt = AllocInst->getParent()->begin();
            while (&(*(InsertPt)) != AllocInst)
              ++InsertPt;
            while (isa<AllocaInst>(InsertPt))
              ++InsertPt;
            iptI = InsertPt;
          }

          // Create a value that calculates the alloca's size
          const Type * AllocaType = AllocInst->getAllocatedType();
          Value *AllocSize = ConstantInt::get (Type::UIntTy,
                                               TD.getTypeSize(AllocaType));

          if (AllocInst->isArrayAllocation())
            AllocSize = BinaryOperator::create(Instruction::Mul, AllocSize,
                                               AllocInst->getOperand(0),
                                               "sizetmp",
                                               AllocInst);

          Value * TheAlloca = castTo (AllocInst, VoidPtrType, iptI);

          std::vector<Value *> args(1, TheAlloca);
          args.push_back (ConstantInt::get(Type::IntTy, meminitvalue));
          args.push_back (AllocSize);
          new CallInst(memsetF, args, "", iptI);
          ++InitAllocas;
        }
      }
    }
    return modified;
  }

namespace {
  RegisterPass<MallocPass> Z("convallocai", "converts unsafe allocas to mallocs");
} // end of namespace

