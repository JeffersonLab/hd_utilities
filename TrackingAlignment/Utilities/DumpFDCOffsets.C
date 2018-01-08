// Script to get current values of the offsets from the alignment root file and dump them to text files
#include <map>
#include <fstream>
#include <iostream>
#include <string>
#include "TProfile.h"
#include "TFile.h"

using namespace std;

void DumpFDCOffsets(TString rootFile = "hd_root.root"){
  
   TFile *file = TFile::Open(rootFile);
   TProfile *hFDCConstants;
   file->GetObject("AlignmentConstants/FDCAlignmentConstants",hFDCConstants);

  // Cell offsets 
  // Indices are 101001, 101100 etc...
   ofstream outFile;
   outFile.open("cell_offsets.txt");
   for (unsigned int i = 1; i<=24; i++){
      int histIndex = i*1000;
      outFile << hFDCConstants->GetBinContent(histIndex+1) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+100) << endl ;
   }
   outFile.close();

   // Cathode Offsets
   // Indices are 101, 102, 103, 104
   outFile.open("cathode_alignment.txt");
   for (unsigned int i = 1; i<=24; i++){
      int histIndex = i*1000;
      outFile << hFDCConstants->GetBinContent(histIndex+103) << " " ;//dPhiU
      outFile << hFDCConstants->GetBinContent(histIndex+101) << " " ;//dU
      outFile << hFDCConstants->GetBinContent(histIndex+104) << " " ;//dPhiV
      outFile << hFDCConstants->GetBinContent(histIndex+102) << endl ;//dV
   }
   outFile.close();

   // Strip Pitches
   // Indices are 200...209
   outFile.open("strip_pitches.txt");
   for (unsigned int i = 1; i<=24; i++){
      int histIndex = i*1000;
      outFile << hFDCConstants->GetBinContent(histIndex+200) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+201) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+202) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+203) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+204) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+205) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+206) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+207) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+208) << " " ;
      outFile << hFDCConstants->GetBinContent(histIndex+209) << endl;
   }
   outFile.close();

   // Wire Alignment
   // Indices are 2
   outFile.open("wire_alignment.txt");
   for (unsigned int i = 1; i<=24; i++){
      int histIndex = i*1000;
      outFile << hFDCConstants->GetBinContent(histIndex+2) << " 0.0 0.0" << endl;
   }
   outFile.close();

   for (unsigned int i = 1; i<=4; i++){
      outFile.open(Form("t0_package%i.txt",i));
      for (unsigned int j=1; j<=6;j++){
         for (unsigned int k = 1; k<= 96; k++){
            int histIndex = ((i-1)*4+j)*1000 + 900;
            outFile << hFDCConstants->GetBinContent(histIndex+k)<< " " ;
         }
         outFile << endl;
      }
      outFile.close();
   }
   //gains
   for (unsigned int i = 1; i<=4; i++){
      outFile.open(Form("gains_package%i.txt",i));
      for (unsigned int j=1; j<=6;j++){
         for (unsigned int k = 1; k<= 216; k++){
            int histIndex = ((i-1)*4+j)*1000 + 300;
            outFile << hFDCConstants->GetBinContent(histIndex+k) << " " ;
         }
         outFile << endl;
         for (unsigned int k = 1; k<= 216; k++){
            int histIndex = ((i-1)*4+j)*1000 + 600;
            outFile << hFDCConstants->GetBinContent(histIndex+k) << " " ;
         }
         outFile << endl;
      }
      outFile.close();
   }
}
