// Script to get current values of the offsets from the alignment root file, and shift them according to the MILLEPEDE result.
#include <map>
#include <fstream>
#include <iostream>
#include <string>
#include "TProfile.h"
#include "TFile.h"

using namespace std;

void DumpCDCOffsets(TString rootFile = "hd_root.root"){
  
   TFile *file = TFile::Open(rootFile);
   TProfile *hCDCConstants;
   file->GetObject("AlignmentConstants/CDCAlignmentConstants",hCDCConstants);

  // Global CDC offsets 
   ofstream outFile;
   outFile.open("cdc_global.txt");
   for (unsigned int i = 1; i<=6; i++){
      int histIndex = i;
      outFile << hCDCConstants->GetBinContent(histIndex) << " " ;
   }
   outFile.close();

   // Wire alignment
   outFile.open("cdc_wire_alignment.txt");
   for (unsigned int i = 1001; i<=15088; i++){
      int histIndex = i;
      outFile << hCDCConstants->GetBinContent(histIndex) << " " ;
      if (i%4 == 0) outFile << endl;
   }
   outFile.close();
   
   // t0 alignment 
   outFile.open("cdc_t0_alignment.txt");
   for (unsigned int i = 16001; i<=19522; i++){
      int histIndex = i;
      outFile << hCDCConstants->GetBinContent(histIndex) << endl;
   }
   outFile.close();

}
