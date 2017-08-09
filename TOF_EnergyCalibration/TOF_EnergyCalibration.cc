void TOF_EnergyCalibration(int option)
{
	// Read from Input File / Write to Output File
	std::fstream txt_file;
	std::fstream MCtxt_file;
	txt_file.open("my_constants.txt", std::fstream::in | std::fstream::out | std::fstream::app);
	MCtxt_file.open("my_MCconstants.txt", std::fstream::in | std::fstream::out | std::fstream::app);

	// Get/Create ROOT Files
	TFile * inputROOTFile;
	TFile * outputROOTFile;
	TDirectory * top = gDirectory;
	top->cd();
	if(option == 1)
	{
		cout<< "\nREADING BENI's File...\n";
		inputROOTFile = new TFile("/work/halld/home/zihlmann/TOF_calib/CAL3/calibration31036/adchists_run31036.root","READ");
		outputROOTFile = new TFile("TOF_EnergyCalibration.root","RECREATE");
	}
	if(option == 2)
	{
		cout<< "\nREADING MC File...\n";
		inputROOTFile = new TFile("./TOF_EnergyCalibration/TOF_EnergyCalibrationMC.root","READ");
		outputROOTFile = new TFile("./TOF_EnergyCalibration/TOF_EnergyCalibrationMC_FIT.root","RECREATE");
	}

	// Arrays to be used for overall statistics and fits
	Double_t adcMPV[176];
	Double_t pmtNUM[176];
	Double_t chisq[176];

	// Get histograms
	char histname[128];
	for (int pmt = 0; pmt < 176; ++pmt)
	{
		// Get and clone histogram from Beni's root file
		sprintf(histname,"ADCHists%d",pmt);
		TH1D * ADCHists = (TH1D*)inputROOTFile->Get(histname); 
		TH1D * ADCHist_clone = (TH1D*)ADCHists->Clone();

		// Find Fit Range
		TSpectrum * t_spectrum = new TSpectrum(2);
		t_spectrum->Search(ADCHist_clone);
		Double_t * x_pos = t_spectrum->GetPositionX();
		Double_t * y_pos = t_spectrum->GetPositionY();
		Double_t y_max = (y_pos[0] > y_pos[1]) ? y_pos[0] : y_pos[1];
		Double_t x_max = (x_pos[0] > x_pos[1]) ? x_pos[0] : x_pos[1];
		Double_t x_min = (x_pos[0] > x_pos[1]) ? x_pos[1] : x_pos[0];
		Double_t minimum = 1000000;
		Double_t begin_fit;
		for (Int_t loc_i = ADCHist_clone->FindBin(x_min); loc_i < ADCHist_clone->FindBin(x_max); ++loc_i)
		{
			if(ADCHist_clone->GetBinContent(loc_i) < minimum)
			{
				minimum = ADCHist_clone->GetBinContent(loc_i);
				begin_fit = ADCHist_clone->GetBinCenter(loc_i);
			}
		}

		// Make sure you are not fitting the double hit enhancement
		if ((x_pos[1] > x_pos[0]) && (x_pos[0] > 2000.))
		{
			begin_fit = begin_fit - 1.1*x_pos[0];
			x_max = x_pos[0];
		}

		// Fit Hist with landau distribution
		Double_t end_fit = 2*x_max - begin_fit;
		Double_t sigma_fit = end_fit-begin_fit;
		TF1 * landau_fit;
		if(option == 1)
		{	
			landau_fit = new TF1("landau_fit","[0]*TMath::Landau(x,[1],[2])",begin_fit*1.1,end_fit*1.1);
			landau_fit->SetParameters(y_max,x_max,sigma_fit);
			ADCHist_clone->Fit("landau_fit","","",begin_fit*1.1,end_fit);
		}
		if(option == 2)
		{	
			y_max = (y_pos[0] > y_pos[1]) ? y_pos[1] : y_pos[0];
			landau_fit = new TF1("landau_fit","[0]*TMath::Landau(x,[1],[2])",0.0016,0.0022);
			landau_fit->SetParameters(y_max*5.,0.0019,0.0002);
			ADCHist_clone->Fit("landau_fit","","",0.0016,0.0022);
		}

		// Fix and issued with fit near 'begin_fit'
		if ((landau_fit->Eval(begin_fit) < ADCHist_clone->GetBinContent(ADCHist_clone->FindBin(begin_fit))) && (option != 2)) 
		{
			Int_t begin_bin = ADCHist_clone->FindBin(begin_fit);
			while (landau_fit->Eval(begin_fit) < 0.9*ADCHist_clone->GetBinContent(begin_bin)) {
				++begin_bin;
				begin_fit = ADCHist_clone->GetBinCenter(begin_bin);
			}
			landau_fit->SetParameters(y_max,x_max,sigma_fit);
			ADCHist_clone->Fit("landau_fit","","",begin_fit,end_fit);
		}

		// Collect statistics for TGraphs
		adcMPV[pmt] = landau_fit->GetParameter(1);
		pmtNUM[pmt] = pmt + 1;
		chisq[pmt] = landau_fit->GetChisquare();

		// Write to text file
		if (option == 1)
		{
			float adcOLD;
			MCtxt_file >> adcOLD;
			txt_file << "  " << adcOLD / adcMPV[pmt] << "\n";
		}
		if (option == 2)
			MCtxt_file << "  " << adcMPV[pmt] << "\n";

		// Write final result to output root file
		outputROOTFile->cd();
		ADCHist_clone->Write();
		top->cd();
	} // for(pmt)

	// TGraphs of overall statistics
	outputROOTFile->cd();
	TGraph * adcMPV_TGraph = new TGraph(176, pmtNUM, adcMPV); 
	TGraph * chisq_TGraph = new TGraph(176, pmtNUM, chisq);
	adcMPV_TGraph->SetTitle("ADC MPV");
	chisq_TGraph->SetTitle("Chi Square of Fits");
	adcMPV_TGraph->SetName("ADC MPV");
	chisq_TGraph->SetName("Chi Square of Fits");
	adcMPV_TGraph->SetLineWidth(0);
	chisq_TGraph->SetLineWidth(0);
	adcMPV_TGraph->SetMarkerColor(kBlue);
	chisq_TGraph->SetMarkerColor(kBlue);
  adcMPV_TGraph->SetMarkerSize(0.5);
	chisq_TGraph->SetMarkerSize(0.5);
  adcMPV_TGraph->SetMarkerStyle(21);
	chisq_TGraph->SetMarkerStyle(21);
	adcMPV_TGraph->GetXaxis()->SetTitle("PMT Number");
	chisq_TGraph->SetTitle("PMT Number");
  adcMPV_TGraph->GetYaxis()->SetTitle("MPV From Fit");
	chisq_TGraph->GetYaxis()->SetTitle("Chi sq From Fit");

	adcMPV_TGraph->Draw();
	chisq_TGraph->Draw();
	adcMPV_TGraph->Write();
	chisq_TGraph->Write();
	top->cd();
	
	// Close output file
	outputROOTFile->Close();
	txt_file.close();
}

