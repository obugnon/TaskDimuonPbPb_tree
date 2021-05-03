// include the header of your analysis task here! for classes already compiled by aliBuild,
// precompiled header files (with extension pcm) are available, so that you do not need to
// specify includes for those. for your own task however, you (probably) have not generated a
// pcm file, so we need to include it explicitly
#include "TaskDimuonPbPb.h"

void runAnalysis()
{
    // set if you want to run the analysis locally (kTRUE), or on grid (kFALSE)
    Bool_t local = kTRUE;
    // if you run on grid, specify test mode (kTRUE) or full grid model (kFALSE)
    Bool_t gridTest = kFALSE;
    
    // since we will compile a class, tell root where to look for headers  
#if !defined (__CINT__) || defined (__CLING__)
    gInterpreter->ProcessLine(".include $ROOTSYS/include");
    gInterpreter->ProcessLine(".include $ALICE_ROOT/include");
#else
    gROOT->ProcessLine(".include $ROOTSYS/include");
    gROOT->ProcessLine(".include $ALICE_ROOT/include");
#endif
     
    // create the analysis manager
    AliAnalysisManager *mgr = new AliAnalysisManager("AnalysisTaskExample");
    AliAODInputHandler *aodH = new AliAODInputHandler();
    mgr->SetInputEventHandler(aodH);



    // compile the class and load the add task macro
    // here we have to differentiate between using the just-in-time compiler
    // from root6, or the interpreter of root5
#if !defined (__CINT__) || defined (__CLING__)
    gInterpreter->LoadMacro("TaskDimuonPbPb.cxx++g");
    TaskDimuonPbPb *task = reinterpret_cast<TaskDimuonPbPb*>(gInterpreter->ExecuteMacro("AddMyTask.C"));
#else
    gROOT->LoadMacro("TaskDimuonPbPb.cxx++g");
    gROOT->LoadMacro("AddMyTask.C");
    TaskDimuonPbPb *task = AddMyTask();
#endif


    if(!mgr->InitAnalysis()) return;
    mgr->SetDebugLevel(2);
    mgr->PrintStatus();
    mgr->SetUseProgressBar(1, 25);

    if(local) {
        // if you want to run locally, we need to define some input
        TChain* chain = new TChain("aodTree");
        // add a few files to the chain (change this so that your local files are added)
        chain->Add("AliAOD.Muons_15o.root");
        // start the analysis locally, reading the events from the tchain
        mgr->StartAnalysis("local", chain);
    } else {
        // if we want to run on grid, we create and configure the plugin
        AliAnalysisAlien *alienHandler = new AliAnalysisAlien();
        // also specify the include (header) paths on grid
        alienHandler->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_ROOT/include -I$ALICE_PHYSICS/include");
        // make sure your source files get copied to grid
        alienHandler->SetAdditionalLibs("TaskDimuonPbPb.cxx TaskDimuonPbPb.h");
        alienHandler->SetAnalysisSource("TaskDimuonPbPb.cxx");
        // select the aliphysics version. all other packages
        // are LOADED AUTOMATICALLY!
        alienHandler->SetAliPhysicsVersion("vAN-20200201_ROOT6-1");
        // set the Alien API version
        alienHandler->SetAPIVersion("V1.1x");
        // select additionnal data file
        // alienHandler->AddDataFile("alien:///alice/cern.ch/user/o/obugnon/downscaling_factors_2018.root");
        // select the input data
        //alienHandler->SetGridDataDir("/alice/data/2015/LHC15o");
         alienHandler->SetGridDataDir("/alice/data/2018/LHC18q");
        // alienHandler->SetGridDataDir("/alice/data/2018/LHC18r");
        // alienHandler->SetDataPattern("*muon_calo_pass1/AOD229/*AliAOD.Muons.root");
        alienHandler->SetDataPattern("*muon_calo_pass3/AOD225/*AliAOD.Muons.root");
        // alienHandler->SetDataPattern("*muon_calo_pass3/AOD225/*AliAOD.Muons.root");
        // MC has no prefix, data has prefix 000
        alienHandler->SetRunPrefix("000");
        // runnumber
        TString inputFile = gSystem->GetFromPipe("cat runList.txt");
        alienHandler->AddRunList(inputFile.Data());
        //alienHandler->AddRunNumber(246994);
        // number of files per subjob
        alienHandler->SetSplitMaxInputFileNumber(100);
        alienHandler->SetExecutable("myTask.sh");
        // specify how many seconds your job may take
        alienHandler->SetTTL(35000);
        alienHandler->SetJDLName("myTask.jdl");

        alienHandler->SetOutputToRunNo(kTRUE);
        alienHandler->SetKeepLogs(kTRUE);
        // merging: run with kTRUE to merge on grid
        // after re-running the jobs in SetRunMode("terminate") 
        // (see below) mode, set SetMergeViaJDL(kFALSE) 
        // to collect final results
        alienHandler->SetMaxMergeStages(1);
        alienHandler->SetMergeViaJDL(kTRUE);

        // define the output folders
        // alienHandler->SetGridWorkingDir("dimuonData_LHC15o");
        alienHandler->SetGridWorkingDir("dimuonData_LHC18q");
        // alienHandler->SetGridWorkingDir("dimuonData_LHC18r");
        alienHandler->SetGridOutputDir("myOutputDir");

        // connect the alien plugin to the manager
        mgr->SetGridHandler(alienHandler);
        if(gridTest) {
            // speficy on how many files you want to run
            alienHandler->SetNtestFiles(1);
            // and launch the analysis
            alienHandler->SetRunMode("test");
            mgr->StartAnalysis("grid");
        } else {
            // else launch the full grid analysis
            alienHandler->SetRunMode("full");
            mgr->StartAnalysis("grid");
        }
    }
}
