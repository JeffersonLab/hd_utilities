

Double_t bkgd_fit( Double_t *x, Double_t *par );
Double_t crys_ball_fit( Double_t *x, Double_t *par );

/*

This root macro just fits the distribution of E_ccal - E_Compotn to a combination of a Crystal Ball function + polynomial background. As it currently is, it should not be used to calculate the new
set of gain constants (but it may be converted to do so in the future).

*/



void calcNewGains_crysBall() {
	
	
	
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(1);
	
	
	
	//---------   Adjust these parameters   ----------//
	
	
	int rebins = 5;
	
	// filename of txt file containing the old gain constants:
	const char old_gain_file_name[256] = "gains.dat";
	
	// filename of the output text file which will store the new gain constants:
	const char new_gain_file_name[256] = "new_gains.dat";
	
	
	// name of the root file:
	const char root_file_name[256] = "test.root";
	
	
	//-----------------------------------------------//
	
	
	
	TFile *fIn = new TFile( root_file_name, "READ" );
	
	
	TH2I *h2 = (TH2I*)fIn->Get("CompDiff_g");
	
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
	  	h1->GetXaxis()->SetRangeUser(-2.0,2.0);
	  	
		
		
		// Fit the background:
		
		TF1 *f_bkgd = new TF1( Form("f_bkgd_%d",ich), bkgd_fit, -2.0, 2.0, 6 );
		
		// These two parameters are the bounds of the region to exclude from the
		// bacgkround fit. Set them to exclude the peak region:
		
		f_bkgd->FixParameter( 4, -0.5 );
		f_bkgd->FixParameter( 5,  0.5 ); 
		
		h1->Fit( Form("f_bkgd_%d",ich), "R0QL" );
		
		
		
		
		// Do a Gaussian fit first to get initial guess parameters:
		
		
	  	cmax = (double)h1->GetMaximum();
	  	xmax = h1->GetBinCenter( h1->GetMaximumBin() );
	  	
		TF1 *f_gaus = new TF1( Form("f_gaus_%d",ich), "gaus", xmax-0.25, xmax+0.25 );
		
	  	f_gaus->SetParameters( cmax, xmax, 0.15 );
	  	f_gaus->SetRange( xmax-0.25, xmax+0.25 );
	  	h1->Fit( Form("f_gaus_%d",ich), "R0QL" );
	  	
	  	double loc_mean  = f_gaus->GetParameter(1);
	  	double loc_width = f_gaus->GetParameter(2);	  
	  	
	  	f_gaus->SetRange( loc_mean - 0.8*loc_width, loc_mean + 0.85*loc_width );
	  	h1->Fit( Form("f_gaus_%d",ich), "R0QL" );
	  	
	  	loc_mean  = f_gaus->GetParameter(1);
	  	loc_width = f_gaus->GetParameter(2);
	  	
	  	
	  	
		// Now do fit deltaE distribution to a Crystal Ball function:
		
		TF1 *f_crys_ball = new TF1( Form("f_crys_ball_%d",ich), crys_ball_fit, -1.5, 1.5, 9 );
		f_crys_ball->SetParameters( f_gaus->GetParameter(0), f_gaus->GetParameter(1), 
			f_gaus->GetParameter(2), 1.85, 2.0 );
		f_crys_ball->SetParameter( 5, f_bkgd->GetParameter(0) );
		f_crys_ball->SetParameter( 6, f_bkgd->GetParameter(1) );
		f_crys_ball->SetParameter( 7, f_bkgd->GetParameter(2) );
		f_crys_ball->SetParameter( 8, f_bkgd->GetParameter(3) );
		
		h1->Fit( Form("f_crys_ball_%d",ich), "R0QL" );
		
		
		
	  	gCorrections[ich] = f_crys_ball->GetParameter(1);
	  	
		chanVec.push_back( ich );
		meanVec.push_back( f_crys_ball->GetParameter(1) );
		meanErrVec.push_back( f_crys_ball->GetParError(1) );
		
		
	  	
	  	TF1 *f_draw = new TF1( Form("f_draw_%d",ich), "pol3", -1.5, 1.5 );
		f_draw->SetParameters( f_crys_ball->GetParameter(5), f_crys_ball->GetParameter(6), 
			f_crys_ball->GetParameter(7), f_crys_ball->GetParameter(8) );
		f_draw->SetLineColor( kGreen );
		f_draw->SetLineStyle( 2 );
		
	  	
	  	pad[counter%4]->cd();
	  	h1->Draw( "hist" );
	  	f_crys_ball->Draw( "same" );
	  	f_draw->Draw( "same" );
		
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
	
	
	
	
	
	return;
}



Double_t bkgd_fit( Double_t *x, Double_t *par ) {
	
	Double_t xx = x[0];
	
	
	if( xx > par[4] && xx < par[5] ) {
		TF1::RejectPoint();
		return 0.;
	}
	
	Double_t f = par[0] + 
		     par[1] * x[0] +
		     par[2] * x[0] * x[0] + 
		     par[3] * x[0] * x[0] * x[0];
	
	return f;
}



Double_t crys_ball_fit( Double_t *x, Double_t *par ) 
{
	
	Double_t xx  =   x[0];
	
	Double_t N   = par[0];
	Double_t mu  = par[1];
	Double_t sig = par[2];
	Double_t a   = par[3];
	Double_t n   = par[4];
	
	Double_t p0  = par[5];
	Double_t p1  = par[6];
	Double_t p2  = par[7];
	Double_t p3  = par[8];
	
	
	Double_t A   = 	pow( n/fabs(a), n ) * exp( -0.5*pow(fabs(a),2.0) );
	Double_t B   =  ( n/fabs(a) ) - fabs(a);
	
	Double_t loc_x = ( xx - mu ) / sig;
	Double_t f;
	
	if( loc_x > -a ) {
		f = N * exp( -0.5*pow(loc_x,2.0) );
	} else {
		f = N * A * pow( B - loc_x, -n );
	}
	
	f += p0 + p1*xx + p2*xx*xx + p3*xx*xx*xx;
	
	
	return f;
}

