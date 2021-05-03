/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* AliAnaysisTaskMyTask
 *
 * empty task which can serve as a starting point for building an analysis
 * as an example, one histogram is filled
 */

// include root libraries
#include <iostream>
#include "TChain.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TList.h"
#include "TChain.h"
#include "TMath.h"
#include "THnSparse.h"
#include "TFile.h"
// include AliRoot Libraries
#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"
#include "AliAODEvent.h"
#include "AliVEvent.h"
#include "AliAODInputHandler.h"
#include "AliMuonTrackCuts.h"
#include "AliVTrack.h"
#include "AliAnalysisMuonUtility.h"
#include "AliMultSelection.h"
#include "TaskDimuonPbPb.h"

class TaskDimuonPbPb;    // your analysis class

using namespace std;            // std namespace: so you can do things like 'cout'

ClassImp(TaskDimuonPbPb) // classimp: necessary for root

TaskDimuonPbPb::TaskDimuonPbPb() : AliAnalysisTaskSE(), 
    fAODEvent(0),
    fVEvent(0),
    fMuonTrackCuts(0),
    fTriggerClass(0),
    fFirstRun(0),
    fLastRun(0),
    fRunNumber(0),
    fCentrality(0),
    fListEvent(0),
    vectorDiMuonOS(0),
    vectorDiMuonLS(0),
    vectorSingleMuon(0)
{
    // default constructor, don't allocate memory here!
    // this is used by root for IO purposes, it needs to remain empty
}
//_____________________________________________________________________________
TaskDimuonPbPb::TaskDimuonPbPb(const char* name,int firstRun, int lastRun, UInt_t triggerClass) : AliAnalysisTaskSE(name),
    fAODEvent(0),
    fVEvent(0),
    fMuonTrackCuts(0),
    fTriggerClass(triggerClass),
    fFirstRun(firstRun),
    fLastRun(lastRun),
    fRunNumber(0),
    fCentrality(0),
    fListEvent(0),
    vectorDiMuonOS(0),
    vectorDiMuonLS(0),
    vectorSingleMuon(0)
{
    // constructor
    DefineInput(0, TChain::Class());    // define the input of the analysis: in this case we take a 'chain' of events
                                        // this chain is created by the analysis manager, so no need to worry about it, 
                                        // it does its work automatically
    DefineOutput(1, TList::Class());    // define the ouptut of the analysis: in this case it's a list of histograms 
                                        // you can add more output objects by calling DefineOutput(2, classname::Class())
                                        // if you add more output objects, make sure to call PostData for all of them, and to
                                        // make changes to your AddTask macro!
}
//_____________________________________________________________________________
TaskDimuonPbPb::~TaskDimuonPbPb()
{
    if(fListEvent && !AliAnalysisManager::GetAnalysisManager()->IsProofMode()) {
        delete fListEvent;
    }
}
//_____________________________________________________________________________
void TaskDimuonPbPb::NotifyRun()
{
  /// Set run number for cuts
  if ( fMuonTrackCuts ) fMuonTrackCuts->SetRun(fInputHandler);
}
//_____________________________________________________________________________
void TaskDimuonPbPb::UserCreateOutputObjects()
{

    fListEvent = new TObjArray(2000);
    fListEvent->SetOwner(kTRUE);
    fListEvent->SetName("ListEvent");
    
    treeEvents = new TTree("eventsTree", "tree that contains information of the event");
    treeEvents->Branch("DimuonOS", &vectorDiMuonOS);
    treeEvents->Branch("DimuonLS", &vectorDiMuonLS);
    treeEvents->Branch("SingleMuon", &vectorSingleMuon);
    treeEvents->Branch("runNumber", &fRunNumber);
    treeEvents->Branch("centrality", &fCentrality);
    fListEvent->AddAtAndExpand(treeEvents, 0);


    if(fTriggerClass == AliVEvent::kMuonUnlikeLowPt7)
    {
        //The muon muonTrackCuts can be defined here. Hiwever it is better to defien it outside (in addTaskDimuonPPB.C). To be fixed
        fMuonTrackCuts = new AliMuonTrackCuts("StandardMuonTrackCuts","StandardMuonTrackCuts");
        fMuonTrackCuts->SetAllowDefaultParams(kTRUE);
        fMuonTrackCuts->SetFilterMask (AliMuonTrackCuts::kMuEta | AliMuonTrackCuts::kMuThetaAbs  | AliMuonTrackCuts::kMuMatchLpt | AliMuonTrackCuts::kMuPdca);//Set the cuts to be used for the muon selections. See all the available cuts in AliMuonTrackCuts.h
    }

  //This is needed to save the outputs.
  PostData(1, fListEvent);

}
//_____________________________________________________________________________
void TaskDimuonPbPb::UserExec(Option_t *)
{
    // user exec this function is called once for each event
    // the manager will take care of reading the events from file, and with the static function InputEvent() you 
    // have access to the current event. 
    // once you return from the UserExec function, the manager will retrieve the next event from the chain
    fAODEvent = dynamic_cast<AliAODEvent*>(InputEvent());    
    if(!fAODEvent) {
        AliError("ERROR: Could not retrieve AOD event !!");
        return;
    }
    TString strFiredTriggers = fAODEvent->GetFiredTriggerClasses();
                                
    //Physics Selection
    UInt_t IsSelected = (((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected());
    if(IsSelected & fTriggerClass)
    {
        //Event histos after physics selection
        //Get the centrality
        AliMultSelection *multSelection = (AliMultSelection * ) fAODEvent->FindListObject("MultSelection");
        fCentrality = multSelection->GetMultiplicityPercentile("V0M", false);
        if (fCentrality > 90.) return;  


        fVEvent = static_cast<AliVEvent *>(InputEvent());
        fRunNumber = fAODEvent->GetRunNumber();

        //Fill Dimuon properties vectors
        vectorDiMuonOS.clear();
        vectorDiMuonLS.clear();
        vectorSingleMuon.clear();

        //Fill Single Muon and Dimuon properties histograms
        TLorentzVector lvMuon1, lvMuon2, lvDiMuon;
        AliVParticle* muonTrack1;
        AliVParticle* muonTrack2;

        Float_t muonMass2 = AliAnalysisMuonUtility::MuonMass2(); // the PDG rest mass of the muon (constante, used for getting the kinematics) en GeV
        int numberOfTracks = AliAnalysisMuonUtility::GetNTracks(fVEvent); // get the number of muon tracks in the event
        for(Int_t iMuon1 = 0 ; iMuon1 < numberOfTracks ; iMuon1++) // loop ove rall these tracks
        {
            muonTrack1 = AliAnalysisMuonUtility::GetTrack(iMuon1,fVEvent);
            if( !muonTrack1 ) { AliError(Form("ERROR: Could not retrieve AOD or ESD track %d", iMuon1)); continue;}

            //single muon properties 
            Float_t energy1 = TMath::Sqrt(muonTrack1->P()*muonTrack1->P() + muonMass2);
            lvMuon1.SetPxPyPzE(muonTrack1->Px(),muonTrack1->Py(),muonTrack1->Pz(),energy1); //def 4-vect muon1
            Short_t muonCharge1 = muonTrack1->Charge();
            
            vectorSingleMuon.push_back(lvMuon1.Pt());
            vectorSingleMuon.push_back(lvMuon1.Eta());
            vectorSingleMuon.push_back(lvMuon1.Theta());
            vectorSingleMuon.push_back(lvMuon1.Phi());
            
            for (Int_t iMuon2 = iMuon1+1; iMuon2 < numberOfTracks; iMuon2++)
            {
                muonTrack2 = AliAnalysisMuonUtility::GetTrack(iMuon2,fVEvent);
                if ( !muonTrack2 ) {AliError(Form("ERROR: Could not retrieve AOD or ESD track %d", iMuon2)); continue;}
                if ( ! fMuonTrackCuts->IsSelected(muonTrack2) ) continue;//include cuts on pDCA, Eta, Rabs

                Float_t energy2 = TMath::Sqrt(muonTrack2->P()*muonTrack2->P() + muonMass2);
                lvMuon2.SetPxPyPzE(muonTrack2->Px(),muonTrack2->Py(),muonTrack2->Pz(),energy2); //def 4-vect muon1
                Short_t muonCharge2 = muonTrack2->Charge();

                //dimuon properties
                lvDiMuon = lvMuon1+lvMuon2;
                if (muonCharge1 != muonCharge2)
                {
                    vectorDiMuonOS.push_back(lvDiMuon.M());
                    vectorDiMuonOS.push_back(lvDiMuon.Pt());
                    vectorDiMuonOS.push_back(lvDiMuon.Rapidity());
                }
                else
                {
                    vectorDiMuonLS.push_back(lvDiMuon.M());
                    vectorDiMuonLS.push_back(lvDiMuon.Pt());
                    vectorDiMuonLS.push_back(lvDiMuon.Rapidity());
                }
            }
                    
        }
    
    }
    if (vectorSingleMuon.size() > 0)
    ((TTree *)fListEvent->UncheckedAt(0))->Fill(); //NOUVEAU : 
    PostData(1, fListEvent);
}
//_____________________________________________________________________________
void TaskDimuonPbPb::Terminate(Option_t *)
{
    // terminate
    // called at the END of the analysis (when all events are processed)
}    
//_____________________________________________________________________________
