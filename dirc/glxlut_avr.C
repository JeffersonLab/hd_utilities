#include <TTree.h>
#include <TFile.h>
#include <TVector3.h>
#include <TInterpreter.h>
#include <TClonesArray.h>
#include <TCanvas.h>
#include <TH1F.h>


#include "/work/halld2/home/jrsteven/2018-dirc/builds/halld_recon/src/plugins/Analysis/lut_dirc/DrcLutNode.h"


void glxlut_avr(TString baseFile = "lut.root"){
  gInterpreter->GenerateDictionary("vector<TVector3>","TVector3.h"); 

  if(baseFile=="") return;
  
  TString inFile =baseFile;
  TString outFile = baseFile.Remove(baseFile.Last('.'))+"_avr.root";

  TFile* f = new TFile(inFile);
  TTree *t=(TTree *) f->Get("lut_dirc") ;
  TClonesArray* fLut[48];
  for(Int_t l=0; l<48; l++){
    fLut[l] = new TClonesArray("DrcLutNode");
    t->SetBranchAddress(Form("LUT_%d",l),&fLut[l]); 
  }

 
  TFile *fFileNew = TFile::Open(outFile, "RECREATE");
  TClonesArray *fLutNew[48];

  TTree *fTreeNew = new TTree("lut_dirc","Look-up table for DIRC. Averaged");

  Int_t Nnodes = 30000;
  for(Int_t l=0; l<48; l++){
    fLutNew[l] = new TClonesArray("DrcLutNode");
    fTreeNew->Branch(Form("LUT_%d",l),&fLutNew[l],256000,-1);
    TClonesArray &fLutaNew = *fLutNew[l];
    for (Long64_t n=0; n<Nnodes; n++) {
      new((fLutaNew)[n]) DrcLutNode(-1);
    }
  }

  // TCanvas* c = new TCanvas("c","c",0,0,800,1200); c->Divide(2,2);
  // TH1F * hTime = new TH1F("hTime","Time",5000,0,10);
  // TH1F * hDirx = new TH1F("hDirx","X component",1000,-1,1);
  // TH1F * hDiry = new TH1F("hDiry","Y component",1000,-1,1);
  // TH1F * hDirz = new TH1F("hDirz","Z component",1000,-1,1);

  const int max = 5000;
  std::vector<TVector3> vArray[max];
  std::vector<Double_t> tArray[max];
  std::vector< Long64_t> pArray;

  int nsum;
  TVector3 dir,dir2,osum,sum;
  Double_t angle,minangle,time,sumt;
  Long64_t pathid;
  DrcLutNode *node;

  for(size_t l=0; l<48; l++){
    t->GetEntry(l);
    for (int inode=0; inode<fLut[l]->GetEntriesFast(); inode++){
      if(inode%1000==0) cout<<"Node # "<<inode << "  L "<<l<<endl;
      node= (DrcLutNode*) fLut[l]->At(inode);
      int size = node->Entries();    
      if(size<1) continue;
      for(int i=0; i<size; i++){
	dir = node->GetEntry(i);
	time = node->GetTime(i);
	pathid = node->GetPathId(i);
	if(time>20) continue;
	
	bool newid = true;
	for(size_t j=0; j<pArray.size(); j++){
	  if(pathid == pArray[j]){	    
	    vArray[j].push_back(dir);
	    tArray[j].push_back(time);
	    newid= false;
	  }
	}
	if(newid) {
	  vArray[pArray.size()].push_back(dir);
	  tArray[pArray.size()].push_back(time);
	  pArray.push_back(pathid);
	}
      }

      for(size_t j=0; j<pArray.size(); j++){
	sum = TVector3(0,0,0);
	sumt=0;
	nsum=0;
	
	if(vArray[j].size()<5) continue;
	
	osum = TVector3(0,0,0);
	for(size_t v=0; v<vArray[j].size(); v++) osum += vArray[j][v];
	osum *= 1/(double)vArray[j].size();
	
	for(size_t v=0; v<vArray[j].size(); v++) {
	  if(osum.Angle(vArray[j][v])>0.1) continue; // remove outliers	  
	  sum += vArray[j][v]; 
	  sumt += tArray[j][v];
	  nsum++;
	  
	  // std::cout<<inode<<" "<<pArray[j]<< " vArray[j][v].Y() "<<vArray[j][v].Y()<<" "<<vArray[j][v].Z()<<std::endl;	  
	  // hDirx->Fill(vArray[j][v].X());
	  // hDiry->Fill(vArray[j][v].Y());
	  // hDirz->Fill(vArray[j][v].Z());
	  // hTime->Fill(tArray[j][v]);
	}		

	if(nsum<1) continue;
	sum *= 1/(double)nsum;
	sumt *=1/(double)nsum;
	
	// c->cd(1);
	// hTime->Draw();
	// c->cd(2);
	// hDirx->Draw();
	// c->cd(3);
	// hDiry->Draw();
	// c->cd(4);
	// hDirz->Draw();	
	// c->Update();  
	// c->WaitPrimitive();
	// hDirx->Reset();
	// hDiry->Reset();
	// hDirz->Reset();
	// hTime->Reset();
      
	((DrcLutNode*)(fLutNew[l]->At(inode)))->AddEntry(node->GetLutId(),node->GetDetectorId(), sum,pArray[j],node->GetNRefl(0),sumt, node->GetDigiPos(),node->GetDigiPos(),nsum/(double)size); 
      }
      for(size_t i=0; i<max; i++) {vArray[i].clear();  tArray[i].clear();}
      pArray.clear();
    }
  }
  
  fTreeNew->Fill();
  fTreeNew->Write();
  //fFileNew->Write();

}
