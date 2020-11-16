

/*

This root macro calculates a new set of CCAL gain constants based on the peak position of the 
E_ccal / E_Compton distribution. For the channels which do not have Compton acceptance, 
the gains are left un-corrected in this procedure. 

*/




void calcNewGains() {
	
	
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	
	
	
	//---------   Adjust these parameters   ----------//
	
	
	int rebins = 5;
	
	
	/* 
	The gain calibration is an iterative procedure. For each iteration, you correct each
	channel's gain constant as such:
	
	new_gain = old_gain / (gain_correction ^ gainCorrectionFactor)
	
	'gain_correction' is the peak value of the Eccal / Ecomp distribution. 
	You can set 'gainCorrectionFactor' to something less than 1 (i.e. 0.5) so that you don't 
	overcorrect the previous gain constants.
	*/
	
	
	const double gainCorrectionFactor = 0.5;
	
	
	// filename of txt file containing the old gain constants:
	const char old_gain_file_name[256] = "old_gains.dat";
	
	// filename of the output text file which will store the new gain constants:
	const char new_gain_file_name[256] = "new_gains.dat";
	
	
	// name of the root file:
	const char root_file_name[256] = "test.root";
	
	
	//-----------------------------------------------//
	
	
	
	TFile *fIn = new TFile( root_file_name, "READ" );
	
	
	TH2I *h2 = (TH2I*)fIn->Get("CompRatio_g");
	
	double gCorrections[144];
	
	vector< int > chanVec;
	vector< double > meanVec, meanErrVec;
	
	
	TCanvas *canvas = new TCanvas( "canvas", "canvas", 1000, 1000 );
	canvas->Divide( 2,2 );
	TPad *pad[4];
	for( int i=0; i<4; i++ ) { pad[i] = (TPad*)canvas->cd(i+1); pad[i]->SetGrid(); }
	
	
	double cmax, xmax;
	int counter = 0;
	
	for( int ich = 0; ich < 144; ich++ ) {
	  	
	  	int row = ich/12; int col = ich%12;
		
		// Skip channels with no Compton acceptance:
		
	  	if( row == 0 || row == 11 || col == 0 || col == 1 || col == 11 
	  		|| (row > 3 && row < 8 && col > 3 && col < 8) ) 
	  	{
	    		gCorrections[ich] = 1.0;
	    		continue;
	  	}
	  	
		
		TH1I *h1 = (TH1I*)h2->ProjectionY( Form("h1_%03d",ich), ich+1, ich+1 );
		h1->Rebin( rebins );
	  	h1->SetTitle( Form("Channel %d",ich) );
	  	h1->GetXaxis()->SetTitle( "E_{ccal} / E_{comp}" );
	  	h1->GetXaxis()->SetTitleSize( 0.05 );
	  	h1->GetXaxis()->SetTitleOffset( 0.9 );
	  	h1->SetLineWidth(2);
	  	h1->SetLineColor(kBlack);
	  	h1->GetXaxis()->SetRangeUser(0.5,1.5);
	  	
	  	cmax = (double)h1->GetMaximum();
	  	xmax = h1->GetBinCenter( h1->GetMaximumBin() );
	  	
		
		TF1 *f_gaus = new TF1( Form("f_gaus_%d",ich), "gaus", xmax-0.025, xmax+0.025 );
		
	  	f_gaus->SetParameters( cmax, xmax, 0.025 );
	  	h1->Fit( Form("f_gaus_%d",ich), "R0QL" );
	  	
	  	double loc_mean  = f_gaus->GetParameter(1);
	  	double loc_width = f_gaus->GetParameter(2);	  
	  	
	  	f_gaus->SetRange( loc_mean - 0.8*loc_width, loc_mean + 0.85*loc_width );
	  	h1->Fit( Form("f_gaus_%d",ich), "R0QL" );
	  	
	  	loc_mean  = f_gaus->GetParameter(1);
	  	loc_width = f_gaus->GetParameter(2);
	  	
	  	chanVec.push_back( ich );
	  	meanVec.push_back( loc_mean );
	  	meanErrVec.push_back( f_gaus->GetParError(1) );
	  	
	  	
	  	gCorrections[ich] = loc_mean;	  
	  	
	  	
	  	pad[counter%4]->cd();
	  	h1->Draw( "hist" );
	  	f_gaus->Draw( "same" );
	  	
	  	if( (counter+1)%4 == 0 ) 
	  	{
	    		if( counter==3 ) {
	      			canvas->Print( "gain_fits.pdf(", "pdf" );
	    		} else {
	      			canvas->Print( "gain_fits.pdf",  "pdf" );
	    		}
	    		
	    		canvas->Clear("D");
	  	}
	  	
	  	counter++;
	
	}
	
	canvas->Print( "gain_fits.pdf)", "pdf" );
	
	
	
	
	
	
	
	
	
	
	//----------   Plot Compton Peak vs. Chan, Row, Col   ----------//
	
	
	int nchans = static_cast<int>( chanVec.size() );
	double *chans = new double[nchans];
	double *means = new double[nchans];
	double *zeros = new double[nchans];
	double *meanErrs = new double[nchans];
	
	double *rs = new double[nchans];
	double *rows = new double[nchans];
	double *cols = new double[nchans];
	
	
	for( int i=0; i<nchans; i++ ) {
	  	chans[i] = static_cast<double>(chanVec[i]);
	  	zeros[i] = 0.;
	  	means[i] = meanVec[i];
	  	meanErrs[i] = meanErrVec[i];
	  	
	  	int row = chanVec[i]/12;
	  	int col = chanVec[i]%12;
	  	double x = 5.5 - static_cast<double>(row);
	  	double y = 5.5 - static_cast<double>(col);
	  	rs[i] = sqrt(x*x + y*y);
	  	rows[i] = static_cast<double>(row);
	  	cols[i] = static_cast<double>(col);
	}
	
	TGraphErrors *gr0 = new TGraphErrors( nchans, chans, means, zeros, meanErrs );
	gr0->SetTitle( "Mean vs. Channel Number using Compton Gains" );
	gr0->GetXaxis()->SetTitle( "CCAL id" );
	gr0->GetYaxis()->SetTitle( "Mean value of E_{ccal}/E_{comp}" );
	gr0->SetMarkerStyle(7);
	
	TGraphErrors *gr1 = new TGraphErrors( nchans, rs, means, zeros, meanErrs );
	gr1->SetTitle( "Mean vs. Distance from Center of CCAL" );
	gr1->GetXaxis()->SetTitle( "radius" );
	gr1->GetYaxis()->SetTitle( "Mean value of E_{ccal}/E_{comp}" );
	gr1->SetMarkerStyle(7);
	
	TGraphErrors *gr2 = new TGraphErrors( nchans, rows, means, zeros, meanErrs );
	gr2->SetTitle( "Mean vs. CCAL row" );
	gr2->GetXaxis()->SetTitle( "CCAL row" );
	gr2->GetYaxis()->SetTitle( "Mean value of E_{ccal}/E_{comp}" );
	gr2->SetMarkerStyle(7);
	
	TGraphErrors *gr3 = new TGraphErrors( nchans, cols, means, zeros, meanErrs );
	gr3->SetTitle( "Mean vs. CCAL column" );
	gr3->GetXaxis()->SetTitle( "CCAL column" );
	gr3->GetYaxis()->SetTitle( "Mean value of E_{ccal}/E_{comp}" );
	gr3->SetMarkerStyle(7);
	
	
	TCanvas *cg0 = new TCanvas( "cg0", "MeanVsChan", 1000, 800 );
	//gr0->GetYaxis()->SetRangeUser(0.99,1.01);
	gr0->Draw("AP");
	TLine *l1 = new TLine( 5., 0.999, 140., 0.999 );
	TLine *l2 = new TLine( 5., 1.001, 140., 1.001 );
	l1->SetLineColor(kRed); l2->SetLineColor(kRed);
	l1->Draw(); l2->Draw();
	
	
	TCanvas *cg1 = new TCanvas( "cg1", "MeanVsRad", 1000, 800 );
	gr1->Draw("AP");
	
	TCanvas *cg2 = new TCanvas( "cg2", "MeanVsRow", 1000, 800 );
	gr2->Draw("AP");
	
	TCanvas *cg3 = new TCanvas( "cg3", "MeanVsCol", 1000, 800 );
	gr3->Draw("AP");
	
	
	
	
	
	
	
	//----------   Calculate a new set of Gain Constants   ----------//
	
	
	double old_gains[144];
	double new_gains[144];
	
	int max_chan    = -1;
	double max_corr =  0.;
	
	
	ifstream oldgFile( old_gain_file_name );
	for( int i=0; i<144; i++ ) {
	  	oldgFile >> old_gains[i]; 
	}
	oldgFile.close();
	
	
	for( int ii = 0; ii < 144; ii++ ) {
	  	
	  	double loc_corr = fabs( 1.0 - gCorrections[ii] );
	  	if( loc_corr > max_corr ) { max_corr = loc_corr; max_chan = ii; }
	  	
	  	new_gains[ii] = old_gains[ii] / pow( gCorrections[ii], gainCorrectionFactor );
	  	if( ii==65 || ii==66 || ii==77 || ii==78 ) new_gains[ii] = 1.0;
	  	cout << ii <<",   "<< old_gains[ii] <<",   "<< new_gains[ii] <<
	  		",   "<< loc_corr << endl;	  
	  	
	}
	
	
	ofstream newgFile( new_gain_file_name );
	for( int i=0; i<144; i++ ) {
	  	newgFile << new_gains[i] << "\n"; 
	}
	newgFile.close();
	
	
	cout << "\n\n\n";
	cout << "max correction: " << max_corr << " (chan " << max_chan << ")" << endl;
	cout << "\n";
	
	
	
	return;
}

