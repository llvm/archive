//===-- HLVM to LLVM Code Generator -----------------------------*- C++ -*-===//
//
//                      High Level Virtual Machine (HLVM)
//
// Copyright (C) 2006 Reid Spencer. All Rights Reserved.
//
// This software is free software; you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published by 
// the Free Software Foundation; either version 2.1 of the License, or (at 
// your option) any later version.
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for 
// more details.
//
// You should have received a copy of the GNU Lesser General Public License 
// along with this library in the file named LICENSE.txt; if not, write to the 
// Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301 USA
//
//===----------------------------------------------------------------------===//
/// @file hlvm/CodeGen/LLVM/LLVMGenerator.cpp
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the implementation of the HLVM -> LLVM Code Generator
//===----------------------------------------------------------------------===//

#include <hlvm/CodeGen/LLVM/LLVMGenerator.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Import.h>
#include <hlvm/AST/Documentation.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Variable.h>
#include <hlvm/AST/Program.h>
#include <hlvm/AST/Block.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/Constants.h>
#include <hlvm/Base/Assert.h>
#include <hlvm/Pass/Pass.h>
#include <llvm/Module.h>
#include <llvm/BasicBlock.h>
#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/DerivedTypes.h>
#include <llvm/TypeSymbolTable.h>
#include <llvm/Constants.h>
#include <llvm/Linker.h>
#include <llvm/Bytecode/Writer.h>
#include <llvm/PassManager.h>
#include <llvm/Assembly/PrintModulePass.h>

using namespace hlvm;

namespace 
{

class LLVMGeneratorPass : public Pass
{
  public:
    LLVMGeneratorPass(const AST* tree)
      : Pass(0,Pass::PreAndPostOrderTraversal),
      modules(), lmod(0), lfunc(0), lblk(0), linst(), lops(), ltypes(),
      ast(tree),   bundle(0), function(0), block(0) { }
    ~LLVMGeneratorPass() { }

  /// Conversion functions
  inline const llvm::Type* getType(const Type* ty);
  inline llvm::GlobalValue::LinkageTypes getLinkageTypes(LinkageKinds lk);
  inline std::string getLinkageName(LinkageItem* li);

  /// Generators
  inline void gen(Bundle* b);
  inline void gen(AliasType* t);
  inline void gen(AnyType* t);
  inline void gen(BooleanType* t);
  inline void gen(CharacterType* t);
  inline void gen(IntegerType* t);
  inline void gen(RangeType* t);
  inline void gen(EnumerationType* t);
  inline void gen(RealType* t);
  inline void gen(OctetType* t);
  inline void gen(VoidType* t);
  inline void gen(PointerType* t);
  inline void gen(ArrayType* t);
  inline void gen(VectorType* t);
  inline void gen(StructureType* t);
  inline void gen(SignatureType* t);
  inline void gen(ConstLiteralInteger* t);
  inline void gen(Variable* v);
  inline void gen(Function* f);
  inline void gen(Program* p);
  inline void gen(Block* f);
  inline void gen(ReturnOp* f);

  virtual void handle(Node* n,Pass::TraversalKinds mode);

  inline llvm::Module* linkModules();

private:
  typedef std::vector<llvm::Module*> ModuleList;
  typedef std::vector<llvm::Value*> OperandList;
  ModuleList modules;        ///< The list of modules we construct
  llvm::Module*     lmod;    ///< The current module we're generation 
  llvm::Function*   lfunc;   ///< The current LLVM function we're generating 
  llvm::BasicBlock* lblk;    ///< The current LLVM block we're generating
  llvm::BasicBlock::InstListType linst; 
  OperandList lops;          ///< The current list of instruction operands
  llvm::TypeSymbolTable ltypes; ///< The cached LLVM types we've generated
    ///< The current LLVM instructions we're generating
  const AST* ast;            ///< The current Tree we're traversing
  const Bundle* bundle;      ///< The current Bundle we're traversing
  const Function* function;  ///< The current Function we're traversing
  const Block* block;        ///< The current Block we're traversing
};


const llvm::Type*
LLVMGeneratorPass::getType(const Type* ty)
{
  // First, lets see if its cached already
  const llvm::Type* result = ltypes.lookup(ty->getName());
  if (result)
    return result;

  // Okay, we haven't seen this type before so let's construct it
  switch (ty->getID()) {
    case SignatureTypeID:
    {
      std::vector<const llvm::Type*> params;
      const SignatureType* st = llvm::cast<SignatureType>(ty);
      for (SignatureType::const_iterator I = st->begin(), E = st->end(); 
           I != E; ++I)
        params.push_back(getType(*I));
      result = llvm::FunctionType::get(
        getType(st->getResultType()),params,st->isVarArgs());
      break;
    }
    case VoidTypeID: result = llvm::Type::VoidTy; break;
    case BooleanTypeID: result = llvm::Type::BoolTy; break;
    case CharacterTypeID: result = llvm::Type::UShortTy; break;
    case OctetTypeID: result = llvm::Type::UByteTy; break;
    case UInt8TypeID: result = llvm::Type::UByteTy; break;
    case UInt16TypeID: result = llvm::Type::UShortTy; break;
    case UInt32TypeID: result = llvm::Type::UIntTy; break;
    case UInt64TypeID: result = llvm::Type::ULongTy; break;
    case UInt128TypeID: 
      hlvmNotImplemented("128 bit primitive integer");
      break;
    case SInt8TypeID: result = llvm::Type::SByteTy; break;
    case SInt16TypeID: result = llvm::Type::ShortTy; break;
    case SInt32TypeID: result = llvm::Type::IntTy; break;
    case SInt64TypeID: result = llvm::Type::LongTy; break;
    case SInt128TypeID: 
      hlvmNotImplemented("128 bit primitive integer");
      break;
    case Float32TypeID: result = llvm::Type::FloatTy; break;
    case Float64TypeID: result = llvm::Type::FloatTy; break;
    case Float44TypeID: 
    case Float80TypeID: 
    case Float128TypeID: 
      hlvmNotImplemented("extended and quad floating point");
      break;
    case IntegerTypeID:
      hlvmNotImplemented("arbitrary precision integer");
      break;
    case RealTypeID:
      hlvmNotImplemented("arbitrary precision real");
    case StringTypeID: {
      std::vector<const llvm::Type*> Fields;
      Fields.push_back(llvm::Type::UIntTy);
      Fields.push_back(llvm::PointerType::get(llvm::Type::UShortTy));
      result = llvm::StructType::get(Fields);
      break;
    }
    case AliasTypeID: {
      result = getType(llvm::cast<AliasType>(ty)->getType());
      break;
    }
    case PointerTypeID: {
      result = llvm::PointerType::get(
        getType(llvm::cast<PointerType>(ty)->getTargetType()));
      break;
    }
    case ArrayTypeID: {
      const llvm::Type* elemType = 
        getType(llvm::cast<ArrayType>(ty)->getElementType());
      std::vector<const llvm::Type*> Fields;
      Fields.push_back(llvm::Type::UIntTy);
      Fields.push_back(llvm::PointerType::get(elemType));
      result = llvm::StructType::get(Fields);
      break;
    }
    default:
      hlvmDeadCode("Invalid type code");
      break;
  }
  if (result)
    ltypes.insert(ty->getName(),result);
  return result;
}

void 
LLVMGeneratorPass::gen(AliasType* t)
{
}
void 
LLVMGeneratorPass::gen(AnyType* t)
{
}

void
LLVMGeneratorPass::gen(BooleanType* t)
{
}

void
LLVMGeneratorPass::gen(CharacterType* t)
{
}

void
LLVMGeneratorPass::gen(IntegerType* t)
{
}

void
LLVMGeneratorPass::gen(RangeType* t)
{
}

void 
LLVMGeneratorPass::gen(EnumerationType* t)
{
}

void
LLVMGeneratorPass::gen(RealType* t)
{
}

void
LLVMGeneratorPass::gen(OctetType* t)
{
}

void
LLVMGeneratorPass::gen(VoidType* t)
{
}

void 
LLVMGeneratorPass::gen(PointerType* t)
{
}

void 
LLVMGeneratorPass::gen(ArrayType* t)
{
}

void 
LLVMGeneratorPass::gen(VectorType* t)
{
}

void 
LLVMGeneratorPass::gen(StructureType* t)
{
}

void 
LLVMGeneratorPass::gen(SignatureType* t)
{
}

void 
LLVMGeneratorPass::gen(ConstLiteralInteger* i)
{
  const Type* hType = i->getType();
  const llvm::Type* lType = getType(hType);
  if (llvm::cast<IntegerType>(hType)->isSigned())
    lops.push_back(llvm::ConstantSInt::get(lType,i->getValue()));
  else
    lops.push_back(llvm::ConstantUInt::get(lType,i->getValue(0)));
}

void
LLVMGeneratorPass::gen(Variable* v)
{
}

void 
LLVMGeneratorPass::gen(Block* b)
{
  lblk = new llvm::BasicBlock(b->getLabel(),lfunc,0);
}

void
LLVMGeneratorPass::gen(ReturnOp* r)
{
  llvm::ReturnInst* ret = 0;
  if (lops.empty())
    ret =  new llvm::ReturnInst(lblk);
  else {
    hlvmAssert(lops.size() == 1 && "Too many operands for ReturnInst");
    llvm::Value* retVal = lops[0];;
    const llvm::Type* retTy = retVal->getType();
    if (retTy != lfunc->getReturnType()) {
      retVal = new llvm::CastInst(retVal,lfunc->getReturnType(),"",lblk);
    }
    ret = new llvm::ReturnInst(retVal,lblk);
    lops.clear();
  }
  // RetInst is never the operand of another instruction (Terminator)
}

llvm::GlobalValue::LinkageTypes
LLVMGeneratorPass::getLinkageTypes(LinkageKinds lk)
{
  return llvm::GlobalValue::LinkageTypes(lk);
}

std::string
LLVMGeneratorPass::getLinkageName(LinkageItem* lk)
{
  if (lk->isProgram())
    return std::string("_hlvm_entry_") + lk->getName();
  // FIXME: This needs to incorporate the bundle name
  return lk->getName();
}

void
LLVMGeneratorPass::gen(Function* f)
{
  lfunc = new llvm::Function(
    llvm::cast<llvm::FunctionType>(getType(f->getSignature())),
    getLinkageTypes(f->getLinkageKind()), 
    getLinkageName(f), lmod);
}

void
LLVMGeneratorPass::gen(Bundle* b)
{
  lmod = new llvm::Module(b->getName());
  modules.push_back(lmod);
}

void
LLVMGeneratorPass::handle(Node* n,Pass::TraversalKinds mode)
{
  if (mode == Pass::PreOrderTraversal) {
    // We process container nodes here (preorder) to ensure that we create the
    // container that is being asked for.
    switch (n->getID()) {
    case BundleID:                gen(llvm::cast<Bundle>(n)); break;
    case FunctionID:              gen(llvm::cast<Function>(n)); break;
    case ProgramID:               gen(llvm::cast<Function>(n)); break;
    case BlockID:                 gen(llvm::cast<Block>(n)); break;
    default:
      break;
    }
  } else {
    // We process non-container nodes and operators. Operators are done
    // post-order because we want their operands to be constructed first.
    switch (n->getID()) 
    {
    case AliasTypeID:             gen(llvm::cast<AliasType>(n)); break;
    case AnyTypeID:               gen(llvm::cast<AnyType>(n)); break;
    case BooleanTypeID:           gen(llvm::cast<BooleanType>(n)); break;
    case CharacterTypeID:         gen(llvm::cast<CharacterType>(n)); break;
    case IntegerTypeID:           gen(llvm::cast<IntegerType>(n)); break;
    case RangeTypeID:             gen(llvm::cast<RangeType>(n)); break;
    case EnumerationTypeID:       gen(llvm::cast<EnumerationType>(n)); break;
    case RealTypeID:              gen(llvm::cast<RealType>(n)); break;
    case OctetTypeID:             gen(llvm::cast<OctetType>(n)); break;
    case VoidTypeID:              gen(llvm::cast<VoidType>(n)); break;
    case PointerTypeID:           gen(llvm::cast<PointerType>(n)); break;
    case ArrayTypeID:             gen(llvm::cast<ArrayType>(n)); break;
    case VectorTypeID:            gen(llvm::cast<VectorType>(n)); break;
    case StructureTypeID:         gen(llvm::cast<StructureType>(n)); break;
    case SignatureTypeID:         gen(llvm::cast<SignatureType>(n)); break;
    case ConstLiteralIntegerOpID: gen(llvm::cast<ConstLiteralInteger>(n));break;
    case VariableID:              gen(llvm::cast<Variable>(n)); break;
    case ReturnOpID:              gen(llvm::cast<ReturnOp>(n)); break;
    default:
      break;
    }
  }
}


llvm::Module*
LLVMGeneratorPass::linkModules()
{
  llvm::Linker linker("HLVM",ast->getPublicID(),0);
  for (ModuleList::iterator I = modules.begin(), E = modules.end(); I!=E; ++I) {
    linker.LinkInModule(*I);
  }
  modules.empty(); // LinkInModules destroyed/merged them all
  return linker.releaseModule();
}

}

void
hlvm::generateBytecode(AST* tree,std::ostream& output)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  llvm::Module* mod = genPass.linkModules();
  llvm::WriteBytecodeToFile(mod, output, /*compress= */ true);
  delete mod;
  delete PM;
}

void
hlvm::generateAssembly(AST* tree, std::ostream& output)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  llvm::Module* mod = genPass.linkModules();
  llvm::PassManager Passes;
  Passes.add(new llvm::PrintModulePass(&output));
  Passes.run(*mod);
  delete mod;
  delete PM;
}
