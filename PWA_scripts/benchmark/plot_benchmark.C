#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <bits/stdc++.h>
#include <string>
#include <sstream>

void plot_benchmark(TString dir = "./") {

	gStyle->SetOptStat(0);

	// initialize list of nCores to plot
	vector<int> numThreadsCPU = {1,2,4,8,16,32,64,96,128};
	int numTestCPU = numThreadsCPU.size();

	// for GPU fits, only add if desired
	vector<int> numThreadsGPUT4 = {1,2,3,4};
	vector<int> numThreadsGPURTX = {};

	// names of directories containing benchmark results
	vector<TString> types = {"cpu"};
	vector<TGraphErrors*> grBenchmarkScan;
	if(numThreadsGPUT4.size() > 0) types.push_back("gpuT4");
	if(numThreadsGPURTX.size() > 0) types.push_back("gpuTitanRTX");

	TH1F *hBenchmarkScan = new TH1F("hBenchmarkScan","; Number of GPUs or CPUs; Fit speed (Likelihood function call rate [Hz])", 200, 0, 200);
	double maxRate = 0;
	
	for(int itype=0; itype<types.size(); itype++) {
		vector<int> numThreads = numThreadsCPU;
		if(types[itype] == "gpuT4") numThreads = numThreadsGPUT4;
		if(types[itype] == "gpuTitanRTX") numThreads = numThreadsGPURTX;
		grBenchmarkScan.push_back(new TGraphErrors(numThreads.size()));

		// loop over number of threads in test
		for(int ithread=0; ithread<numThreads.size(); ithread++) {
			
			int nThreads = numThreads[ithread];
			string spath = Form("%s/%s%03d/log/fit.out", dir.Data(), types[itype].Data(), nThreads);	
			cout << spath << endl;	
			
			std::string read_line;
			ifstream file(spath);
			double parValue = 0;
			double parAvg = 0;
			vector<double> parSq;
			int nValues = 0;
			while (std::getline(file, read_line)) {

				TString line = read_line;
				if(line.Contains("time ")) {
					line.ReplaceAll("average time per function call:  ","");
					line.ReplaceAll(" ms.","");
					parValue = 1./(atof(line)/1000);
					parAvg += parValue;
					parSq.push_back(parValue*parValue);
					nValues++;
				}
				else continue;

			}

			if(nValues > 0) {
				parAvg /= float(nValues);
				double parRms = 0;
				for(uint ip=0; ip<parSq.size(); ip++)
					parRms += (parSq.at(ip) + parAvg*parAvg - 2*sqrt(parSq.at(ip))*parAvg);
				parRms /= float(nValues);
				parRms = sqrt(parRms);
				if(parAvg > maxRate) maxRate = parAvg;
				cout<<parAvg<<" "<<parRms<<endl;
				if(parRms < 1e-9) parRms = 0.01;
				grBenchmarkScan[itype]->SetPoint(ithread, nThreads, parAvg);
				grBenchmarkScan[itype]->SetPointError(ithread, 0, parRms);
			}
		}
	}
	
	TCanvas *cc = new TCanvas("cc","cc",800,400);
	auto legend = new TLegend(0.47,0.17,0.9,0.42);

	hBenchmarkScan->SetMaximum(maxRate*2.5);
	hBenchmarkScan->SetMinimum(0.1);
	hBenchmarkScan->Draw();
	vector<TF1*> fit;
	for(int itype=0; itype<types.size(); itype++) {
		grBenchmarkScan[itype]->SetMarkerStyle(20);
		grBenchmarkScan[itype]->SetMarkerColor(kBlack+itype);
		grBenchmarkScan[itype]->Draw("same pl");

		if(itype==0) {
			fit.push_back(new TF1(types[itype],"pol1",1,200)); 
			fit[itype]->FixParameter(0,0);
			grBenchmarkScan[itype]->Fit(fit[itype],"N","",0.5,24);
			fit[itype]->SetLineColor(kBlack+itype); fit[itype]->SetLineStyle(kDashed);
			fit[itype]->Draw("same");
		}

		if(itype==0) 
			legend->AddEntry(grBenchmarkScan[0],"ifarm19 CPU (2 thread/core)","pl");
		if(types[itype] == "gpuT4") 
			legend->AddEntry(grBenchmarkScan[itype],"sciml21 T4 GPU","pl");
		if(types[itype] == "gpuTitanRTX") 
			legend->AddEntry(grBenchmarkScan[itype],"sciml19 Titan RTX GPU","pl");
	}

	gPad->SetLeftMargin(0.09);
	gPad->SetBottomMargin(0.15);
	gPad->SetTopMargin(0.05);
	gPad->SetRightMargin(0.05);
	gPad->SetLogx(); gPad->SetLogy();
	gPad->SetGridy(); gPad->SetGridx();
	hBenchmarkScan->GetXaxis()->SetTitleSize(0.05);
	hBenchmarkScan->GetYaxis()->SetTitleSize(0.05);
	hBenchmarkScan->GetXaxis()->SetTitleOffset(1.3);
	hBenchmarkScan->GetYaxis()->SetTitleOffset(0.8);

	legend->SetFillColor(0);
	legend->Draw();

	cc->Print("benchmark.png");

	return;
}
