//
// 2010/09/02 David Lawrence
//
// This macro can be used to fit a 1-D histogram to a Voigt function.
// A Voigt function is a convolution of a Briet-Wigner resonance 
// distribution and a Gaussian. The Voigt function is defined in 
// ROOT's TMath class, but needs to be wrapped in a TF1 to make it
// usable to the TH1D::Fit() method.
//
// To use this, include it in the top of the macro that produces
// the histogram to be fit and then just call fit_voigt(...) passing
// a pointer to the histogram to be fit.
//
// To access the parameters, you'll need to do something like:
//
// TF1 *voigt = hist->GetFunction("voigt");
// double amp   = voigt->GetParameter(0);
// double mean  = voigt->GetParameter(1);
// double sigma = voigt->GetParameter(2);
// double gamma = voigt->GetParameter(3);
//


int fit_voigt(TH1D *h, const char *opts="")
{
	// Get amplitude and location of maximum
	int maxbin = h->GetMaximumBin();
	double amp = h->GetMaximum();
	double mean = h->GetBinCenter(maxbin);
	
	// Get rough estimate of width by looking
	// for point to either side of maximum that
	// is 10% of maximum.
	double xright = mean;
	for(int bin = maxbin; bin<h->GetNbinsX(); bin++){
		if(h->GetBinContent(bin)/amp <= 0.1){
			xright = h->GetBinCenter(bin);
			break;
		}
	}

	double xleft = mean;
	for(int bin = maxbin; bin>=1; bin--){
		if(h->GetBinContent(bin)/amp <= 0.1){
			xleft = h->GetBinCenter(bin);
			break;
		}
	}

	double width = 0.4*(xright-xleft);
	double sigma = width/2.0;
	double gamma = width/2.0;
	
	// Define voigt function
	TF1 *voigt = new TF1("voigt","[0]*TMath::Voigt(x-[1],[2],[3])", mean-5.0*width, mean+5.0*width);
	voigt->SetParName(0, "amp");
	voigt->SetParName(1, "mean");
	voigt->SetParName(2, "sigma");
	voigt->SetParName(3, "#Gamma");
	
	// Initialize parameters giving half of "width" to the
	// sigma and half to the gamma
	voigt->SetParameter(0, amp/4.0);
	voigt->SetParameter(1, mean);
	voigt->SetParameter(2, sigma);
	voigt->SetParameter(3, gamma);

	// Do the fit
	h->Fit(voigt,opts);
	//h->Draw();
	//voigt->Draw("same");
	//voigt->Print();
	
	// Get parameters
	amp = voigt->GetParameter(0);
	mean = voigt->GetParameter(1);
	sigma = voigt->GetParameter(2);
	gamma = voigt->GetParameter(3);
	
	// Get integral over limits of histogram
	double integral = voigt->Integral(h->GetBinLowEdge(1), h->GetBinLowEdge(h->GetNbinsX()+1));
	double Nevents = integral/h->GetBinWidth(1);
	cout<<"Nevents = "<<Nevents<<endl;
	
	return Nevents;
}
