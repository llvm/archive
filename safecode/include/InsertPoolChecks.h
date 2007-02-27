#ifndef INSERT_BOUNDS_H
#define INSERT_BOUNDS_H

#include "safecode/Config/config.h"
#include "llvm/Pass.h"
#include "ConvertUnsafeAllocas.h"

#ifndef LLVA_KERNEL
#include "SafeDynMemAlloc.h"
#include "poolalloc/PoolAllocate.h"
#endif

namespace llvm {

ModulePass *creatInsertPoolChecks();
using namespace CUA;

struct InsertPoolChecks : public ModulePass {
    public :
    const char *getPassName() const { return "Inserting pool checks for array bounds "; }
    virtual bool runOnModule(Module &M);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<ConvertUnsafeAllocas>();
//      AU.addRequired<CompleteBUDataStructures>();
//      AU.addRequired<TDDataStructures>();
#ifndef LLVA_KERNEL      
      AU.addRequired<EquivClassGraphs>();
      AU.addRequired<PoolAllocate>();
      AU.addRequired<EmbeCFreeRemoval>();
      AU.addRequired<TargetData>();
#else 
      AU.addRequired<TDDataStructures>();
      AU.addRequired<TargetData>();
#endif
      
    };
    private :
    CUA::ConvertUnsafeAllocas * cuaPass;
  TargetData * TD;
#ifndef  LLVA_KERNEL
  PoolAllocate * paPass;
  EquivClassGraphs *equivPass;
  EmbeCFreeRemoval *efPass;
#else
  TDDataStructures * TDPass;
#endif  
  Function *PoolCheck;
  Function *PoolCheckArray;
  Function *PoolCheckIArray;
  Function *ExactCheck;
  Function *FunctionCheck;
  Function *BoundsCheck;
  Function *ExactCheck2;
  Function *GetActualValue;
  Function *PoolRegister;
  void addPoolCheckProto(Module &M);
  void addPoolChecks(Module &M);
  void addGetElementPtrChecks(Module &M);
  DSNode* getDSNode(const Value *V, Function *F);
  unsigned getDSNodeOffset(const Value *V, Function *F);
  void addLoadStoreChecks(Module &M);
  void TransformFunction(Function &F);
  void handleCallInst(CallInst *CI);
  void handleGetElementPtr(GetElementPtrInst *MAI);
  void addGetActualValue(SetCondInst *SCI, unsigned operand);
  void registerAllocaInst(AllocaInst *AI, AllocaInst *AIOrig);
  void registerGlobalArraysWithGlobalPools(Module &M);
  
#ifndef LLVA_KERNEL  
  void addLSChecks(Value *Vnew, const Value *V, Instruction *I, Function *F);
  Value * getPoolHandle(const Value *V, Function *F, PA::FuncInfo &FI, bool collapsed = false);
#else
  void addLSChecks(Value *V, Instruction *I, Function *F);
  Value * getPoolHandle(const Value *V, Function *F);
#endif  

};
}
#endif
