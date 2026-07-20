namespace ExtractTrackBasedTimingNS {
   TFile * thisFile;

   TH1F * Get1DHistogram(const char * plugin, const char * directoryName, const char * name){
      TH1F * histogram;
      //TString fullName = TString(plugin) + "/" + TString(directoryName) + "/" + TString(name);
      TString fullName = TString(plugin) + "/Physics Triggers/" + TString(directoryName) + "/" + TString(name);
      thisFile->GetObject(fullName, histogram);
      if (histogram == 0){
         cout << "Unable to find histogram " << fullName.Data() << endl;
         return NULL;
      }
      return histogram;
   }

   TH2F * Get2DHistogram(const char * plugin, const char * directoryName, const char * name){
      TH2F * histogram;
      //TString fullName = TString(plugin) + "/" + TString(directoryName) + "/" + TString(name);
      TString fullName = TString(plugin) + "/Physics Triggers/" + TString(directoryName) + "/" + TString(name);
      thisFile->GetObject(fullName, histogram);
      if (histogram == 0){
         cout << "Unable to find histogram " << fullName.Data() << endl;
         return NULL;
      }
      return histogram;
   }
};

void GetCCDBConstants(TString path, Int_t run, TString variation, vector<double>& container, Int_t column = 1){
   char command[256];

   sprintf(command, "ccdb dump %s:%i:%s", path.Data(), run, variation.Data());
   FILE* inputPipe = gSystem->OpenPipe(command, "r");
   if(inputPipe == NULL)
      return;
   //get the first (comment) line
   char buff[1024];
   if(fgets(buff, sizeof(buff), inputPipe) == NULL)
      return;
   //get the remaining lines
   double entry;
   int counter = 0;
   while(fgets(buff, sizeof(buff), inputPipe) != NULL){
      istringstream locConstantsStream(buff);
      while (locConstantsStream >> entry){
         counter++;
         if (counter % column == 0) container.push_back(entry);
      }
   }
   //Close the pipe
   gSystem->ClosePipe(inputPipe);
}

void GetCCDBConstants2D(TString path, Int_t run, TString variation, vector< vector<double> >& container, Int_t numColumns){
   char command[256];

   sprintf(command, "ccdb dump %s:%i:%s", path.Data(), run, variation.Data());
   FILE* inputPipe = gSystem->OpenPipe(command, "r");
   if(inputPipe == NULL)
      return;
   //get the first (comment) line
   char buff[1024];
   if(fgets(buff, sizeof(buff), inputPipe) == NULL)
      return;
   //get the remaining lines
   double entry;
   int counter = 0;
   vector<double> row;
   while(fgets(buff, sizeof(buff), inputPipe) != NULL){
      istringstream locConstantsStream(buff);
      while (locConstantsStream >> entry){
         counter++;
	 if(counter == numColumns) {
		 container.push_back(row);
		 row.clear();
		 counter = 0;
	 }
         row.push_back(entry);
      }
   }
   //Close the pipe
   gSystem->ClosePipe(inputPipe);
}

//Overload this function to handle the base time offsets
void GetCCDBConstants1(TString path, Int_t run, TString variation, double& constant1){
   char command[256];
   sprintf(command, "ccdb dump %s:%i:%s", path.Data(), run, variation.Data());
   FILE* inputPipe = gSystem->OpenPipe(command, "r");
   if(inputPipe == NULL)
      return;
   //get the first (comment) line
   char buff[1024];
   if(fgets(buff, sizeof(buff), inputPipe) == NULL)
      return;
   //get the line containing the values
   while(fgets(buff, sizeof(buff), inputPipe) != NULL){
      istringstream locConstantsStream(buff);
      locConstantsStream >> constant1;
   }
   //Close the pipe
   gSystem->ClosePipe(inputPipe);
}

void GetCCDBConstants2(TString path, Int_t run, TString variation, double& constant1, double& constant2){
   char command[256];
   sprintf(command, "ccdb dump %s:%i:%s", path.Data(), run, variation.Data());
   FILE* inputPipe = gSystem->OpenPipe(command, "r");
   if(inputPipe == NULL)
      return;
   //get the first (comment) line
   char buff[1024];
   if(fgets(buff, sizeof(buff), inputPipe) == NULL)
      return;
   //get the line containing the values
   while(fgets(buff, sizeof(buff), inputPipe) != NULL){
      istringstream locConstantsStream(buff);
      locConstantsStream >> constant1 >> constant2;
   }
   //Close the pipe
   gSystem->ClosePipe(inputPipe);
}

int GetCCDBIndexTAGM(unsigned int column, unsigned int row){
   int CCDBIndex = column + row;
   if (column > 9) CCDBIndex += 5;
   if (column > 27) CCDBIndex += 5;
   if (column > 81) CCDBIndex += 5;
   if (column > 99) CCDBIndex += 5;

   return CCDBIndex;
}

int GetF1TDCslotTAGH(int id) {
   double N = 32.0; // channels per slot
   if (id >= 132 && id <= 172) throw("TAGH: unknown id in [132,172]");
   int HVid = (id <= 131) ? id : (id - 274 + 233);
   return int((HVid-1)/N) + 1;
}

void FindFDCPackageChamber(int plane, int &package, int &chamber) {
  package = plane / 6 + 1;
  chamber = plane % 6;
  if(chamber == 0) {
    chamber = 6;
    package--;
  }
}



void AdjustTiming(TString fileName = "hd_root.root", int runNumber = 10390, TString variation = "default", bool verbose = false,TString prefix = "") {
   // configuration parameters
   bool FIX_START_COUNTER = false;    // should we keep the start counter peak aligned to a certain time?
                                     // this helps in certain analysis without tracks
   double start_counter_t0 = 0.;     // time we should move the start counter time peak to 
   bool DONT_SHIFT_SC_RF = true;
   bool USE_NEUTRALS_FOR_BCAL_RF_ALIGN = true;
   bool USE_NEUTRALS_FOR_FCAL_RF_ALIGN = true;
   bool USE_NEUTRALS_FOR_ECAL_RF_ALIGN = true;
   int TOF_TABLE_VERSION = 2;

   // set "prefix" in case you want to ship the txt files elsewhere...
   cout << "Performing adjustment timing fits for File: " << fileName.Data() << " Run: " << runNumber << " Variation: " << variation.Data() << endl;

   ExtractTrackBasedTimingNS::thisFile = TFile::Open( fileName , "UPDATE");
   if (ExtractTrackBasedTimingNS::thisFile == 0) {
      cout << "Unable to open file " << fileName.Data() << "...Exiting" << endl;
      return;
   }

   //We need the existing constants, The best we can do here is just read them from the file.
   vector<double> sc_tdc_time_offsets;
   vector<double> sc_fadc_time_offsets;
   vector<double> tof_tdc_time_offsets;
   vector<double> tof_fadc_time_offsets;
   vector<double> tagm_tdc_time_offsets;
   vector<double> tagm_fadc_time_offsets;
   vector<double> tagh_tdc_time_offsets;
   vector<double> tagh_fadc_time_offsets;
   vector<double> tagh_counter_quality;
   vector< vector<double> > fdc_wire_offsets_package1;
   vector< vector<double> > fdc_wire_offsets_package2;
   vector< vector<double> > fdc_wire_offsets_package3;
   vector< vector<double> > fdc_wire_offsets_package4;

   double sc_t_base_fadc, sc_t_base_tdc;
   double tof_t_base_fadc, tof_t_base_tdc;
   double bcal_t_base_fadc, bcal_t_base_tdc;
   double tagm_t_base_fadc, tagm_t_base_tdc;
   double tagh_t_base_fadc, tagh_t_base_tdc;
   double fdc_t_base_fadc, fdc_t_base_tdc;
   double fcal_t_base;
   double ecal_t_base;
   double cdc_t_base;
   double RF_Period;

   cout << "Grabbing CCDB constants..." << endl;
   // Base times
   GetCCDBConstants1("/CDC/base_time_offset" ,runNumber, variation, cdc_t_base);
   GetCCDBConstants1("/ECAL/base_time_offset",runNumber, variation, ecal_t_base);
   GetCCDBConstants1("/FCAL/base_time_offset",runNumber, variation, fcal_t_base);
   GetCCDBConstants1("/PHOTON_BEAM/RF/beam_period",runNumber, variation, RF_Period);
   GetCCDBConstants2("/FDC/base_time_offset" ,runNumber, variation, fdc_t_base_fadc, fdc_t_base_tdc);
   GetCCDBConstants2("/BCAL/base_time_offset" ,runNumber, variation, bcal_t_base_fadc, bcal_t_base_tdc);
   GetCCDBConstants2("/PHOTON_BEAM/microscope/base_time_offset" ,runNumber, variation, tagm_t_base_fadc, tagm_t_base_tdc);
   GetCCDBConstants2("/PHOTON_BEAM/hodoscope/base_time_offset" ,runNumber, variation, tagh_t_base_fadc, tagh_t_base_tdc);
   GetCCDBConstants2("/START_COUNTER/base_time_offset" ,runNumber, variation, sc_t_base_fadc, sc_t_base_tdc);
   if(TOF_TABLE_VERSION==2) {
	   GetCCDBConstants2("/TOF2/base_time_offset" ,runNumber, variation, tof_t_base_fadc, tof_t_base_tdc);
	   GetCCDBConstants("/TOF2/adc_timing_offsets",runNumber, variation, tof_fadc_time_offsets);
	   GetCCDBConstants("/TOF2/timing_offsets",runNumber, variation, tof_tdc_time_offsets);
   } else {
	   GetCCDBConstants2("/TOF/base_time_offset" ,runNumber, variation, tof_t_base_fadc, tof_t_base_tdc);
	   GetCCDBConstants("/TOF/adc_timing_offsets",runNumber, variation, tof_fadc_time_offsets);
	   GetCCDBConstants("/TOF/timing_offsets",runNumber, variation, tof_tdc_time_offsets);
   }
   // Per channel
   //GetCCDBConstants("/BCAL/TDC_offsets"    ,runNumber, variation, bcal_tdc_offsets);
   //GetCCDBConstants("/FCAL/timing_offsets" ,runNumber, variation, fcal_adc_offsets);
   GetCCDBConstants("/START_COUNTER/adc_timing_offsets" ,runNumber, variation, sc_fadc_time_offsets);
   GetCCDBConstants("/START_COUNTER/tdc_timing_offsets" ,runNumber, variation, sc_tdc_time_offsets);
   GetCCDBConstants("/PHOTON_BEAM/microscope/fadc_time_offsets" ,runNumber, variation, tagm_fadc_time_offsets,3);// Interested in 3rd column
   GetCCDBConstants("/PHOTON_BEAM/microscope/tdc_time_offsets"  ,runNumber, variation, tagm_tdc_time_offsets,3);
   GetCCDBConstants("/PHOTON_BEAM/hodoscope/fadc_time_offsets"  ,runNumber, variation, tagh_fadc_time_offsets,2);// Interested in 2nd column
   GetCCDBConstants("/PHOTON_BEAM/hodoscope/tdc_time_offsets"   ,runNumber, variation, tagh_tdc_time_offsets,2);
   GetCCDBConstants("/PHOTON_BEAM/hodoscope/counter_quality"    ,runNumber, variation, tagh_counter_quality,2);
   GetCCDBConstants2D("/FDC/package1/wire_timing_offsets", runNumber, variation, fdc_wire_offsets_package1, 96);
   GetCCDBConstants2D("/FDC/package2/wire_timing_offsets", runNumber, variation, fdc_wire_offsets_package2, 96);
   GetCCDBConstants2D("/FDC/package3/wire_timing_offsets", runNumber, variation, fdc_wire_offsets_package3, 96);
   GetCCDBConstants2D("/FDC/package4/wire_timing_offsets", runNumber, variation, fdc_wire_offsets_package4, 96);

   vector< vector< vector<double> > >  old_FDC_wire_offsets;
   old_FDC_wire_offsets.push_back( fdc_wire_offsets_package1 );
   old_FDC_wire_offsets.push_back( fdc_wire_offsets_package2 );
   old_FDC_wire_offsets.push_back( fdc_wire_offsets_package3 );
   old_FDC_wire_offsets.push_back( fdc_wire_offsets_package4 );

   cout << "CDC base times = " << cdc_t_base << endl;
   cout << "ECAL base times = " << ecal_t_base << endl;
   cout << "FCAL base times = " << fcal_t_base << endl;
   cout << "FDC base times = " << fdc_t_base_fadc << ", " << fdc_t_base_tdc << endl;
   cout << "BCAL base times = " << bcal_t_base_fadc << ", " << bcal_t_base_tdc << endl;
   cout << "SC base times = " << sc_t_base_fadc << ", " << sc_t_base_tdc << endl;
   cout << "TOF base times = " << tof_t_base_fadc << ", " << tof_t_base_tdc << endl;
   cout << "TAGH base times = " << tagh_t_base_fadc << ", " << tagh_t_base_tdc << endl;
   cout << "TAGM base times = " << tagm_t_base_fadc << ", " << tagm_t_base_tdc << endl;

   cout << endl;
   cout << "RF_Period = " << RF_Period << endl;
   cout << endl;

   cout << "Done grabbing CCDB constants...Entering fits..." << endl;

   // Assume our alignment is okay, and mainly we need to either correct for some overall timing shifts
   // or per-channel shifts due to some firmware timing "features".  These include:
   // - 2 ns shifts of RF time
   // - 32 ns shifts of high-res TDC modules
   // - 4 ns shifts of fADC250 channels
   // - FDC TDC crate shifts
   // Note that BCAL/FCAL 4ns shifts are handled with different plugins

   //When the RF is present we can try to simply pick out the correct beam bucket for each of the runs
   //First just a simple check to see if we have the appropriate data
   bool useRF = false;
   TH1F *testHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TAGH_TDC_RF_Compare","TAGH Counter ID 001");
   if (testHist != NULL){ // Not great since we rely on channel 1 working, but can be craftier later.
      cout << "Using RF Times for Calibration" << endl;
      useRF = true;
   }
   ofstream outFile;
   TH2F *thisHist; 


   // check to make sure we have enough data to fit anything
   TH1F *this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "BCAL - SC Target Time");
   if(this1DHist == NULL) {
	   cout << "No BCAL - SC Target Time histogram exists, exiting ..." << endl;
	   return;
   }
   if(this1DHist->Integral() < 200.) {
	   cout << "Too few counts in BCAL - SC Target Time (" << this1DHist->Integral() << "), exiting ..." << endl;
	   return;
   }


   /***** old TAGM calibrations, now we just do the base times, the rest is handled by another plugin  
   thisHist = ExtractTrackBasedTimingNS::Get2DHistogram("HLDetectorTiming", "TRACKING", "TAGM - SC Target Time");
   if (useRF) thisHist = ExtractTrackBasedTimingNS::Get2DHistogram("HLDetectorTiming", "TRACKING", "TAGM - RFBunch Time");
   if (thisHist != NULL){
      //Statistics on these histograms are really quite low we will have to rebin and do some interpolation
      outFile.open(prefix + "tagm_tdc_timing_offsets.txt", ios::out | ios::trunc);
      outFile.close(); // clear file
      outFile.open(prefix + "tagm_adc_timing_offsets.txt", ios::out | ios::trunc);
      outFile.close(); // clear file
      int nBinsX = thisHist->GetNbinsX();
      int nBinsY = thisHist->GetNbinsY();
      TH1D * selectedTAGMOffset = new TH1D("selectedTAGMOffset", "Selected TAGM Offset; Column; Offset [ns]", nBinsX, 0.5, nBinsX + 0.5);
      TH1F * TAGMOffsetDistribution = new TH1F("TAGMOffsetDistribution", "TAGM Offset; TAGM Offset [ns]; Entries", 500, -250, 250);
      for (int i = 1 ; i <= nBinsX; i++){ 
         TH1D *projY = thisHist->ProjectionY("temp", i, i);
         // Scan over the histogram
         //chose the correct number of bins based on the histogram
         float nsPerBin = (projY->GetBinCenter(projY->GetNbinsX()) - projY->GetBinCenter(1)) / projY->GetNbinsX();
         float timeWindow = 3; //ns (Full Width)
         int binWindow = int(timeWindow / nsPerBin);
         double maxEntries = 0;
         double maxMean = 0;
         for (int j = 1 ; j <= projY->GetNbinsX();j++){
            int minBin = j;
            int maxBin = (j + binWindow) <= projY->GetNbinsX() ? (j + binWindow) : projY->GetNbinsX();
            double sum = 0, nEntries = 0;
            for (int bin = minBin; bin <= maxBin; bin++){
               sum += projY->GetBinContent(bin) * projY->GetBinCenter(bin);
               nEntries += projY->GetBinContent(bin);
               if (bin == maxBin){
                  if (nEntries > maxEntries) {
                     maxMean = sum / nEntries;
                     maxEntries = nEntries;
                  }
               } 
            }
         }
         //In the case there is RF, our job is to pick just the number of the correct beam bunch, so that's really all we need.
         if(useRF) {
            int beamBucket = int((maxMean / RF_Period) + 0.5); // +0.5 to handle rounding correctly
            selectedTAGMOffset->SetBinContent(i, beamBucket);
            TAGMOffsetDistribution->Fill(beamBucket);
         }
         else{
            selectedTAGMOffset->SetBinContent(i, maxMean);
            TAGMOffsetDistribution->Fill(maxMean);
         }
      }
      double meanOffset = TAGMOffsetDistribution->GetMean();
      // This might be in units of beam bunches, so we need to convert
      if (useRF) meanOffset *= RF_Period;
      if (verbose) {
         cout << "Dumping TAGM results...\n=======================================" << endl;
         cout << "TAGM mean Offset = " << meanOffset << endl;
         cout << "fADC Offsets" << endl;
      }

      outFile.open(prefix + "tagm_adc_timing_offsets.txt", ios::out);
      //for (int i = 1 ; i <= nBinsX; i++){
      // Loop over rows
      if (verbose) cout << "Column\tRow\tvalueToUse\toldValue\tmeanOffset\tTotal" << endl;
      for (unsigned int column = 1; column <= 102; column++){
         int index = GetCCDBIndexTAGM(column, 0);
         double valueToUse = selectedTAGMOffset->GetBinContent(index);
         if (useRF) valueToUse *= RF_Period;

         //if (valueToUse == 0) valueToUse = meanOffset;
         outFile << "0 " << column << " " << valueToUse + tagm_fadc_time_offsets[index-1] - meanOffset<< endl;
         if (verbose) printf("0\t%i\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n", column, valueToUse, tagm_fadc_time_offsets[index-1], meanOffset, 
               valueToUse + tagm_fadc_time_offsets[index-1] - meanOffset);
         if (column == 9 || column == 27 || column == 81 || column == 99){
            for (unsigned int row = 1; row <= 5; row++){
               index = GetCCDBIndexTAGM(column, row);
               valueToUse = selectedTAGMOffset->GetBinContent(index);
               if (useRF) valueToUse *= RF_Period;
               //if (valueToUse == 0) valueToUse = meanOffset;
               outFile << row << " " << column << " " << valueToUse + tagm_fadc_time_offsets[index-1] - meanOffset<< endl;
               if (verbose) printf("%i\t%i\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n", row, column, valueToUse, tagm_fadc_time_offsets[index-1], meanOffset,
                     valueToUse + tagm_fadc_time_offsets[index-1] - meanOffset);
            }
         }
      }
      outFile.close();

      if (verbose) {
         cout << "TDC Offsets" << endl;
         cout << "Column\tRow\tvalueToUse\toldValue\tmeanOffset\tTotal" << endl;
      }
      outFile.open(prefix + "tagm_tdc_timing_offsets.txt", ios::out);
      //for (int i = 1 ; i <= nBinsX; i++){
      // Loop over rows
      for (unsigned int column = 1; column <= 102; column++){
         int index = GetCCDBIndexTAGM(column, 0);
         double valueToUse = selectedTAGMOffset->GetBinContent(index);
         if (useRF) valueToUse *= RF_Period;
         //if (valueToUse == 0) valueToUse = meanOffset;
         outFile << "0 " << column << " " << valueToUse + tagm_tdc_time_offsets[index-1] - meanOffset << endl;
         if (verbose) printf("0\t%i\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n", column, valueToUse, tagm_tdc_time_offsets[index-1], meanOffset,
               valueToUse + tagm_tdc_time_offsets[index-1] - meanOffset);
         if (column == 9 || column == 27 || column == 81 || column == 99){
            for (unsigned int row = 1; row <= 5; row++){
               index = GetCCDBIndexTAGM(column, row);
               valueToUse = selectedTAGMOffset->GetBinContent(index);
               if (useRF) valueToUse *= RF_Period;
               //if (valueToUse == 0) valueToUse = meanOffset;
               outFile << row << " " << column << " " << valueToUse + tagm_tdc_time_offsets[index-1] - meanOffset << endl;
               if (verbose) printf("%i\t%i\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n", row, column, valueToUse, tagm_tdc_time_offsets[index-1], meanOffset,
                     valueToUse + tagm_tdc_time_offsets[index-1] - meanOffset);
            }
         }
      }
      outFile.close();
      outFile.open(prefix + "tagm_base_time.txt", ios::out);
      if (verbose) {
         printf("TAGM ADC Base = %f - (%f) = %f\n", tagm_t_base_fadc, meanOffset, tagm_t_base_fadc - meanOffset);
         printf("TAGM TDC Base = %f - (%f) = %f\n", tagm_t_base_tdc, meanOffset, tagm_t_base_tdc - meanOffset);
      }
      outFile << tagm_t_base_fadc - meanOffset << " " << tagm_t_base_tdc - meanOffset << endl;
      outFile.close();

   }
   ***/


   // TAGM base time calibration
   TH1F *tagmRFalignHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "TAGM - RFBunch 1D Time");
   if(tagmRFalignHist != NULL) {
     double maximum = tagmRFalignHist->GetBinCenter(tagmRFalignHist->GetMaximumBin());
     TFitResultPtr fr = tagmRFalignHist->Fit("gaus", "SQ", "", maximum - 0.3, maximum + 0.3);
     double meanOffset = fr->Parameter(1);
   
     outFile.open(prefix + "tagm_base_time.txt", ios::out);
     if (verbose) {
       printf("TAGM ADC Base = %f - (%f) = %f\n", tagm_t_base_fadc, meanOffset, tagm_t_base_fadc - meanOffset);
       printf("TAGM TDC Base = %f - (%f) = %f\n", tagm_t_base_tdc, meanOffset, tagm_t_base_tdc - meanOffset);
     }
     outFile << tagm_t_base_fadc - meanOffset << " " << tagm_t_base_tdc - meanOffset << endl;
     outFile.close();
   }



   thisHist = ExtractTrackBasedTimingNS::Get2DHistogram("HLDetectorTiming", "TRACKING", "TAGH - SC Target Time");
   if (useRF) thisHist = ExtractTrackBasedTimingNS::Get2DHistogram("HLDetectorTiming", "TRACKING", "TAGH - RFBunch Time");
   if (thisHist != NULL) {
      outFile.open(prefix + "tagh_tdc_timing_offsets.txt", ios::out | ios::trunc);
      outFile.close(); // clear file
      //outFile.open(prefix + "tagh_adc_timing_offsets.txt", ios::out | ios::trunc);   // KEEP THESE FIXED FOR NOW
      //outFile.close(); // clear file

      int nBinsX = thisHist->GetNbinsX();
      //double taghAdcOffsets[274] = { 0. };
      //double taghTdcOffsets[274] = { 0. };
      outFile.open(prefix + "tagh_tdc_timing_offsets.txt");
      for (int i = 1; i <= nBinsX; i++) {
	      if (tagh_counter_quality[i-1] == 0.0) {
		      outFile << i << " " << 0 << endl;
		      continue;
	      }

	      TH1D *projY = thisHist->ProjectionY("temp", i, i);
	      double delta = 0.;
	      if(projY->Integral() > 0.) {
		      double maximum = projY->GetBinCenter(projY->GetMaximumBin());
		      TFitResultPtr fr = projY->Fit("gaus", "QSQ", "", maximum - 0.3, maximum + 0.3);
		      delta = fr->Parameter(1);

		      // check for bad fit
		      if(delta < -37.)
			      delta = 0.;
		      if(fabs(delta) < 0.1)
			      delta = 0.;
	      }

	      double ccdb = tagh_tdc_time_offsets[i-1];
	      double offset = ccdb + delta;
	      
	      outFile << i << " " << offset << endl;
	      if (verbose) printf("TDC\t%i\t%.3f\t\t%.3f\t\t%.3f\n", i, delta, ccdb, offset);
      }
      outFile.close();

   }


   // TAGH base time calibration
   TH1F *taghRFalignHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "Tagger - RFBunch 1D Time");
   if(taghRFalignHist != NULL) {
     double maximum = taghRFalignHist->GetBinCenter(taghRFalignHist->GetMaximumBin());
     TFitResultPtr fr = taghRFalignHist->Fit("gaus", "SQ", "", maximum - 0.3, maximum + 0.3);
     double meanOffset = fr->Parameter(1);
   
     outFile.open(prefix + "tagh_base_time.txt", ios::out);
     if (verbose) {
       printf("TAGH ADC Base = %f - (%f) = %f\n", tagh_t_base_fadc, meanOffset, tagh_t_base_fadc - meanOffset);
       printf("TAGH TDC Base = %f - (%f) = %f\n", tagh_t_base_tdc, meanOffset, tagh_t_base_tdc - meanOffset);
     }
     outFile << tagh_t_base_fadc - meanOffset << " " << tagh_t_base_tdc - meanOffset << endl;
     outFile.close();
   }


      /* 
      // Also realign ADCs and TDCs
      // assuming anything larger than 28 ns is due to a TDC shift, otherwise the ADCs are shifted
      TH2F *taghADCTDCHist = ExtractTrackBasedTimingNS::Get2DHistogram("HLDetectorTiming", "TAGH", "TAGHHit TDC_ADC Difference");
      int nBinsX = thisHist->GetNbinsX();
      double taghAdcOffsets[274] = { 0. };
      double taghTdcOffsets[274] = { 0. };

      //TH1D * selectedTAGHOffset = new TH1D("selectedTAGHOffset", "Selected TAGH Offset; Column; Offset [ns]", nBinsX, 0.5, nBinsX + 0.5);
      for (int i = 1 ; i <= nBinsX; i++) {
	TH1D *projY = taghADCTDCHist->ProjectionY("temp", i, i);
	// Scan over histogram to find mean offset in timeWindow with largest integral
	double nsPerBin = (projY->GetBinCenter(projY->GetNbinsX()) - projY->GetBinCenter(1)) / projY->GetNbinsX();
	double timeWindow = 2.0; //ns (Full Width)
	int binWindow = int(timeWindow / nsPerBin);
	double maxEntries = 0;
	double maxMean = 0;
	for (int j = 1 ; j <= projY->GetNbinsX();j++) {
	  int minBin = j;
	  int maxBin = (j + binWindow) <= projY->GetNbinsX() ? (j + binWindow) : projY->GetNbinsX();
	  double sum = 0, nEntries = 0;
	  for (int bin = minBin; bin <= maxBin; bin++) {
	    sum += projY->GetBinContent(bin) * projY->GetBinCenter(bin);
	    nEntries += projY->GetBinContent(bin);
	    if (bin == maxBin) {
	      if (nEntries > maxEntries) {
		maxMean = sum / nEntries;
		maxEntries = nEntries;
	      }
	    }
	  }
	}
	//selectedTAGHOffset->SetBinContent(i, maxMean);

	 // classify offset
	 if(fabs(maxMean) > 1.5) {    // only correct shifts more than 1.5 ns, to avoid specious shifts
	   if(fabs(maxMean) > 28.)
	     taghTdcOffsets[i-1] = maxMean;
	   else
	     taghAdcOffsets[i-1] = maxMean;
	 }
      }
      */

/*
      // Setup histogram for determining the most probable change in offset for each F1TDC slot
      // This is needed to account for the occasional uniform shift in offsets of the 32 counters in a slot
      // Disable this for now, it hasn't been working well...
      //const int NtdcSlots = 8;
      //TH1F * tdcDist[NtdcSlots];
      //for (int i = 1; i <= NtdcSlots; i++) {
      //   stringstream ss; ss << i;
      //   TString s = ss.str();
      //   double range = 500.0; double width = 0.1;
      //   int Nbins = range/width;
      //   double low = -0.5*range - 0.5*width;
      //   double high = 0.5*range - 0.5*width;
      //   tdcDist[i-1] = new TH1F("TAGHOffsetDistribution_"+s, "TAGH Offset (slot "+s+"); TAGH Offset [ns]; Entries", Nbins, low, high);
      // }

      //int nBinsX = thisHist->GetNbinsX();
      TH1D * selectedTAGHOffset = new TH1D("selectedTAGHOffset", "Selected TAGH Offset; ID; Offset [ns]", nBinsX, 0.5, nBinsX + 0.5);
      for (int i = 1 ; i <= nBinsX; i++) {
         TH1D *projY = thisHist->ProjectionY("temp", i, i);
         // Scan over histogram to find mean offset in timeWindow with largest integral
         // Choose the correct number of bins based on the histogram
         double nsPerBin = (projY->GetBinCenter(projY->GetNbinsX()) - projY->GetBinCenter(1)) / projY->GetNbinsX();
         double timeWindow = 2.0; // ns (Full Width)
         int binWindow = int(timeWindow / nsPerBin);

         double maxEntries = 0;
         double maxMean = 0;
         for (int j = 1; j <= projY->GetNbinsX(); j++) {
            int minBin = j;
            int maxBin = (j + binWindow) <= projY->GetNbinsX() ? (j + binWindow) : projY->GetNbinsX();
            double sum = 0; 
            double nEntries = 0;
            for (int bin = minBin; bin <= maxBin; bin++) {
               sum += projY->GetBinContent(bin) * projY->GetBinCenter(bin);
               nEntries += projY->GetBinContent(bin);
               if (bin == maxBin) {
                  if (nEntries > maxEntries) {
                     maxMean = sum / nEntries;
                     maxEntries = nEntries;
                  }
               }
            }
         }

         if (tagh_counter_quality[i-1] == 0.0) {
            selectedTAGHOffset->SetBinContent(i, 0);
            continue;
         }
         //int tdc_slot = GetF1TDCslotTAGH(i);
	 // if the shift is too large, it's probably due to a TDC module shifting, so ignore that
	 if(fabs(maxMean) > 28.) 
	   maxMean = 0.;
         if (useRF) {
            int beamBucket;
            if (maxMean >= 0) beamBucket = int((maxMean / RF_Period) + 0.5); // +0.5 to handle rounding correctly
            else beamBucket = int((maxMean / RF_Period) - 0.5);
            selectedTAGHOffset->SetBinContent(i, beamBucket);
            //if (maxEntries != 0.0) tdcDist[tdc_slot - 1]->Fill(beamBucket);
         } else {
            selectedTAGHOffset->SetBinContent(i, maxMean);
            //if (maxEntries != 0.0) tdcDist[tdc_slot - 1]->Fill(maxMean);
         }
      }
      // Most probable change in offset or beam bucket per F1TDC slot
      //double mpDelta[NtdcSlots];
      //for (int i = 1; i <= NtdcSlots; i++) {
      //   int mpBin = tdcDist[i-1]->GetMaximumBin();
      //   mpDelta[i-1] = (mpBin > 0) ? tdcDist[i-1]->GetBinCenter(mpBin) : 0.0;
      //   if (useRF) mpDelta[i-1] *= RF_Period;
      //   if (verbose) {
      //      cout << "TAGH most probable Offset = " << i << ", " << mpDelta[i-1] << endl;
      //   }
      // }

      if (verbose) {
         cout << "Dumping TAGH results...\n=======================================" << endl;
         //cout << "Type\tChannel\tvalueToUse\toldValue\tmpDelta\tTotal" << endl;
         cout << "Type\tChannel\tvalueToUse\toldValue\tTotal" << endl;
      }

      double limit = 2.5; // ns
      double ccdb_sum = 0.0;
      for (int i = 1; i <= nBinsX; i++) ccdb_sum += tagh_tdc_time_offsets[i-1];
      double c1_tdcOffset = 0.0;
      outFile.open(prefix + "tagh_tdc_timing_offsets.txt");
      for (int i = 1; i <= nBinsX; i++) {
         if (tagh_counter_quality[i-1] == 0.0) {
           outFile << i << " " << 0 << endl;
            continue;
         }
         //int tdc_slot = GetF1TDCslotTAGH(i);
         double delta = selectedTAGHOffset->GetBinContent(i);
         if (useRF) delta *= RF_Period;
         //if (ccdb_sum > 0.0 && fabs(delta - mpDelta[tdc_slot-1]) > limit) {
         //   delta = mpDelta[tdc_slot-1];
	 // }
         double ccdb = tagh_tdc_time_offsets[i-1];
         double offset = ccdb + delta - taghTdcOffsets[i-1];
         //if (i == 1) c1_tdcOffset = offset;
         //offset -= c1_tdcOffset;   // disable recentering
         outFile << i << " " << offset << endl;
         //if (verbose) printf("TDC\t%i\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n", i, delta, ccdb, mpDelta[tdc_slot-1], offset);
         if (verbose) printf("TDC\t%i\t%.3f\t\t%.3f\t\t%.3f\n", i, delta, ccdb, offset);
      }
      outFile.close();

      ccdb_sum = 0.0;
      for (int i = 1; i <= nBinsX; i++) ccdb_sum += tagh_fadc_time_offsets[i-1];
      double c1_adcOffset = 0.0;
      outFile.open(prefix + "tagh_adc_timing_offsets.txt");
      for (int i = 1; i <= nBinsX; i++) {
         if (tagh_counter_quality[i-1] == 0.0) {
            outFile << i << " " << 0 << endl;
            continue;
         }
         //int tdc_slot = GetF1TDCslotTAGH(i);
         double delta = selectedTAGHOffset->GetBinContent(i);
         if (useRF) delta *= RF_Period;
         //if (ccdb_sum > 0.0 && fabs(delta - mpDelta[tdc_slot-1]) > limit) {
         //   delta = mpDelta[tdc_slot-1];
	 // }
         double ccdb = tagh_fadc_time_offsets[i-1];
         double offset = ccdb + delta - taghAdcOffsets[i-1];
         if (i == 1) c1_adcOffset = offset;
         //offset -= c1_adcOffset;   // disable recentering
         outFile << i << " " << offset << endl;
         //if (verbose) printf("ADC\t%i\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n", i, delta, ccdb, mpDelta[tdc_slot-1], offset);
         if (verbose) printf("ADC\t%i\t%.3f\t\t%.3f\t\t%.3f\n", i, delta, ccdb, offset);
      }
      outFile.close();

      //outFile.open(prefix + "tagh_base_time.txt");
      outFile << tagh_t_base_fadc - c1_adcOffset << " " << tagh_t_base_tdc - c1_tdcOffset << endl;
      if (verbose) {
         printf("TAGH ADC Base = %f - (%f) = %f\n", tagh_t_base_fadc, c1_adcOffset, tagh_t_base_fadc - c1_adcOffset);
         printf("TAGH TDC Base = %f - (%f) = %f\n", tagh_t_base_tdc, c1_tdcOffset, tagh_t_base_tdc - c1_tdcOffset);
      }
      outFile.close();
   }
*/

   // We can use the RF time to calibrate the SC time 
   double meanSCOffset = 0.0; // In case we change the time of the SC, we need this in this scope
   if(useRF){
      TH1F * selectedSCSectorOffset = new TH1F("selectedSCSectorOffset", "Selected TDC-RF offset;Sector; Time", 30, 0.5, 30.5);
      TH1F * selectedSCSectorOffsetDistribution = new TH1F("selectedSCSectorOffsetDistribution", "Selected TDC-RF offset;Time;Entries", 100, -3.0, 3.0);
      TF1* f = new TF1("f","pol0(0)+gaus(1)", -3.0, 3.0);

      // Also realign ADCs and TDCs
      // assuming anything larger than 28 ns is due to a TDC shift, otherwise the ADCs are shifted
      double scAdcOffsets[30] = { 0. };
      double scTdcOffsets[30] = { 0. };
      TH2F *scADCTDCHist = ExtractTrackBasedTimingNS::Get2DHistogram("HLDetectorTiming", "SC", "SCHit TDC_ADC Difference");

      for (int sector = 1; sector <= 30; sector++){
	 // first realign ADCs and TDCs
	 TH1D *projY = scADCTDCHist->ProjectionY("temp", sector, sector);
	 float nsPerBin = (projY->GetBinCenter(projY->GetNbinsX()) - projY->GetBinCenter(1)) / projY->GetNbinsX();
	 float timeWindow = 2; //ns (Full Width)
	 int binWindow = int(timeWindow / nsPerBin);
	 double maxEntries = 0;
	 double maxMean = 0;
	 for (int j = 1 ; j <= projY->GetNbinsX();j++){
	   int minBin = j;
	   int maxBin = (j + binWindow) <= projY->GetNbinsX() ? (j + binWindow) : projY->GetNbinsX();
	   double sum = 0, nEntries = 0;
	   for (int bin = minBin; bin <= maxBin; bin++){
	     sum += projY->GetBinContent(bin) * projY->GetBinCenter(bin);
	     nEntries += projY->GetBinContent(bin);
	     if (bin == maxBin){
	       if (nEntries > maxEntries) {
		 maxMean = sum / nEntries;
		 maxEntries = nEntries;
	       }
	     }
	   }
	 }

	 // classify offset
	 if(fabs(maxMean) > 1.5) {    // only correct shifts more than 1.5 ns, to avoid specious shifts
	   if(fabs(maxMean) > 28.)
	     scTdcOffsets[sector-1] = maxMean;
	   else
	     scAdcOffsets[sector-1] = maxMean;
	 }

	 // now align RF times
     if(!DONT_SHIFT_SC_RF) { 
         TH1F *scRFHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "SC_Target_RF_Compare", Form("Sector %.2i", sector));
         if (scRFHist == NULL) continue;
         //Do the fit
         TFitResultPtr fr = scRFHist->Fit("pol0", "SQ", "", -2, 2);
         double p0 = fr->Parameter(0);
         
         f->FixParameter(0,p0);
         f->SetParLimits(2, -2, 2);
         f->SetParLimits(3, 0, 2);
         f->SetParameter(1, 10);
         f->SetParameter(2, scRFHist->GetBinCenter(scRFHist->GetMaximumBin()));
         f->SetParameter(3, 0);
         
         fr = scRFHist->Fit(f, "SQ", "", -2, 2);
         double SCOffset = fr->Parameter(2);
         selectedSCSectorOffset->SetBinContent(sector, SCOffset);
         selectedSCSectorOffsetDistribution->Fill(SCOffset);
     }
      }

      // Now write out the offsets
      meanSCOffset = selectedSCSectorOffsetDistribution->GetMean();
      // Move mean SC time around if we want it to end up in a particular location
      if(FIX_START_COUNTER) {  // experimental
	TH1F *scHitTimeHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "SC", "SCHitMatchedTime");
	double scMax = scHitTimeHist->GetBinCenter(scHitTimeHist->GetMaximumBin());
	TFitResultPtr fr = scHitTimeHist->Fit("gaus", "S", "", scMax - 5., scMax + 5.);

	meanSCOffset = fr->Parameter(0) - meanSCOffset - start_counter_t0;   // this will get it close, need to make it line up
      }
      if (verbose){
         cout << "Dumping SC results...\n=======================================" << endl;
         cout << "SC mean Offset = " << meanSCOffset << endl;
         cout << "TDC Offsets" << endl;
         cout << "Sector\toldValue\tValueToUse\tmeanOffset\tTotal" << endl;
      }
      outFile.open(prefix + "sc_tdc_timing_offsets.txt");
      for (int sector = 1; sector <= 30; sector++){
         outFile << sc_tdc_time_offsets[sector-1] + scTdcOffsets[sector-1] + selectedSCSectorOffset->GetBinContent(sector) - meanSCOffset << endl;
         if (verbose) printf("%i\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n",sector, sc_tdc_time_offsets[sector-1], selectedSCSectorOffset->GetBinContent(sector), meanSCOffset,
               sc_tdc_time_offsets[sector-1] - scTdcOffsets[sector-1] + selectedSCSectorOffset->GetBinContent(sector) - meanSCOffset);
      }
      outFile.close();
      if (verbose){
         cout << "ADC Offsets" << endl;
         cout << "Sector\tvalueToUse\toldValue\tmeanOffset\tTotal" << endl;
      }
      outFile.open(prefix + "sc_adc_timing_offsets.txt");
      for (int sector = 1; sector <= 30; sector++){
         outFile << sc_fadc_time_offsets[sector-1] + scAdcOffsets[sector-1] + selectedSCSectorOffset->GetBinContent(sector) - meanSCOffset << endl;
         if (verbose) printf("%i\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n",sector,sc_fadc_time_offsets[sector-1], selectedSCSectorOffset->GetBinContent(sector), meanSCOffset,
               sc_fadc_time_offsets[sector-1] - scAdcOffsets[sector-1] + selectedSCSectorOffset->GetBinContent(sector) - meanSCOffset);
      }
      outFile.close();

      outFile.open(prefix + "sc_base_time.txt");
      outFile << sc_t_base_fadc - meanSCOffset << " " << sc_t_base_tdc - meanSCOffset << endl;
      if (verbose) {
         printf("SC ADC Base = %f - (%f) = %f\n", sc_t_base_fadc, meanSCOffset, sc_t_base_fadc - meanSCOffset);
         printf("SC TDC Base = %f - (%f) = %f\n", sc_t_base_tdc, meanSCOffset, sc_t_base_tdc - meanSCOffset);
      }
      outFile.close();
   }

   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "TOF - RF Time");
   if(this1DHist != NULL){
      //Gaussian
      Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
      TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 0.2, maximum + 0.2);
      float mean = fr->Parameter(1);
      outFile.open(prefix + "tof_base_time.txt");
      if (verbose) {
         printf("TOF ADC Base = %f - (%f) - (%f) = %f\n", tof_t_base_fadc, mean, meanSCOffset, tof_t_base_fadc - mean - meanSCOffset);
         printf("TOF TDC Base = %f - (%f) - (%f) = %f\n", tof_t_base_tdc, mean, meanSCOffset, tof_t_base_tdc - mean - meanSCOffset);
      }
      outFile << tof_t_base_fadc - mean - meanSCOffset<< " " << tof_t_base_tdc - mean - meanSCOffset<< endl;
      outFile.close();
   }

   if(USE_NEUTRALS_FOR_BCAL_RF_ALIGN)
	   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "BCAL - RF Time (Neutral)");
   else
	   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "BCAL - RF Time");
   if(this1DHist != NULL){
      //Gaussian
      Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
      TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 0.55, maximum + 0.55);
      float mean = fr->Parameter(1);
      outFile.open(prefix + "bcal_base_time.txt");
      if (verbose) {
         printf("BCAL ADC Base = %f - (%f) - (%f) = %f\n", bcal_t_base_fadc, mean, meanSCOffset, bcal_t_base_fadc - mean - meanSCOffset);
         printf("BCAL TDC Base = %f - (%f) - (%f) = %f\n", bcal_t_base_tdc, mean, meanSCOffset, bcal_t_base_tdc - mean - meanSCOffset);
      }
      outFile << bcal_t_base_fadc - mean - meanSCOffset << " " << bcal_t_base_tdc - mean - meanSCOffset << endl; // TDC info not used
      outFile.close();
   }

   if(USE_NEUTRALS_FOR_ECAL_RF_ALIGN)
	   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "ECAL - RF Time (Neutral)");
   else
	   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "ECAL - RF Time");
   if(this1DHist != NULL){
      //Gaussian
      Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
      TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 0.4, maximum + 0.4);
      float mean = fr->Parameter(1);
      outFile.open(prefix + "ecal_base_time.txt");
      if (verbose) {
         printf("ECAL ADC Base = %f - (%f) - (%f) = %f\n",fcal_t_base, mean, meanSCOffset, fcal_t_base - mean - meanSCOffset);
      }
      outFile << ecal_t_base - mean - meanSCOffset<< endl; 
      outFile.close();
   }

   if(USE_NEUTRALS_FOR_FCAL_RF_ALIGN)
	   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "FCAL - RF Time (Neutral)");
   else
	   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "FCAL - RF Time");
   if(this1DHist != NULL){
      //Gaussian
      Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
      TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 0.5, maximum + 0.5);
      float mean = fr->Parameter(1);
      outFile.open(prefix + "fcal_base_time.txt");
      if (verbose) {
         printf("FCAL ADC Base = %f - (%f) - (%f) = %f\n",fcal_t_base, mean, meanSCOffset, fcal_t_base - mean - meanSCOffset);
      }
      outFile << fcal_t_base - mean - meanSCOffset<< endl; 
      outFile.close();
   }

   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "Earliest CDC Time Minus Matched SC Time");
   if(this1DHist != NULL){
      //Gaussian
      Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
      TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 10, maximum + 6);
      float mean = fr->Parameter(1);
      outFile.open(prefix + "cdc_base_time.txt");
      if (verbose) {
         printf("CDC ADC Base = %f - (%f) - (%f) = %f\n",cdc_t_base, mean, meanSCOffset, cdc_t_base - mean - meanSCOffset);
      }
      outFile << cdc_t_base - mean - meanSCOffset << endl;
      outFile.close();
   }
   
   // We want to account for any residual difference between the cathode and anode times.
   double FDC_ADC_Offset = 0.0, FDC_TDC_Offset = 0.0; 
   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "FDC", "FDCHit Cathode time;1");
    if(this1DHist != NULL){
        Int_t firstBin = this1DHist->FindFirstBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
        // extended range due to extra FDC noise - sdobbs, 4/9/2018
        for (int i = 0; i <= 46; i++){
            if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
        }
        //Fit a gaussian to the left of the main peak
        Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());

        //cout << "FDC max = " << maximum << endl;
        //cout << "FDC first = " << firstBin << endl;
        //this1DHist->Print("all");

        TF1 *f = new TF1("f", "gaus");
        f->SetParameters(100, maximum, 20);
        //this1DHist->Rebin(2);
        TFitResultPtr fr = this1DHist->Fit(f, "S", "", maximum - 10, maximum + 7); // Cant fix value at end of range
        double mean = fr->Parameter(1);
        float sigma = fr->Parameter(2);
        FDC_ADC_Offset = mean;
        //FDC_ADC_Offset = mean - 10.;   // HACK 
        delete f;
    }

    this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "FDC", "FDCHit Wire time;1");
    if(this1DHist != NULL){
        Int_t firstBin = this1DHist->FindLastBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
        for (int i = 0; i <= 25; i++){
            if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
        }
        //Fit a gaussian to the left of the main peak
        Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
        TF1 *f = new TF1("f", "gaus");
        f->SetParameters(100, maximum, 20);
        TFitResultPtr fr = this1DHist->Fit(f, "S", "", maximum - 10, maximum + 5); // Cant fix value at end of range
        double mean = fr->Parameter(1);
        float sigma = fr->Parameter(2);
        FDC_TDC_Offset = mean;
        //FDC_TDC_Offset = mean - 10.;   // HACK 
        delete f;
    }
    double FDC_ADC_TDC_Offset = FDC_ADC_Offset - FDC_TDC_Offset;

   this1DHist = ExtractTrackBasedTimingNS::Get1DHistogram("HLDetectorTiming", "TRACKING", "Earliest Flight-time Corrected FDC Time");
   float MPV = 0;
   if(this1DHist != NULL){
      //Landau
      Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
      TFitResultPtr fr = this1DHist->Fit("landau", "S", "", maximum - 2.5, maximum + 4);
      //TFitResultPtr fr = this1DHist->Fit("landau", "S", "", maximum - 3.5, maximum + 6);
      //TFitResultPtr fr = this1DHist->Fit("landau", "S", "", maximum - 2., maximum + 6);
      //float MPV = fr->Parameter(1);
      MPV = fr->Parameter(1);
   }

   //cerr << "*** MPV = " << MPV << endl;

   // see if we need to correct any FDC individual wire times.  generally, assume that most of the detector is in time
   // so the main wire timing peak is correct.  then if any TDC modules deviate from this, then align them to the main peak
   thisHist = ExtractTrackBasedTimingNS::Get2DHistogram("HLDetectorTiming", "FDC", "FDCHit Wire time vs. module");
   double avg_FDC_TDC_wire_offsets = 0;
   if(thisHist != NULL){
     bool package_times_shifted[4] = { false, false, false, false };
     double FDC_wire_offsets[4][6][96] = { 0. };
     for(int plane=1; plane<=24; plane++) {
       // check one half of the plane
       char buf[50];
       sprintf(buf,"temp_fdc_%d", 2*plane-1);
       TH1D *projY = thisHist->ProjectionY(buf, 2*plane-1, 2*plane-1);
       cout << " plane " << plane << endl;
       //TH1D *projY = thisHist->ProjectionY("temp", 2*plane-1, 2*plane-1);
       //Int_t firstBin = projY->FindLastBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
       //for (int i = 0; i <= 15; i++){
       // if ((firstBin + i) > 0) projY->SetBinContent((firstBin + i), 0);
       //}
       //Fit a gaussian to the left of the main peak
       Double_t maximum = projY->GetBinCenter(projY->GetMaximumBin());
       double mean1 = 40.;
       float sigma1 = 0.;
       TF1 *f = new TF1("f", "gaus");
       
       if(maximum > -190.) {
           mean1 = maximum - MPV;
           //cout << mean1 << " " << maximum << " " << MPV << endl;
           // Disable fits for now, not terribly stable
           //TF1 *f = new TF1("f", "gaus");
           TF1 fg("fg", "gaus");
           fg.SetParameters(100, maximum, 20);
           TFitResultPtr fr = projY->Fit(&fg, "S", "", maximum - 10, maximum + 8); // Cant fix value at end of range
           mean1 = fr->Parameter(1) - MPV;
           sigma1 = fr->Parameter(2);
           //cerr << " mean = " << mean1 << endl;
           //delete f;
       }


       // now check the other half
       sprintf(buf,"temp_fdc_%d", 2*plane);
       projY = thisHist->ProjectionY(buf, 2*plane, 2*plane);
       //projY = thisHist->ProjectionY("temp", 2*plane, 2*plane);
       //firstBin = projY->FindLastBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
       //for (int i = 0; i <= 15; i++){
       //	 if ((firstBin + i) > 0) projY->SetBinContent((firstBin + i), 0);
       //}
       //Fit a gaussian to the left of the main peak
       maximum = projY->GetBinCenter(projY->GetMaximumBin());
       double mean2 = 40.;
       float sigma2 = 0.;

       if(maximum > -190.) {
           mean2 = maximum - MPV;
           //cout << mean2 << " " << maximum << " " << MPV << endl;
           // Disable fits for now, not terribly stable 
           //f = new TF1("f", "gaus");
           TF1 fg2("fg2", "gaus");
           fg2.SetParameters(100, maximum, 20);
           TFitResultPtr fr2 = projY->Fit(&fg2, "S", "", maximum - 10, maximum + 8); // Cant fix value at end of range
           mean2 = fr2->Parameter(1) - MPV;
           sigma2 = fr2->Parameter(2);
           //cerr << " mean = " << mean2 << endl;
           //delete f;
       }
       
       int package = 0;
       int chamber = 0;
       FindFDCPackageChamber(plane, package, chamber);

       package_times_shifted[package-1] = true;
       
       // handle TDC modules separately
       for(int wire=0; wire<48; wire++) {
	       // only correct wire shifts greater than some amount
	       //if(fabs(mean1) > 8.) {
           FDC_wire_offsets[package-1][chamber-1][wire] = mean1 + old_FDC_wire_offsets[package-1][chamber-1][wire];
	       //} else {
		   //    FDC_wire_offsets[package-1][chamber-1][wire] = old_FDC_wire_offsets[package-1][chamber-1][wire];
	       //}
       }

       for(int wire=48; wire<96; wire++) {
	       // only correct wire shifts greater than some amount
	       //if(fabs(mean2) > 8.) {
           FDC_wire_offsets[package-1][chamber-1][wire] = mean2 + old_FDC_wire_offsets[package-1][chamber-1][wire];
	       //} else {
           //FDC_wire_offsets[package-1][chamber-1][wire] = old_FDC_wire_offsets[package-1][chamber-1][wire];
	       //}
       }

       // keep an average on a per-module basis
       avg_FDC_TDC_wire_offsets += FDC_wire_offsets[package-1][chamber-1][1];
       avg_FDC_TDC_wire_offsets += FDC_wire_offsets[package-1][chamber-1][50];
     }

     // make the average 
     avg_FDC_TDC_wire_offsets /= 48.;

     // now write out anything we need
     for(int package = 0; package<4; package++) {
       if(package_times_shifted[package]) {
	 char buf[50];
	 sprintf(buf, "%sfdc_package%d_wire_offsets.txt", prefix.Data(), package+1);
	 outFile.open(buf);
	 for(int chamber=0; chamber<6; chamber++) {
	   for(int wire=0; wire<96; wire++) {
		   outFile << (FDC_wire_offsets[package][chamber][wire] - avg_FDC_TDC_wire_offsets)/2. << " ";
	   }
	   outFile << endl;
	 }
	 outFile.close();
       }
     }
   }

   // now we can finally write out the FDC offsets
   outFile.open(prefix + "fdc_base_time.txt");
   if (verbose) {
	   printf("FDC ADC Base = %f - (%f) - (%f) - (%f) = %f\n",fdc_t_base_fadc, MPV, meanSCOffset, FDC_ADC_TDC_Offset, fdc_t_base_fadc - MPV - meanSCOffset - FDC_ADC_TDC_Offset);
	   printf("FDC TDC Base = %f - (%f) - (%f) = %f\n",fdc_t_base_tdc, MPV, meanSCOffset, fdc_t_base_tdc - MPV - meanSCOffset);
   }
   //outFile << fdc_t_base_fadc - MPV - meanSCOffset - (FDC_ADC_TDC_Offset-avg_FDC_TDC_wire_offsets) << " " << fdc_t_base_tdc - MPV - avg_FDC_TDC_wire_offsets - meanSCOffset << endl;
   //outFile << fdc_t_base_fadc + 5. - MPV - meanSCOffset - FDC_ADC_Offset << " " << fdc_t_base_tdc + 5. - MPV - avg_FDC_TDC_wire_offsets - meanSCOffset << endl;
   //outFile << fdc_t_base_fadc - MPV - meanSCOffset - FDC_ADC_TDC_Offset << " " << fdc_t_base_tdc - avg_FDC_TDC_wire_offsets - meanSCOffset << endl;
   outFile << fdc_t_base_fadc - MPV - FDC_ADC_TDC_Offset << " " << fdc_t_base_tdc - MPV << endl;
   outFile.close();
   
   
   cout << "FDC ADC: " << fdc_t_base_fadc << " " <<  MPV << " " <<  meanSCOffset << " "<< FDC_ADC_Offset << " "
	<< (FDC_ADC_TDC_Offset-avg_FDC_TDC_wire_offsets) << " "
	<< (fdc_t_base_fadc - MPV - meanSCOffset - FDC_ADC_TDC_Offset) << endl;
   cout << "FDC TDC: " << fdc_t_base_tdc << " " << MPV << " " << avg_FDC_TDC_wire_offsets << " " << meanSCOffset << " "
	<< (fdc_t_base_tdc - MPV - avg_FDC_TDC_wire_offsets - meanSCOffset) << endl;
   

   ExtractTrackBasedTimingNS::thisFile->Write();
   ExtractTrackBasedTimingNS::thisFile->Close();

   return;
}
