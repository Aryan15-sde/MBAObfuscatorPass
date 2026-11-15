#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct MBAObfuscatorPass : PassInfoMixin<MBAObfuscatorPass> {
  static bool isRequired() { return true; }

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    int TotalTransformed = 0;

    for (int Round = 0; Round < 2; ++Round) {
      SmallVector<Instruction *, 32> ToErase;

      for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
          auto *BinOp = dyn_cast<BinaryOperator>(&I);
          if (!BinOp || !BinOp->getType()->isIntOrIntVectorTy()) {
            continue;
          }

          IRBuilder<> Builder(BinOp);
          Value *A = BinOp->getOperand(0);
          Value *B = BinOp->getOperand(1);
          Value *One = ConstantInt::get(A->getType(), 1);
          Value *Replacement = nullptr;

          switch (BinOp->getOpcode()) {
            case Instruction::Add: {
              Value *XorVal = Builder.CreateXor(A, B, "mba_xor");
              Value *AndVal = Builder.CreateAnd(A, B, "mba_and");
              Value *ShlVal = Builder.CreateShl(AndVal, One, "mba_shl");
              Replacement = Builder.CreateAdd(XorVal, ShlVal, "mba_add");
              break;
            }
            case Instruction::Sub: {
              Value *XorVal = Builder.CreateXor(A, B, "mba_sxor");
              Value *NotA = Builder.CreateNot(A, "mba_not");
              Value *AndVal = Builder.CreateAnd(NotA, B, "mba_sand");
              Value *ShlVal = Builder.CreateShl(AndVal, One, "mba_sshl");
              Replacement = Builder.CreateSub(XorVal, ShlVal, "mba_sub");
              break;
            }
            case Instruction::Xor: {
              Value *OrVal = Builder.CreateOr(A, B, "mba_xor_or");
              Value *AndVal = Builder.CreateAnd(A, B, "mba_xor_and");
              Replacement = Builder.CreateSub(OrVal, AndVal, "mba_xor_sub");
              break;
            }
            case Instruction::And: {
              Value *AddVal = Builder.CreateAdd(A, B, "mba_and_add");
              Value *OrVal = Builder.CreateOr(A, B, "mba_and_or");
              Replacement = Builder.CreateSub(AddVal, OrVal, "mba_and_sub");
              break;
            }
            case Instruction::Or: {
              Value *XorVal = Builder.CreateXor(A, B, "mba_or_xor");
              Value *AndVal = Builder.CreateAnd(A, B, "mba_or_and");
              Replacement = Builder.CreateAdd(XorVal, AndVal, "mba_or_add");
              break;
            }
            default:
              break;
          }

          if (Replacement) {
            BinOp->replaceAllUsesWith(Replacement);
            ToErase.push_back(BinOp);
          }
        }
      }

      for (Instruction *Inst : ToErase) {
        Inst->eraseFromParent();
      }

      TotalTransformed += ToErase.size();
    }

    if (TotalTransformed > 0) {
      errs() << "Obfuscated " << TotalTransformed
             << " instruction(s) across 2 rounds in function: "
             << F.getName() << "\n";
      return PreservedAnalyses::none();
    }

    return PreservedAnalyses::all();
  }
};

}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "MBAObfuscatorPass", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "mba-obfuscator") {
            FPM.addPass(MBAObfuscatorPass());
            return true;
          }
          return false;
        });
    }
  };
}
