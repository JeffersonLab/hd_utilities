#include <TTree.h>
#include <TFile.h>
#include <TVector3.h>
#include <TInterpreter.h>
#include <TClonesArray.h>
#include <TSystem.h>
#include <TSystemDirectory.h>

#include "DrcLutNode.h"

void glxlut_convert(TString inFileName = "lut_all_avr.root", TString outFileName = "lut_all_flat.root"){

  vector<Float_t> fLutPixelAngleX[48];
  vector<Float_t> fLutPixelAngleY[48];
  vector<Float_t> fLutPixelAngleZ[48];
  vector<Float_t> fLutPixelTime[48];
  vector<Long64_t> fLutPixelPath[48];

  // create output file and tree
  TFile *outFile = TFile::Open(outFileName,"RECREATE","lut_all_flat",9);
  TTree *outTree = new TTree("lut_dirc_flat","Look-up table for DIRC");
  for(int l=0; l<48; l++){
	  outTree->Branch(Form("LUT_AngleX_%d",l),&fLutPixelAngleX[l]); 
	  outTree->Branch(Form("LUT_AngleY_%d",l),&fLutPixelAngleY[l]); 
	  outTree->Branch(Form("LUT_AngleZ_%d",l),&fLutPixelAngleZ[l]); 
	  outTree->Branch(Form("LUT_Time_%d",l),&fLutPixelTime[l]); 
	  outTree->Branch(Form("LUT_Path_%d",l),&fLutPixelPath[l]); 
  }

  // get LUT from input file
  TFile* f = new TFile(inFileName);
  TTree *t=(TTree *) f->Get("lut_dirc") ;
  TClonesArray* fLut[48];
  for(int l=0; l<48; l++){
    fLut[l] = new TClonesArray("DrcLutNode");
    t->SetBranchAddress(Form("LUT_%d",l),&fLut[l]); 
  }
  t->GetEntry(0);

  // fill flat output TTree with required information
  std::cout<<inFileName.Data()<<" has "<<fLut[0]->GetEntriesFast()<< " entries" <<std::endl;
  
  // loop over each node (pixel) and convert to single entry in flat TTree 
  for (int inode=0; inode<6912; inode++){ 

	  if(inode%1000 == 0)
		  cout<<"Filling output TTree for pixel "<<inode<<endl;
	  
	  // clear vectors for this pixel
	  for(size_t l=0; l<48; l++){
		  fLutPixelAngleX[l].clear();
		  fLutPixelAngleY[l].clear();
		  fLutPixelAngleZ[l].clear();
		  fLutPixelTime[l].clear();
		  fLutPixelPath[l].clear();
	  }

	  // loop over bars to get info for each pixel
	  for(size_t l=0; l<48; l++){

		  // get node for this bar/pixel combination
		  for(int channel=inode; channel<13824; channel=channel+6912) {
			  DrcLutNode *node= (DrcLutNode*) fLut[l]->At(channel);
			  if(node->Entries() == 0) {
				fLutPixelAngleX[l].push_back(0);
				fLutPixelAngleY[l].push_back(0);
				fLutPixelAngleZ[l].push_back(0);
			  	fLutPixelTime[l].push_back(0);
			  	fLutPixelPath[l].push_back(0);
		  	}
		  	for(int i=0; i< node->Entries(); i++){
				TVector3 angle = node->GetEntry(i);
			  	fLutPixelAngleX[l].push_back(angle.X());
			  	fLutPixelAngleY[l].push_back(angle.Y());
			  	fLutPixelAngleZ[l].push_back(angle.Z());
			  	fLutPixelTime[l].push_back(node->GetTime(i));
			  	fLutPixelPath[l].push_back(node->GetPathId(i));
		  	}
		  	delete node;
		}

	  } // end loop over bars
	  
	  // fill entry in TTree for this pixel as an entry for all bars
	  outTree->Fill();

  } // end loop over pixels (ie. entries in new TTree)
    
  // close input file
  f->Close();

  // write new TTree to output file
  //outTree->Print();
  outFile->Write();
  std::cout<<"File  "<<outFileName<<" was created." <<std::endl;
  
}

