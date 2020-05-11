#include <iostream>
#include <fstream>
#include <string>
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/MC/StringTableBuilder.h"
#include "llvm/Support/Allocator.h"
#include <map>
#include <memory>
#include <vector>
#include <string>
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"

using namespace llvm;
using namespace std;

static Module *TheModule;
static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);

void GetPsycloneVariables(int &Start, int &End, int &Step, double &Assignment)
{

    //Function opens file produced by PSyclone and assigns each value in the text file 
    //to the corresponding variable Depending on the line in the file.

    ifstream File;
    int File_Line = 1;
    string File_Text;

    File.open("Variables.txt");
    File >> Start >> End >> Step >> Assignment;
    File.close();
    Start--;
}

void WriteIRToFile(Module &MyModule)
{
    //Function opens file IR_Output.ll and writes the LLVM IR inside Module into the file
    
    auto Filename = "IR_Output.ll";
    error_code EC;
    raw_fd_ostream Dest(Filename, EC);
    MyModule.print(Dest, nullptr);
    Dest.close();
}

int main()
{   
    int Loop_Start;
    int Loop_End; 
    int Loop_Step;
    double Assignment_Value;

    GetPsycloneVariables(Loop_Start, Loop_End, Loop_Step, Assignment_Value);

    //Link in functionality for all architectures
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();

    //Initialise module
    TheModule = new Module("assign_scalar.f90", TheContext);

    //Set target triple for current machine
    auto TargetTriple = sys::getDefaultTargetTriple();
    TheModule->setTargetTriple(TargetTriple);
    
    //Set data layout for current machine
    string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
    auto CPU = "generic";
    auto Features = "";
    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    //Initialise function main()
    FunctionType *FType = FunctionType::get(Type::getInt32Ty(TheContext), false);
    Function *Func = Function::Create(FType, GlobalValue::LinkageTypes::ExternalLinkage, "main", TheModule);

    //Initialise each label
    BasicBlock *Label_0 = BasicBlock::Create(TheContext, "0", Func);
    BasicBlock *Label_4 = BasicBlock::Create(TheContext, "4", Func);
    BasicBlock *Label_7 = BasicBlock::Create(TheContext, "7", Func);
    BasicBlock *Label_11 = BasicBlock::Create(TheContext, "11", Func);
    BasicBlock *Label_14 = BasicBlock::Create(TheContext, "14", Func);

    //Basic block 0
    Builder.SetInsertPoint(Label_0);
    AllocaInst *alloc_1 = new AllocaInst(Type::getInt32Ty(TheContext),0, nullptr, 4, "1", Label_0);
    Type *The_Double = IntegerType::getDoubleTy(TheContext);
    ArrayType *Double_Array = ArrayType::get(The_Double, Loop_End);
    AllocaInst *alloc_2 = new AllocaInst(Double_Array, 0, nullptr, 16, "2", Label_0);
    AllocaInst *alloc_3 = new AllocaInst(Type::getInt32Ty(TheContext),0, nullptr, 4, "3", Label_0);
    Value *Zero_Int = ConstantInt::get(Type::getInt32Ty(TheContext), 0, false);
    Value *Start = ConstantInt::get(Type::getInt32Ty(TheContext), Loop_Start, false);
    Builder.CreateAlignedStore(Zero_Int, alloc_1, 4, false);
    Builder.CreateAlignedStore(Start, alloc_3, 4, false);
    Builder.CreateBr(Label_4);

    //Basic block 4
    Builder.SetInsertPoint(Label_4); 
    LoadInst *Load_3 = Builder.CreateAlignedLoad(alloc_3, 4, "5");
    Value *Loop_Length = ConstantInt::get(Type::getInt32Ty(TheContext), Loop_End, false);
    Value *Icmp_5  = Builder.CreateICmpSLT(Load_3, Loop_Length, "6");
    Builder.CreateCondBr(Icmp_5, Label_7, Label_14);

    //Basic block 7
    Builder.SetInsertPoint(Label_7);
    Load_3 = Builder.CreateAlignedLoad(alloc_3, 4, "8");
    Value *Sext_8 = Builder.CreateSExt(Load_3, Type::getInt64Ty(TheContext), "9");
    Value *Gep_Start = ConstantInt::get(Type::getInt64Ty(TheContext), 0, false);
    Value *Indices[2] = {Gep_Start, Sext_8};
    Value *Gep_1 = Builder.CreateInBoundsGEP(alloc_2, ArrayRef<Value *>(Indices, 2), "10");
    Value *Double_Store = ConstantInt::get(The_Double, 0, false);
    Constant *Floating_Double = ConstantFP::get(IntegerType::getDoubleTy(TheContext), Assignment_Value);
    StoreInst *Store_10 = Builder.CreateAlignedStore(Floating_Double, Gep_1, 8, false);
    Builder.CreateBr(Label_11);

    //Basic block 11
    Builder.SetInsertPoint(Label_11);
    Load_3 = Builder.CreateAlignedLoad(alloc_3, 4, "12");
    Value *Iterate = ConstantInt::get(Type::getInt32Ty(TheContext), Loop_Step, false);
    Value *Add_1 = Builder.CreateNSWAdd(Load_3, Iterate, "13");
    Builder.CreateAlignedStore(Add_1, alloc_3, 4, false);
    Builder.CreateBr(Label_4);

    //Basic block 14
    Builder.SetInsertPoint(Label_14);
    Value *Return = ConstantInt::get(Type::getInt32Ty(TheContext), 0, false);
    Builder.CreateRet(Return);

    WriteIRToFile(*TheModule);
    return 0;
}