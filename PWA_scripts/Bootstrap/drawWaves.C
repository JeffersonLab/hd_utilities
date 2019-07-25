
{
    enum { kMaxPoints = 100 };
    enum { kBootstrapPoints = 100 };
    enum { NumBins=60 };

    double ll = 0.8;
    double ul = 2.0;
    
    double eventCounter = 0;
    double eventCounter2 = 0;

    double mass[kMaxPoints][kBootstrapPoints];
    double masse[kMaxPoints][kBootstrapPoints];
    double etaPiSWave[kMaxPoints][kBootstrapPoints];
    double etaPiSWavee[kMaxPoints][kBootstrapPoints];
    double etaPiP0Wave[kMaxPoints][kBootstrapPoints];
    double etaPiP0Wavee[kMaxPoints][kBootstrapPoints];
    double etaPiP1pWave[kMaxPoints][kBootstrapPoints];
    double etaPiP1pWavee[kMaxPoints][kBootstrapPoints];
    double etaPiP1mWave[kMaxPoints][kBootstrapPoints];
    double etaPiP1mWavee[kMaxPoints][kBootstrapPoints];
    double etaPiD0Wave[kMaxPoints][kBootstrapPoints];
    double etaPiD0Wavee[kMaxPoints][kBootstrapPoints];
    double etaPiD1Wave[kMaxPoints][kBootstrapPoints];
    double etaPiD1Wavee[kMaxPoints][kBootstrapPoints];
    double etaPiDpWave[kMaxPoints][kBootstrapPoints];
    double etaPiDpWavee[kMaxPoints][kBootstrapPoints];
    double all[kMaxPoints][kBootstrapPoints];
    double alle[kMaxPoints][kBootstrapPoints];

    
    for(int i=0; i<NumBins; i++){
        ifstream in;
        ostringstream file;
        file.str("");
        file<<"EtaPi_fit/bin_"<<i<<"/"<<"etapi_fit.txt";
        in.open( file.str().c_str() );
        
        int line = 0;
        while( ! in.eof() ){
            
            in >> mass[i][line]
            >> etaPiSWave[i][line] >> etaPiSWavee[i][line]
            >> etaPiP0Wave[i][line] >> etaPiP0Wavee[i][line]
            >> etaPiP1mWave[i][line] >> etaPiP1mWavee[i][line]
            >> etaPiD0Wave[i][line] >> etaPiD0Wavee[i][line]
            >> etaPiD1Wave[i][line] >> etaPiD1Wavee[i][line]
            >> etaPiP1pWave[i][line] >> etaPiP1pWavee[i][line]
            >> etaPiDpWave[i][line] >> etaPiDpWavee[i][line]
            >> all[i][line] >> alle[i][line];
            
            eventCounter += all[i][line];
            
            line++;
            if(line>=kBootstrapPoints) break;
        }
        
    }
    
    double mass2[kMaxPoints];
    double masse2[kMaxPoints];
    double etaPiSWave2[kMaxPoints];
    double etaPiSWavee2[kMaxPoints];
    double etaPiP0Wave2[kMaxPoints];
    double etaPiP0Wavee2[kMaxPoints];
    double etaPiP1pWave2[kMaxPoints];
    double etaPiP1pWavee2[kMaxPoints];
    double etaPiP1mWave2[kMaxPoints];
    double etaPiP1mWavee2[kMaxPoints];
    double etaPiD0Wave2[kMaxPoints];
    double etaPiD0Wavee2[kMaxPoints];
    double etaPiD1Wave2[kMaxPoints];
    double etaPiD1Wavee2[kMaxPoints];
    double etaPiDpWave2[kMaxPoints];
    double etaPiDpWavee2[kMaxPoints];
    double all2[kMaxPoints];
    double alle2[kMaxPoints];

    ifstream in2;
    ostringstream file2;

    file2.str("");
   /*
   //Uncomment this section to get the results from the fit to real data, not the bootstrap mean! file2<<"/w/halld-scifs17exp/home/gleasonc/Hall-D/AmplitudeAnalysis/EtaPiMinusDeltaPP/data/Flat_G3_Randoms/etapi_fit.txt";
    in2.open( file2.str().c_str() );
    
    int line2 = 0;
    while( ! in2.eof() ){
        
        in2 >> mass2[line2]
        >> etaPiSWave2[line2] >> etaPiSWavee2[line2]
        >> etaPiP0Wave2[line2] >> etaPiP0Wavee2[line2]
        >> etaPiP1mWave2[line2] >> etaPiP1mWavee2[line2]
        >> etaPiD0Wave2[line2] >> etaPiD0Wavee2[line2]
        >> etaPiD1Wave2[line2] >> etaPiD1Wavee2[line2]
        >> etaPiP1pWave2[line2] >> etaPiP1pWavee2[line2]
        >> etaPiDpWave2[line2] >> etaPiDpWavee2[line2]
        >> all2[line2] >> alle2[line2];

        eventCounter2 += all2[line2];
        
        cout<<"S = "<<etaPiSWave2[line2]<< endl;
        line2++;
        if(line2>=65) break;
    }
*/

    double etaPiSWave_Mean[kMaxPoints], etaPiSWave_stdev[kMaxPoints];
    double etaPiP0Wave_Mean[kMaxPoints], etaPiP0Wave_stdev[kMaxPoints];
    double etaPiP1pWave_Mean[kMaxPoints], etaPiP1pWave_stdev[kMaxPoints];
    double etaPiP1mWave_Mean[kMaxPoints], etaPiP1mWave_stdev[kMaxPoints];
    double etaPiD0Wave_Mean[kMaxPoints], etaPiD0Wave_stdev[kMaxPoints];
    double etaPiD1Wave_Mean[kMaxPoints], etaPiD1Wave_stdev[kMaxPoints];
    double etaPiDpWave_Mean[kMaxPoints], etaPiDpWave_stdev[kMaxPoints];
    double allInt_Mean[kMaxPoints], allInt_stdev[kMaxPoints], allInt_stdevb[kMaxPoints];

    double mass_graph[kMaxPoints];
    for(int i=0; i<kMaxPoints; i++){
        mass_graph[i]=mass[i][0];
        
        double tempMeanSWave=0;
        double tempMeanP0Wave=0;
        double tempMeanP1pWave=0;
        double tempMeanP1mWave=0;
        double tempMeanD0Wave=0;
        double tempMeanD1Wave=0;
        double tempMeanDpWave=0;
        double tempMeanTotal=0;

        for(int j=0; j<kBootstrapPoints; j++){
            tempMeanSWave+=etaPiSWave[i][j];
            tempMeanP0Wave+=etaPiP0Wave[i][j];
            tempMeanP1pWave+=etaPiP1pWave[i][j];
            tempMeanP1mWave+=etaPiP1mWave[i][j];
            tempMeanD0Wave+=etaPiD0Wave[i][j];
            tempMeanD1Wave+=etaPiD1Wave[i][j];
            tempMeanDpWave+=etaPiDpWave[i][j];
            tempMeanTotal+=etaPiSWave[i][j]+etaPiP0Wave[i][j]+etaPiP1pWave[i][j]+etaPiP1mWave[i][j]+etaPiD0Wave[i][j]+etaPiDpWave[i][j]+etaPiD1Wave[i][j];

        }
        etaPiSWave_Mean[i]=tempMeanSWave/kBootstrapPoints;
        etaPiP0Wave_Mean[i]=tempMeanP0Wave/kBootstrapPoints;
        etaPiP1pWave_Mean[i]=tempMeanP1pWave/kBootstrapPoints;
        etaPiP1mWave_Mean[i]=tempMeanP1mWave/kBootstrapPoints;
        etaPiD0Wave_Mean[i]=tempMeanD0Wave/kBootstrapPoints;
        etaPiD1Wave_Mean[i]=tempMeanD1Wave/kBootstrapPoints;
        etaPiDpWave_Mean[i]=tempMeanDpWave/kBootstrapPoints;
        allInt_Mean[i]=tempMeanTotal/kBootstrapPoints;
        
        //hetaPiSWave_Mean[i]->Fill(etaPiSWave_Mean[i]);
        
        //allInt_Mean[i] = etaPiSWave_Mean[i] +etaPiP0Wave_Mean[i]+ etaPiP1pWave_Mean[i] +etaPiP1mWave_Mean[i] + etaPiD0Wave_Mean[i] + etaPiD1Wave_Mean[i] + etaPiDpWave_Mean[i];
        
        double tempDevSWave=0;
        double tempDevP0Wave=0;
        double tempDevP1pWave=0;
        double tempDevP1mWave=0;
        double tempDevD0Wave=0;
        double tempDevD1Wave=0;
        double tempDevDpWave=0;
        double tempDevTotal=0;

        for(int j=0; j<kBootstrapPoints; j++){
            tempDevSWave+=pow((etaPiSWave[i][j]-etaPiSWave_Mean[i]),2);
            tempDevP0Wave+=pow((etaPiP0Wave[i][j]-etaPiP0Wave_Mean[i]),2);
            tempDevP1pWave+=pow((etaPiP1pWave[i][j]-etaPiP1pWave_Mean[i]),2);
            tempDevP1mWave+=pow((etaPiP1mWave[i][j]-etaPiP1mWave_Mean[i]),2);
            tempDevD0Wave+=pow((etaPiD0Wave[i][j]-etaPiD0Wave_Mean[i]),2);
            tempDevD1Wave+=pow((etaPiD1Wave[i][j]-etaPiD1Wave_Mean[i]),2);
            tempDevDpWave+=pow((etaPiDpWave[i][j]-etaPiDpWave_Mean[i]),2);
            tempDevTotal += pow((etaPiSWave[i][j]-etaPiSWave_Mean[i]),2)+pow((etaPiP0Wave[i][j]-etaPiP0Wave_Mean[i]),2)+pow((etaPiP1pWave[i][j]-etaPiP1pWave_Mean[i]),2)+pow((etaPiP1mWave[i][j]-etaPiP1mWave_Mean[i]),2)+pow((etaPiD0Wave[i][j]-etaPiD0Wave_Mean[i]),2)+pow((etaPiD1Wave[i][j]-etaPiD1Wave_Mean[i]),2)+pow((etaPiDpWave[i][j]-etaPiDpWave_Mean[i]),2);
        }
        etaPiSWave_stdev[i]=sqrt(tempDevSWave/kBootstrapPoints);
        etaPiP0Wave_stdev[i]=sqrt(tempDevP0Wave/kBootstrapPoints);
        etaPiP1pWave_stdev[i]=sqrt(tempDevP1pWave/kBootstrapPoints);
        etaPiP1mWave_stdev[i]=sqrt(tempDevP1mWave/kBootstrapPoints);
        etaPiD0Wave_stdev[i]=sqrt(tempDevD0Wave/kBootstrapPoints);
        etaPiD1Wave_stdev[i]=sqrt(tempDevD1Wave/kBootstrapPoints);
        etaPiDpWave_stdev[i]=sqrt(tempDevDpWave/kBootstrapPoints);
        allInt_stdev[i]=sqrt(tempDevTotal/kBootstrapPoints);
        //allInt_stdev[i] = sqrt(pow(etaPiSWave_stdev[i],2) + pow(etaPiP0Wave_stdev[i],2) + pow(etaPiP1pWave_stdev[i],2) + pow(etaPiP1mWave_stdev[i],2) + pow(etaPiD0Wave_stdev[i],2) + pow(etaPiD1Wave_stdev[i],2) + pow(etaPiDpWave_stdev[i],2));
    }
    //etaPiSWave_Mean[0]=(etaPiSWave[0][1]+etaPiSWave[0][2]+etaPiSWave[0][3]+etaPiSWave[0][4])/5.;
    
    TFile* fileMeanHists=new TFile("histograms.root","RECREATE");
    
    ostringstream histname, histtitle;
    TH1F* hetaPiSWave_Mean[NumBins];
    TH1F* hetaPiP0Wave_Mean[NumBins];
    TH1F* hetaPiP1pWave_Mean[NumBins];
    TH1F* hetaPiP1mWave_Mean[NumBins];
    TH1F* hetaPiD0Wave_Mean[NumBins];
    TH1F* hetaPiD1Wave_Mean[NumBins];
    TH1F* hetaPiDpWave_Mean[NumBins];
    TH1F* hetaPiTotal_Mean[NumBins];

    double step=(ul-ll)/NumBins;
    
    for(i=0; i<NumBins; i++){
        histname.str("");
        histname<<"hetaPiSWave_Mean_"<<i;
        histtitle.str("");
        histtitle<<"SWave M(#eta#pi)="<<mass_graph[i]<<" GeV";
        
        hetaPiSWave_Mean[i]=new TH1F( histname.str().c_str(), histtitle.str().c_str(), kBootstrapPoints, etaPiSWave_Mean[i]-3*etaPiSWave_stdev[i], etaPiSWave_Mean[i]+3*etaPiSWave_stdev[i] );
        
        //P0Wave
        histname.str("");
        histname<<"hetaPiP0Wave_Mean_"<<i;
        histtitle.str("");
        histtitle<<"P0Wave M(#eta#pi)="<<ll + step * i + step / 2.<<" GeV";
        
        hetaPiP0Wave_Mean[i]=new TH1F( histname.str().c_str(), histtitle.str().c_str(), kBootstrapPoints, etaPiP0Wave_Mean[i]-3*etaPiP0Wave_stdev[i], etaPiP0Wave_Mean[i]+3*etaPiP0Wave_stdev[i] );
        
        //P1pWave
        histname.str("");
        histname<<"hetaPiP1pWave_Mean_"<<i;
        histtitle.str("");
        histtitle<<"P1pWave M(#eta#pi)="<<ll + step * i + step / 2.<<" GeV";
        
        hetaPiP1pWave_Mean[i]=new TH1F( histname.str().c_str(), histtitle.str().c_str(), kBootstrapPoints, etaPiP1pWave_Mean[i]-3*etaPiP1pWave_stdev[i], etaPiP1pWave_Mean[i]+3*etaPiP1pWave_stdev[i] );
        
        //P1mWave
        histname.str("");
        histname<<"hetaPiP1mWave_Mean_"<<i;
        histtitle.str("");
        histtitle<<"P1mWave M(#eta#pi)="<<ll + step * i + step / 2.<<" GeV";
        
        hetaPiP1mWave_Mean[i]=new TH1F( histname.str().c_str(), histtitle.str().c_str(), kBootstrapPoints, etaPiP1mWave_Mean[i]-3*etaPiP1mWave_stdev[i], etaPiP1mWave_Mean[i]+3*etaPiP1mWave_stdev[i] );
        
        //D0Wave
        histname.str("");
        histname<<"hetaPiD0Wave_Mean_"<<i;
        histtitle.str("");
        histtitle<<"D0Wave M(#eta#pi)="<<ll + step * i + step / 2.<<" GeV";
        
        hetaPiD0Wave_Mean[i]=new TH1F( histname.str().c_str(), histtitle.str().c_str(), kBootstrapPoints, etaPiD0Wave_Mean[i]-3*etaPiD0Wave_stdev[i], etaPiD0Wave_Mean[i]+3*etaPiD0Wave_stdev[i] );
        
        //D1Wave
        histname.str("");
        histname<<"hetaPiD1Wave_Mean_"<<i;
        histtitle.str("");
        histtitle<<"D1Wave M(#eta#pi)="<<ll + step * i + step / 2.<<" GeV";
        
        hetaPiD1Wave_Mean[i]=new TH1F( histname.str().c_str(), histtitle.str().c_str(), kBootstrapPoints, etaPiD1Wave_Mean[i]-3*etaPiD1Wave_stdev[i], etaPiD1Wave_Mean[i]+3*etaPiD1Wave_stdev[i] );
        
        //DpWave
        histname.str("");
        histname<<"hetaPiDpWave_Mean_"<<i;
        histtitle.str("");
        histtitle<<"DpWave M(#eta#pi)="<<ll + step * i + step / 2.<<" GeV";
        
        hetaPiDpWave_Mean[i]=new TH1F( histname.str().c_str(), histtitle.str().c_str(), kBootstrapPoints, etaPiDpWave_Mean[i]-3*etaPiDpWave_stdev[i], etaPiDpWave_Mean[i]+3*etaPiDpWave_stdev[i] );
        
        //Total
        histname.str("");
        histname<<"hetaPiTotal_Mean_"<<i;
        histtitle.str("");
        histtitle<<" M(#eta#pi)="<<ll + step * i + step / 2.<<" GeV";
        
        hetaPiTotal_Mean[i]=new TH1F( histname.str().c_str(), histtitle.str().c_str(), kBootstrapPoints, allInt_Mean[i]-3*allInt_stdev[i], allInt_Mean[i]+3*allInt_stdev[i] );
        
        for(int j=0; j<kBootstrapPoints; j++){
            hetaPiSWave_Mean[i]->Fill(etaPiSWave[i][j]);
            hetaPiP0Wave_Mean[i]->Fill(etaPiP0Wave[i][j]);
            hetaPiP1pWave_Mean[i]->Fill(etaPiP1pWave[i][j]);
            hetaPiP1mWave_Mean[i]->Fill(etaPiP1mWave[i][j]);
            hetaPiD0Wave_Mean[i]->Fill(etaPiD0Wave[i][j]);
            hetaPiD1Wave_Mean[i]->Fill(etaPiD1Wave[i][j]);
            hetaPiDpWave_Mean[i]->Fill(etaPiDpWave[i][j]);
            hetaPiTotal_Mean[i]-> Fill (etaPiSWave[i][j]+etaPiP0Wave[i][j]+etaPiP1pWave[i][j]+etaPiP1mWave[i][j]+etaPiD0Wave[i][j]+etaPiD1Wave[i][j]+etaPiDpWave[i][j]);

        }
        
        allInt_stdevb[i]=hetaPiTotal_Mean[i]->GetRMS();
    }

    TGraphErrors etaPiSWaveGraph( NumBins, mass_graph, etaPiSWave_Mean, 0, etaPiSWave_stdev );
    etaPiSWaveGraph.SetMarkerStyle( 20 );
    etaPiSWaveGraph.SetMarkerSize( .5 );
    etaPiSWaveGraph.SetName("SGraph");
    
    TGraphErrors etaPiP0WaveGraph( NumBins, mass_graph, etaPiP0Wave_Mean, 0, etaPiP0Wave_stdev );
    etaPiP0WaveGraph.SetMarkerStyle( 20 );
    etaPiP0WaveGraph.SetMarkerSize( .5 );
    etaPiP0WaveGraph.SetName("P0Graph");
    
    TGraphErrors etaPiP1pWaveGraph( NumBins, mass_graph, etaPiP1pWave_Mean, 0, etaPiP1pWave_stdev );
    etaPiP1pWaveGraph.SetMarkerStyle( 20 );
    etaPiP1pWaveGraph.SetMarkerSize( .5 );
    etaPiP1pWaveGraph.SetName("P1mGraph");
    
    TGraphErrors etaPiP1mWaveGraph( NumBins, mass_graph, etaPiP1mWave_Mean, 0, etaPiP1mWave_stdev );
    etaPiP1mWaveGraph.SetMarkerStyle( 20 );
    etaPiP1mWaveGraph.SetMarkerSize( .5 );
    etaPiP1mWaveGraph.SetName("P1pGraph");
    
    TGraphErrors etaPiD0WaveGraph( NumBins, mass_graph, etaPiD0Wave_Mean, 0, etaPiD0Wave_stdev );
    etaPiD0WaveGraph.SetMarkerStyle( 20 );
    etaPiD0WaveGraph.SetMarkerSize( .5 );
    etaPiD0WaveGraph.SetName("D0Graph");
    
    TGraphErrors etaPiD1WaveGraph( NumBins, mass_graph, etaPiD1Wave_Mean, 0, etaPiD1Wave_stdev );
    etaPiD1WaveGraph.SetMarkerStyle( 20 );
    etaPiD1WaveGraph.SetMarkerSize( .5 );
    etaPiD1WaveGraph.SetName("D1mGraph");
    
    TGraphErrors etaPiDpWaveGraph( NumBins, mass_graph, etaPiDpWave_Mean, 0, etaPiDpWave_stdev );
    etaPiDpWaveGraph.SetMarkerStyle( 20 );
    etaPiDpWaveGraph.SetMarkerSize( .5 );
    etaPiDpWaveGraph.SetName("D1pGraph");
    
    TGraphErrors allGraph( NumBins, mass_graph, allInt_Mean, 0, allInt_stdevb );
    allGraph.SetMarkerStyle( 20 );
    allGraph.SetMarkerSize( 0.5 );
    allGraph.SetName("allGraph");
    
    /*
     //Uncomment this section to plot true results. Need to comment out above graphs
     
    TGraphErrors etaPiSWaveGraph( NumBins, mass_graph, etaPiSWave2, 0, etaPiSWave_stdev );
    etaPiSWaveGraph.SetMarkerStyle( 20 );
    etaPiSWaveGraph.SetMarkerSize( .5 );
    etaPiSWaveGraph.SetName("SGraph");
    
    TGraphErrors etaPiP0WaveGraph( NumBins, mass_graph, etaPiP0Wave2, 0, etaPiP0Wave_stdev );
    etaPiP0WaveGraph.SetMarkerStyle( 20 );
    etaPiP0WaveGraph.SetMarkerSize( .5 );
    etaPiP0WaveGraph.SetName("P0Graph");

    TGraphErrors etaPiP1pWaveGraph( NumBins, mass_graph, etaPiP1pWave2, 0, etaPiP1pWave_stdev );
    etaPiP1pWaveGraph.SetMarkerStyle( 20 );
    etaPiP1pWaveGraph.SetMarkerSize( .5 );
    etaPiP1pWaveGraph.SetName("P1mGraph");

    TGraphErrors etaPiP1mWaveGraph( NumBins, mass_graph, etaPiP1mWave2, 0, etaPiP1mWave_stdev );
    etaPiP1mWaveGraph.SetMarkerStyle( 20 );
    etaPiP1mWaveGraph.SetMarkerSize( .5 );
    etaPiP1mWaveGraph.SetName("P1pGraph");

    TGraphErrors etaPiD0WaveGraph( NumBins, mass_graph, etaPiD0Wave2, 0, etaPiD0Wave_stdev );
    etaPiD0WaveGraph.SetMarkerStyle( 20 );
    etaPiD0WaveGraph.SetMarkerSize( .5 );
    etaPiD0WaveGraph.SetName("D0Graph");

    TGraphErrors etaPiD1WaveGraph( NumBins, mass_graph, etaPiD1Wave2, 0, etaPiD1Wave_stdev );
    etaPiD1WaveGraph.SetMarkerStyle( 20 );
    etaPiD1WaveGraph.SetMarkerSize( .5 );
    etaPiD1WaveGraph.SetName("D1mGraph");

    TGraphErrors etaPiDpWaveGraph( NumBins, mass_graph, etaPiDpWave2, 0, etaPiDpWave_stdev );
    etaPiDpWaveGraph.SetMarkerStyle( 20 );
    etaPiDpWaveGraph.SetMarkerSize( .5 );
    etaPiDpWaveGraph.SetName("D1pGraph");

    TGraphErrors allGraph( NumBins, mass_graph, all2, 0, allInt_stdevb );
    allGraph.SetMarkerStyle( 20 );
    allGraph.SetMarkerSize( 0.5 );
    allGraph.SetName("allGraph");
     */
    
    int ymax=350000;
    int ymax1=150000;

    TCanvas* can = new TCanvas( "can", "Amplitude Analysis Plots", 800, 800 );
    can->Divide( 3, 3 );
    
    can->cd( 1 );
    TH1F h1( "h1", "#eta#pi S0", 1, ll, ul );
    h1.SetMaximum( ymax1 );
    h1.GetXaxis()->SetTitle( "#eta#pi Invariant Mass [GeV/c^{2}]" );
    h1.SetStats( 0 );
    h1.Draw();

    etaPiSWaveGraph.Draw( "P" );
    etaPiSWaveGraph.Write();
    
    can->cd( 2 );
    TH1F h2( "h2", "#eta#pi P0", 1, ll, ul );
    h2.SetMaximum( ymax1 );
    h2.GetXaxis()->SetTitle( "#eta#pi Invariant Mass [GeV/c^{2}]" );
    h2.SetStats( 0 );
    h2.Draw();
    etaPiP0WaveGraph.Draw( "P" );
    etaPiP0WaveGraph.Write();

    can->cd( 3 );
    TH1F h3( "h3", "#eta#pi P1-", 1, ll, ul );
    h3.SetMaximum( ymax1 );
    h3.GetXaxis()->SetTitle( "#eta#pi Invariant Mass [GeV/c^{2}]" );
    h3.SetStats( 0 );
    h3.Draw();
    etaPiP1mWaveGraph.Draw( "P" );
    etaPiP1mWaveGraph.Write();

    can->cd( 4 );
    TH1F h4( "h4", "#eta#pi P1+", 1, ll, ul );
    h4.SetMaximum( ymax1 );
    h4.GetXaxis()->SetTitle( "#eta#pi Invariant Mass [GeV/c^{2}]" );
    h4.SetStats( 0 );
    h4.Draw();
    etaPiP1pWaveGraph.Draw( "P" );
    etaPiP1pWaveGraph.Write();

    can->cd( 5 );
    TH1F h5( "h5", "#eta#pi D0", 1, ll, ul );
    h5.SetMaximum( ymax1 );
    h5.GetXaxis()->SetTitle( "#eta#pi Invariant Mass [GeV/c^{2}]" );
    h5.SetStats( 0 );
    h5.Draw();
    etaPiD0WaveGraph.Draw( "P" );
    etaPiD0WaveGraph.Write();

    can->cd( 6 );
    TH1F h6( "h6", "#eta#pi D1-", 1, ll, ul );
    h6.SetMaximum( ymax1 );
    h6.GetXaxis()->SetTitle( "#eta#pi Invariant Mass [GeV/c^{2}]" );
    h6.SetStats( 0 );
    h6.Draw();
    etaPiD1WaveGraph.Draw( "P" );
    etaPiD1WaveGraph.Write();

    can->cd( 7 );
    TH1F h7( "h7", "#eta#pi D1+", 1, ll, ul );
    h7.SetMaximum( ymax1 );
    h7.GetXaxis()->SetTitle( "#eta#pi Invariant Mass [GeV/c^{2}]" );
    h7.SetStats( 0 );
    h7.Draw();
    etaPiDpWaveGraph.Draw( "P" );
    etaPiDpWaveGraph.Write();

    can->cd( 8 );
    TH1F h8( "h8", "#eta#pi All Waves", 1, ll, ul );
    h8.SetMaximum( ymax );
    h8.GetXaxis()->SetTitle( "#eta#pi Invariant Mass [GeV/c^{2}]" );
    h8.SetStats( 0 );
    h8.Draw();
    allGraph.Draw( "P" );
    allGraph.Write();

    can->SaveAs("CombinedFit.pdf");
    cout << "Total number of events:  " << eventCounter << endl;
    
    TCanvas *c2=new TCanvas("c2","");
    c2->Divide(8,5);
    for(int i=0; i<40; i++){
        c2->cd(i+1);
        hetaPiD1Wave_Mean[i]->Draw();
    }
    /*
    c2->cd(1);
    hetaPiSWave_Mean[3]->Draw();
    c2->cd(2);
    hetaPiD1Wave_Mean[15]->Draw();
    */
    for (int i=0; i<NumBins; i++){
        hetaPiSWave_Mean[i]->Write();
        hetaPiP0Wave_Mean[i]->Write();
        hetaPiP1pWave_Mean[i]->Write();
        hetaPiP1mWave_Mean[i]->Write();
        hetaPiD0Wave_Mean[i]->Write();
        hetaPiD1Wave_Mean[i]->Write();
        hetaPiDpWave_Mean[i]->Write();
        hetaPiTotal_Mean[i]->Write();
    }
    fileMeanHists->Write();
    fileMeanHists->Close();
    
    
    ofstream outfile;
    outfile.open ("etapi_fit.txt");
    for(int i=0; i<NumBins; i++){
        //myfile << "Writing this to a file.\n";
        outfile << mass_graph[i] <<" "<< etaPiSWave_Mean[i]<<" " << etaPiSWave_stdev[i]<<" " << etaPiP0Wave_Mean[i]<<" " << etaPiP0Wave_stdev[i]<<" " << etaPiP1mWave_Mean[i]<<" " << etaPiP1mWave_stdev[i]<<" " << etaPiD0Wave_Mean[i]<<" " << etaPiD0Wave_stdev[i]<<" " << etaPiD1Wave_Mean[i]<<" " << etaPiD1Wave_stdev[i]<<" " << etaPiP1pWave_Mean[i]<<" " << etaPiP1pWave_stdev[i]<<" " << etaPiDpWave_Mean[i]<<" " << etaPiDpWave_stdev[i]<<" " << allInt_Mean[i]<<" " << allInt_stdev[i] << endl;

        
      /*  in >> mass[i][line]
        >> etaPiSWave[i][line] >> etaPiSWavee[i][line]
        >> etaPiP0Wave[i][line] >> etaPiP0Wavee[i][line]
        >> etaPiP1mWave[i][line] >> etaPiP1mWavee[i][line]
        >> etaPiD0Wave[i][line] >> etaPiD0Wavee[i][line]
        >> etaPiD1Wave[i][line] >> etaPiD1Wavee[i][line]
        >> etaPiP1pWave[i][line] >> etaPiP1pWavee[i][line]
        >> etaPiDpWave[i][line] >> etaPiDpWavee[i][line]
        >> all[i][line] >> alle[i][line];
       */
    }
    outfile.close();
    
}

