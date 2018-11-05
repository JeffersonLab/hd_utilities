#include <TTree.h>
#include <TFile.h>
#include <TVector3.h>
#include <TInterpreter.h>
#include <TClonesArray.h>
#include <TSystem.h>
#include <TSystemDirectory.h>

#include "/work/halld2/home/jrsteven/2018-dirc/builds/halld_recon/src/plugins/Analysis/lut_dirc/DrcLutNode.h"

TClonesArray *fLutSum[48];


void adddirs(TString filename){
  TFile* f = new TFile(filename);
  TTree *t=(TTree *) f->Get("lut_dirc") ;
  TClonesArray* fLut[48];
  for(int l=0; l<48; l++){
    fLut[l] = new TClonesArray("DrcLutNode");
    t->SetBranchAddress(Form("LUT_%d",l),&fLut[l]); 
  }
  t->GetEntry(0);
  std::cout<<filename<<" has "<<fLut[0]->GetEntriesFast()<< " entries" <<std::endl;
  for(size_t l=0; l<48; l++){
    for (int inode=0; inode<fLut[l]->GetEntriesFast(); inode++){
      DrcLutNode *node= (DrcLutNode*) fLut[l]->At(inode);
      for(int i=0; i< node->Entries(); i++){
	((DrcLutNode*)(fLutSum[l]->At(inode)))->AddEntry(node->GetLutId(),node->GetDetectorId(), node->GetEntry(i), node->GetPathId(i), node->GetNRefl(i), node->GetTime(i), node->GetHitPos(i), node->GetDigiPos());
      }
      delete node;
    }
  }
  for(int l=0; l<48; l++) fLut[l]->Clear();
    
  f->Close();
}


void glxlut_add(TString inFile = "lut_avr*.root", TString outFile = "lut_all_avr.root"){

  TTree *fTreeNew = new TTree("lut_dirc","Look-up table for DIRC");
  for(int l=0; l<48; l++){
    fLutSum[l] = new TClonesArray("DrcLutNode");
    fTreeNew->Branch(Form("LUT_%d",l),&fLutSum[l],256000,0); 
  }

  int Nnodes = 30000;
  for(int l=0; l<48; l++){
    TClonesArray &fLutaSum = *fLutSum[l];
    for (int n=0; n<Nnodes; n++) {
      new((fLutaSum)[n]) DrcLutNode(-1);
    }
  }

  if(inFile.Contains("*")){
    TString tname = inFile;
    TString tname1 = inFile;
    TString tname2 = inFile;
    TString end = tname1.Remove(0,inFile.Last('*')+1);
    TString start = tname2.Remove(inFile.Last('*'));
    
    TString dirname("./");
    if(inFile.Last('/')!=-1) dirname= tname.Remove(inFile.Last('/')) + "/";

    const char *ext=".root";
    TSystemDirectory dir(dirname, dirname);
    TList *files = dir.GetListOfFiles();
    if (files) {
      TSystemFile *file;
      TString fname;
      TIter next(files);
      while ((file=(TSystemFile*)next())) {
	fname = file->GetName();
	if (!file->IsDirectory() && fname.EndsWith(ext)) {
	  TString path = dirname+fname;
	  TString substr = path.SubString(start);
	  if( substr.Length()>0 && path.EndsWith(end)){
	    adddirs(path);
	  }
	}
      }
    }
  }else{
    std::cout<<"infile  "<<inFile  <<std::endl;
    adddirs(inFile);
  }
 
  TFile *fFileNew = TFile::Open(outFile,"RECREATE");
  fTreeNew->Fill();
  fTreeNew->Write();
  fFileNew->Write();
  std::cout<<"File  "<<outFile<<" was created." <<std::endl;
  
}

