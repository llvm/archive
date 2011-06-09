//===---------- TypeChecksOpt.h - Remove safe runtime type checks ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass removes type checks that are statically proven safe
//
//===----------------------------------------------------------------------===//

#include "assistDS/TypeChecksOpt.h"
#include "llvm/Constants.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/ADT/Statistic.h"

#include <set>
#include <vector>

using namespace llvm;

char TypeChecksOpt::ID = 0;

static RegisterPass<TypeChecksOpt> 
TC("typechecks-opt", "Remove safe runtime type checks", false, true);

// Pass statistics
STATISTIC(numSafe,  "Number of statically proven safe type checks");

static const Type *VoidTy = 0;
static const Type *Int8Ty = 0;
static const Type *Int32Ty = 0;
static const Type *Int64Ty = 0;
static const PointerType *VoidPtrTy = 0;
static Constant *trackGlobal;
static Constant *trackStringInput;
static Constant *trackArray;
static Constant *trackInitInst;
static Constant *trackStoreInst;
static Constant *trackLoadInst;
static Constant *copyTypeInfo;

bool TypeChecksOpt::runOnModule(Module &M) {
  bool modified = false; // Flags whether we modified the module.
  TS = &getAnalysis<dsa::TypeSafety<TDDataStructures> >();

  // Create the necessary prototypes
  VoidTy = IntegerType::getVoidTy(M.getContext());
  Int8Ty = IntegerType::getInt8Ty(M.getContext());
  Int32Ty = IntegerType::getInt32Ty(M.getContext());
  Int64Ty = IntegerType::getInt64Ty(M.getContext());
  VoidPtrTy = PointerType::getUnqual(Int8Ty);

  trackGlobal = M.getOrInsertFunction("trackGlobal",
                                      VoidTy,
                                      VoidPtrTy,/*ptr*/
                                      Int8Ty,/*type*/
                                      Int64Ty,/*size*/
                                      Int32Ty,/*tag*/
                                      NULL);
  trackArray = M.getOrInsertFunction("trackArray",
                                     VoidTy,
                                     VoidPtrTy,/*ptr*/
                                     Int64Ty,/*size*/
                                     Int64Ty,/*count*/
                                     Int32Ty,/*tag*/
                                     NULL);
  trackInitInst = M.getOrInsertFunction("trackInitInst",
                                        VoidTy,
                                        VoidPtrTy,/*ptr*/
                                        Int64Ty,/*size*/
                                        Int32Ty,/*tag*/
                                        NULL);
  trackStoreInst = M.getOrInsertFunction("trackStoreInst",
                                         VoidTy,
                                         VoidPtrTy,/*ptr*/
                                         Int8Ty,/*type*/
                                         Int64Ty,/*size*/
                                         Int32Ty,/*tag*/
                                         NULL);
  trackLoadInst = M.getOrInsertFunction("trackLoadInst",
                                        VoidTy,
                                        VoidPtrTy,/*ptr*/
                                        Int8Ty,/*type*/
                                        Int64Ty,/*size*/
                                        Int32Ty,/*tag*/
                                        NULL);
  copyTypeInfo = M.getOrInsertFunction("copyTypeInfo",
                                       VoidTy,
                                       VoidPtrTy,/*dest ptr*/
                                       VoidPtrTy,/*src ptr*/
                                       Int64Ty,/*size*/
                                       Int32Ty,/*tag*/
                                       NULL);
  trackStringInput = M.getOrInsertFunction("trackStringInput",
                                           VoidTy,
                                           VoidPtrTy,
                                           Int32Ty,
                                           NULL);

  for(Value::use_iterator User = trackGlobal->use_begin(); User != trackGlobal->use_end(); ++User) {
    CallInst *CI = dyn_cast<CallInst>(User);
    assert(CI);
    
    if(TS->isTypeSafe((CI->getOperand(1)->stripPointerCasts(), CI->getParent()->getParent()))) {
      toDelete.push_back(CI);
    }
  }

  for(Value::use_iterator User = trackLoadInst->use_begin(); User != trackLoadInst->use_end(); ++User) {
    CallInst *CI = dyn_cast<CallInst>(User);
    assert(CI);
    
    if(TS->isTypeSafe(CI->getOperand(1)->stripPointerCasts(), CI->getParent()->getParent())) {
      toDelete.push_back(CI);
    }
  }

  for(Value::use_iterator User = trackStoreInst->use_begin(); User != trackStoreInst->use_end(); ++User) {
    CallInst *CI = dyn_cast<CallInst>(User);
    assert(CI);
    
    if(TS->isTypeSafe(CI->getOperand(1)->stripPointerCasts(), CI->getParent()->getParent())) {
      toDelete.push_back(CI);
    }
  }

  for(Value::use_iterator User = trackInitInst->use_begin(); User != trackInitInst->use_end(); ++User) {
    CallInst *CI = dyn_cast<CallInst>(User);
    assert(CI);
    
    if(TS->isTypeSafe(CI->getOperand(1)->stripPointerCasts(), CI->getParent()->getParent())) {
      toDelete.push_back(CI);
    }
  }

  for(Value::use_iterator User = trackArray->use_begin(); User != trackArray->use_end(); ++User) {
    CallInst *CI = dyn_cast<CallInst>(User);
    assert(CI);
    
    if(TS->isTypeSafe(CI->getOperand(1)->stripPointerCasts(), CI->getParent()->getParent())) {
      toDelete.push_back(CI);
    }
  }
  for(Value::use_iterator User = copyTypeInfo->use_begin(); User != copyTypeInfo->use_end(); ++User) {
    CallInst *CI = dyn_cast<CallInst>(User);
    assert(CI);
    
    if(TS->isTypeSafe(CI->getOperand(1)->stripPointerCasts(), CI->getParent()->getParent())) {
      toDelete.push_back(CI);
      continue;
    }
    if(TS->isTypeSafe(CI->getOperand(2)->stripPointerCasts(), CI->getParent()->getParent())) {
      std::vector<Value*> Args;
      Args.push_back(CI->getOperand(1));
      Args.push_back(CI->getOperand(3)); // size
      Args.push_back(CI->getOperand(4));
      CallInst::Create(trackInitInst, Args.begin(), Args.end(), "", CI);
      toDelete.push_back(CI);
    }
  }

  numSafe += toDelete.size();
  
  while(!toDelete.empty()) {
    Instruction *I = toDelete.back();
    toDelete.pop_back();
    I->eraseFromParent();
  }

  return modified;
}


