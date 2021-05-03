/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. */
/* See cxx source for full Copyright notice */
/* $Id$ */

#ifndef TaskDimuonPbPb_H
#define TaskDimuonPbPb_H

#include "AliAnalysisTaskSE.h"

class TaskDimuonPbPb : public AliAnalysisTaskSE  
{
    public:
                                TaskDimuonPbPb();
                                TaskDimuonPbPb(const char *name,int firstRun, int lastRun, UInt_t triggerClass);
        virtual                 ~TaskDimuonPbPb();
        
        virtual void            NotifyRun();
        virtual void            UserCreateOutputObjects();
        virtual void            UserExec(Option_t* option);
        virtual void            Terminate(Option_t* option);

    private:
        AliAODEvent*            fAODEvent;      //! input event
        AliVEvent*              fVEvent;        //! input event
        AliMuonTrackCuts*       fMuonTrackCuts; //! usual cuts on single muon tracks
        UInt_t                  fTriggerClass;  // trigger selection
        int                     fFirstRun, fLastRun;
        Int_t                   fRunNumber;
        Float_t                 fCentrality;


        TTree *treeEvents;
        std::vector<double> vectorDiMuonOS;     // histogram to store some properties of dimuons unlike sign
        std::vector<double> vectorDiMuonLS;
        std::vector<double> vectorSingleMuon;   // histogram to store some properties of single muons
      
        TObjArray *fListEvent;   

        TaskDimuonPbPb(const TaskDimuonPbPb&); // not implemented
        TaskDimuonPbPb& operator=(const TaskDimuonPbPb&); // not implemented

        ClassDef(TaskDimuonPbPb, 2);
};

#endif
