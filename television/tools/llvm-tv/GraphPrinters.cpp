//===- GraphPrinters.cpp - DOT printers for various graph types -----------===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file defines several printers for various different types of graphs used
// by the LLVM infrastructure.  It uses the generic graph interface to convert
// the graph into a .dot graph.  These graphs can then be processed with the
// "dot" tool to convert them to postscript or some other suitable format.
//
//===----------------------------------------------------------------------===//

#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/Value.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/DataStructure/DataStructure.h"
#include "llvm/Analysis/DataStructure/DSGraph.h"
#include "llvm/Support/GraphWriter.h"
#include <fstream>
using namespace llvm;

template<typename GraphType>
static void WriteGraphToFile(std::ostream &O, const std::string &GraphName,
                             const GraphType &GT) {
  std::string Filename = GraphName + ".dot";
  O << "Writing '" << Filename << "'...";
  std::ofstream F(Filename.c_str());
  
  if (F.good())
    WriteGraph(F, GT);
  else
    O << "  error opening file for writing!";
  O << "\n";
}


//===----------------------------------------------------------------------===//
//                              Call Graph Printer
//===----------------------------------------------------------------------===//

namespace llvm {
  template<>
  struct DOTGraphTraits<CallGraph*> : public DefaultDOTGraphTraits {
    static std::string getGraphName(CallGraph *F) {
      return "Call Graph";
    }
    
    static std::string getNodeLabel(CallGraphNode *Node, CallGraph *Graph) {
      if (Node->getFunction())
        return ((Value*)Node->getFunction())->getName();
      else
        return "Indirect call node";
    }
  };
}

namespace {
  struct CallGraphPrinter : public ModulePass {
    virtual bool runOnModule(Module &M) {
      WriteGraphToFile(std::cerr, "callgraph", &getAnalysis<CallGraph>());
      return false;
    }

    void print(std::ostream &OS) const {}
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<CallGraph>();
      AU.setPreservesAll();
    }
  };

  RegisterAnalysis<CallGraphPrinter> P2("print-callgraph",
                                        "Print Call Graph to 'dot' file");
}

//===----------------------------------------------------------------------===//
//                     Generic DataStructures Graph Printer
//===----------------------------------------------------------------------===//

namespace {

  template<class DSType>
  class DSModulePrinter : public ModulePass {
  protected:
    virtual std::string getFilename() = 0;

  public:
    bool runOnModule(Module &M) {
      DSType *DS = &getAnalysis<DSType>();
      std::string File = getFilename();
      std::ofstream of(File.c_str());
      if (of.good()) {
        DS->getGlobalsGraph().print(of);
        of.close();
      } else
        std::cerr << "Error writing to " << File << "!\n";
      return false;
    }

    void print(std::ostream &os) const {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.template addRequired<DSType>();
      AU.setPreservesAll();
    }
  };

  template<class DSType>
  class DSFunctionPrinter : public ModulePass {
  protected:
    Function *F;
    virtual std::string getFilename(Function &F) = 0;

  public:
    DSFunctionPrinter(Function *_F) : F(_F) {}

    bool runOnModule(Module &M) { 
      DSType *DS = &getAnalysis<DSType>();
      std::string File = getFilename(*F);
      std::ofstream of(File.c_str());
      if (of.good()) {
        if (DS->hasGraph(*F)) {
          DS->getDSGraph(*F).print(of);
          of.close();
        } else
          // Can be more creative and print the analysis name here
          std::cerr << "No DSGraph for: " << F->getName() << "\n";
      } else
        std::cerr << "Error writing to " << File << "!\n";
      return false;
    }

    void print(std::ostream &os) const {}

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.template addRequired<DSType>();
      AU.setPreservesAll();
    }
  };
}

//===----------------------------------------------------------------------===//
//                     BU DataStructures Graph Printer
//===----------------------------------------------------------------------===//

namespace {
  struct BUModulePrinter : public DSModulePrinter<BUDataStructures> {
    std::string getFilename() { return "buds.dot"; }
  };
  struct BUFunctionPrinter : public DSFunctionPrinter<BUDataStructures> {
    BUFunctionPrinter(Function *F) : DSFunctionPrinter<BUDataStructures>(F) {}
    std::string getFilename(Function &F) {
      return "buds." + F.getName() + ".dot";
    }
  };
}

//===----------------------------------------------------------------------===//
//                     TD DataStructures Graph Printer
//===----------------------------------------------------------------------===//

namespace {
  struct TDModulePrinter : public DSModulePrinter<TDDataStructures> {
    std::string getFilename() { return "tdds.dot"; }
  };
  struct TDFunctionPrinter : public DSFunctionPrinter<TDDataStructures> {
    TDFunctionPrinter(Function *F) : DSFunctionPrinter<TDDataStructures>(F) {}
    std::string getFilename(Function &F) {
      return "tdds." + F.getName() + ".dot";
    }
  };
}

//===----------------------------------------------------------------------===//
//                   Local DataStructures Graph Printer
//===----------------------------------------------------------------------===//

namespace {
  struct LocalModulePrinter : public DSModulePrinter<LocalDataStructures> {
    std::string getFilename() { return "localds.dot"; }
  };
  struct LocalFunctionPrinter : public DSFunctionPrinter<LocalDataStructures> {
    LocalFunctionPrinter(Function *F) 
      : DSFunctionPrinter<LocalDataStructures>(F) {}
    std::string getFilename(Function &F) {
      return "localds." + F.getName() + ".dot";
    }
  };
}

//===----------------------------------------------------------------------===//
//                      Pass Creation Methods
//===----------------------------------------------------------------------===//

namespace llvm {

  Pass *createCallGraphPrinterPass () { return new CallGraphPrinter(); }

  // BU DataStructures
  //
  Pass *createBUDSModulePrinterPass () {
    return new BUModulePrinter();
  }

  Pass *createBUDSFunctionPrinterPass (Function *F) {
    return new BUFunctionPrinter(F);
  }

  // TD DataStructures
  //
  Pass *createTDDSModulePrinterPass () {
    return new TDModulePrinter();
  }

  Pass *createTDDSFunctionPrinterPass (Function *F) {
    return new TDFunctionPrinter(F);
  }

  // Local DataStructures
  //
  Pass *createLocalDSModulePrinterPass () {
    return new LocalModulePrinter();
  }

  Pass *createLocalDSFunctionPrinterPass (Function *F) {
    return new LocalFunctionPrinter(F);
  }

} // end namespace llvm
