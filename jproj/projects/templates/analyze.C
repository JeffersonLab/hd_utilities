
/********************************************************************
 *
 * 2015/01/27 Kei Moriya
 *
 * Take in formatted file from format_jobs_data.C and
 * do analysis.
 *
 * The script format_jobs_data.C has added to the original
 * file the unix time of each stage time, as well as the
 * integer values of mem and vmem (in kb).
 *
 * It has also changed the column error into errorCode,
 * since the original error could have been one of
 * - NULL
 * - Job timed out.
 * - exceeded resource limit.
 * The error code will change these to 0, 1, 2, respectively.
 *
 ********************************************************************/

#include <iostream>
#include <iomanip>

Int_t convert_to_sec(std::string input, Bool_t debug = false);

int analyze(Int_t TOTALTIME = 80, Bool_t debug = false){

  gStyle->SetOptStat(0);
  gStyle->SetTitleOffset(1.250,"Y");

  // Read in formatted txt file of analysis results
  char filename[200];
  sprintf(filename,"formatted_jobs_data.txt");
  ifstream IN(filename);
  if(!IN){
    cout << "file " << filename << " does not exist" << endl;
    abort();
  }
  Int_t nlines = 0;

  // Count how many jobs were processed
  system("wc -l formatted_jobs_data.txt > ___tmp_wc.txt");
  ifstream IN_wc("___tmp_wc.txt");
  Int_t NTOTAL;
  IN_wc >> NTOTAL;
  system("rm -f ___tmp_wc.txt");
  cout << "total of " << NTOTAL << " files to process..." << endl;

  char command[200];
  sprintf(command,"mkdir -p figures");
  system(command);

  // Variables to read in
  Int_t id, run, file, jobId;
  std::string timeChangeDay, timeChangeTime;
  std::string hostname, status, exitCode, result;
  std::string timeSubmittedDay,  timeSubmittedTime;  Long64_t utimeSubmitted;
  std::string timeDependencyDay, timeDependencyTime; Long64_t utimeDependency;
  std::string timePendingDay,    timePendingTime;    Long64_t utimePending;
  std::string timeStagingInDay,  timeStagingInTime;  Long64_t utimeStagingIn;
  std::string timeActiveDay,     timeActiveTime;     Long64_t utimeActive;
  std::string timeStagingOutDay, timeStagingOutTime; Long64_t utimeStagingOut;
  std::string timeCompleteDay,   timeCompleteTime;   Long64_t utimeComplete;
  std::string walltime, cput, mem, vmem, error;      Int_t errorCode;
  Int_t nevents, timeCopy, timePlugin;

  Int_t walltime_sec, cput_sec;
  Int_t hostnameNum;

  Int_t mem_int, vmem_int;
  char command[400];
  char hname[400];
  char label[400];

  Int_t nTotal = 0;
  const Int_t colors[7] = {kBlack, kRed, kYellow+2, kGreen+2, kBlue, kMagenta, 18};

  ///////////////////////////////////////////
  //                                       //
  //   Create outfile, histograms, graphs  //
  //                                       //
  ///////////////////////////////////////////
  TFile *outfile = new TFile("analysis.root","recreate");

  // ---   Used mem, vmem for each job                                                 --- //
  TH1F *hmem = new TH1F("hmem",";mem used (MB);",1000,0, 3. * 1000.);
  TH1F *hvmem = new TH1F("hvmem",";vmem used (MB);",1000,0, 5. * 1000.);

  // ---   walltime, timeCopy, timePlugin                                              --- //
  TH1F *hcopyTime = new TH1F("hcopyTime",";copy time (min);",120,0,60); // 60 min/120 bins = 30 sec. bins
  TH1F *hwalltime = new TH1F("hwalltime",";wall time (hrs);",300,0,25); // 25 hours/300 bins = 5 min. bins
  TH1F *htimePlugin = new TH1F("htimePlugin",";plugin time (hrs);",300,0,25); // 25 hours/300 bins = 5 min. bins

  TGraph *gcput_walltime = new TGraph(NTOTAL);
  gcput_walltime->SetName("gcput_walltime");

  TGraph *gcput_timePlugin = new TGraph(NTOTAL);
  gcput_timePlugin->SetName("gcput_timePlugin");

  // ---   Processing time / event                                                     --- //
  TH1F *htimePlugin_per_event = new TH1F("htimePlugin_per_event",";time to process / event (sec);",10000,0,10);

  // Variables for calculating average time per event.
  // We calculate this for all files and also for files
  // where the average was less than 0.1 s.
  Double_t timePluginPerEvent_average = 0;
  Int_t    nTimePluginPerEvent_average = 0;
  Double_t timePluginPerEvent_average_100ms = 0;
  Int_t    nTimePluginPerEvent_average_100ms = 0;

  // ---   OBSOLETE                                                                    --- //
  // TGraph *gwalltime_activeTime = new TGraph(NTOTAL);
  // gwalltime_activeTime->SetName("gwalltime_activeTime");
  // 
  // TH1F *hhostnameNum = new TH1F("hhostnameNum",";host name num;",150,140100,140250);
  // TGraph *ghostnameNum_jobNum = new TGraph(NTOTAL);
  // ghostnameNum_jobNum->SetName("ghostnameNum_jobNum");

  // Define the 7 stages of grief:
  // 1. submitted
  // 2. dependency
  // 3. pending
  // 4. stagingIn
  // 5. active
  // 6. stagingOut
  // 7. complete
  const Int_t NSTAGES = 7;
  const char STAGE[NSTAGES][40] = {"submitted", "dependency", "pending",
				   "stagingIn", "active",     "stagingOut",
				   "complete"};

  const char STAGETEXT[NSTAGES][120] = {"submit time since launch",     "dependency time since launch", "pending time since launch",
					"staging in time since launch", "active time since launch",     "staging out time since launch",
					"complete time since launch"};

  const Double_t markersize[NSTAGES] = {1.2, 0.6, 0.8,
					1.2, 0.6, 1.2,
					0.6};

  // Histograms of each times since launch
  TH1F *hTimeSinceLaunch[NSTAGES];
  for(Int_t i=0;i<NSTAGES;i++){
    sprintf(hname,"hTimeSinceLaunch_%s",STAGE[i]);
    sprintf(label,"%s (hrs)",STAGETEXT[i]);
    hTimeSinceLaunch[i] = new TH1F(hname,label,TOTALTIME * 10,0,TOTALTIME);
  }

  Long64_t INITIAL_SUBMIT;
  Long64_t timeSinceLaunch[NSTAGES];

  // These will look at the time each job spent in each stage
  // by looking at the difference in two different times
  TH1F *hStageTimesEachJob[NSTAGES-1];
  TH1F *hStageTimesEachJob_sec[NSTAGES-1];
  for(Int_t i=0;i<NSTAGES-1;i++){
    sprintf(hname,"hStageTimesEachJob_%s_%s",STAGE[i+1],STAGE[i]);
    hStageTimesEachJob[i] = new TH1F(hname,";job # ; time (hrs);",NTOTAL,0,NTOTAL);

    sprintf(hname,"hStageTimesEachJob_sec_%s_%s",STAGE[i+1],STAGE[i]);
    hStageTimesEachJob_sec[i] = new TH1F(hname,";job # ; time (sec);",NTOTAL,0,NTOTAL);
  }

  // Times we check how many jobs were running.
  // Check every 10 min, over 60 hours.
  // const Int_t NCHECKTIMES = 360;
  // Long64_t CHECKTIMES[NCHECKTIMES];
  // Int_t TIMESINCEINITFORCHECKTIMES[NCHECKTIMES];
  // Int_t jobsRunningAtCheckTimes[NCHECKTIMES];

  // TH1F *hdiff_walltime_activeTime = new TH1F("hdiff_walltime_activeTime",";diff of wall time - active time (sec);",300,-600,600);

  ///////////////////////////////////////////
  //                                       //
  //   Read in results from input file     //
  //                                       //
  ///////////////////////////////////////////
  while(IN >> id >> run >> file >> jobId
	>> timeChangeDay >> timeChangeTime
	>> hostname >> status >> exitCode >> result
	>> timeSubmittedDay  >> timeSubmittedTime
	>> timeDependencyDay >> timeDependencyTime
	>> timePendingDay    >> timePendingTime
	>> timeStagingInDay  >> timeStagingInTime
	>> timeActiveDay     >> timeActiveTime
	>> timeStagingOutDay >> timeStagingOutTime
	>> timeCompleteDay   >> timeCompleteTime
	>> walltime >> cput >> mem >> vmem >> errorCode
	>> utimeSubmitted
	>> utimeDependency
	>> utimePending
	>> utimeStagingIn
	>> utimeActive
	>> utimeStagingOut
	>> utimeComplete
	>> mem_int >> vmem_int
	>> nevents >> timeCopy >> timePlugin){
    
    if(debug){
      cout << "---------- entry " << nTotal << " -------------------" << endl;
      cout << "id = " << id << " run = " << run << " file = " << file << " jobId = " << jobId << endl
	   << " timeChangeDay = " << timeChangeDay << " timeChangeTime = " << timeChangeTime << endl
	   << " hostname = " << hostname << " status = " << status << " exitCode = " << exitCode << " result = " << result << endl
	   << " timeSubmittedDay = " << timeSubmittedDay  << " timeSubmittedTime = " << timeSubmittedTime << endl
	   << " timeDependencyDay = " << timeDependencyDay << " timeDependencyTime = " << timeDependencyTime << endl
	   << " timePendingDay = " << timePendingDay    << " timePendingTime = " << timePendingTime << endl
	   << " timeStagingInDay = " << timeStagingInDay  << " timeStagingInTime = " << timeStagingInTime << endl
	   << " timeActiveDay = " << timeActiveDay     << " timeActiveTime = " << timeActiveTime << endl
	   << " timeStagingOutDay = " << timeStagingOutDay << " timeStagingOutTime = " << timeStagingOutTime << endl
	   << " timeCompleteDay = " << timeCompleteDay   << " timeCompleteTime = " << timeCompleteTime << endl
	   << " walltime = " << walltime << " cput = " << cput << " mem = " << mem << " vmem = " << vmem << " errorCode = " << errorCode << endl
	   << " utimeSubmitted = " << utimeSubmitted << endl
	   << " utimeDependency = " << utimeDependency << endl
	   << " utimePending = " << utimePending << endl
	   << " utimeStagingIn = " << utimeStagingIn << endl
	   << " utimeActive = " << utimeActive << endl
	   << " utimeStagingOut = " << utimeStagingOut << endl
	   << " utimeComplete = " << utimeComplete << endl
	   << " mem = " << mem_int << " vmem = " << vmem_int << endl;
    }

    nTotal++;
    if(nTotal % 1000 == 0) cout << "processed " << setw(5) << nTotal << " / " << NTOTAL << endl;

    // Get launch start time from first job
    if(nTotal==1){
      INITIAL_SUBMIT = utimeSubmitted;

      // Set up times to check how many jobs were running
      // for(Int_t i=0;i<NCHECKTIMES;i++){
      //   CHECKTIMES[i] = INITIAL_SUBMIT + 600 * i;
      //   TIMESINCEINITFORCHECKTIMES[i] = 600 * i;
      // }
    }
    
    // How many jobs were running at CHECKTIME[i]
    // for(Int_t i=0;i<NCHECKTIMES;i++){
    //   if(utimeActive < CHECKTIMES[i] && CHECKTIMES[i] < utimeStagingOut)
    //     jobsRunningAtCheckTimes[i]++;
    // }
    
    ///////////////////////////////////////////
    //                                       //
    //   Fill in histograms for # of jobs    //
    //   arriving at each stage              //
    //                                       //
    ///////////////////////////////////////////
    // submit time
    timeSinceLaunch[0] = utimeSubmitted   - INITIAL_SUBMIT;
    // dependency
    timeSinceLaunch[1] = utimeDependency  - INITIAL_SUBMIT;
    // pending
    timeSinceLaunch[2] = utimePending     - INITIAL_SUBMIT;
    // staging in
    timeSinceLaunch[3] = utimeStagingIn   - INITIAL_SUBMIT;
    // active
    timeSinceLaunch[4] = utimeActive      - INITIAL_SUBMIT;
    // staging out
    timeSinceLaunch[5] = utimeStagingOut  - INITIAL_SUBMIT;
    // complete
    timeSinceLaunch[6] = utimeComplete    - INITIAL_SUBMIT;

    for(Int_t i=0;i<NSTAGES;i++){
      hTimeSinceLaunch[i]->Fill(timeSinceLaunch[i] / 3600.);
    }

    ///////////////////////////////////////////
    //                                       //
    //   Fill in histograms for mem, vmem    //
    //                                       //
    ///////////////////////////////////////////
    hmem->Fill(mem_int / 1024.);
    hvmem->Fill(vmem_int / 1024.);

    ///////////////////////////////////////////
    //                                       //
    //   Fill in histograms for walltime,    //
    //   timeCopy, timePlugin                //
    //                                       //
    ///////////////////////////////////////////

    // convert walltime, cput into seconds
    Int_t hour, min, sec;
    walltime_sec = convert_to_sec(walltime);
    cput_sec = convert_to_sec(cput);

    // histograms for wall time, copy time, plugin time
    hcopyTime->Fill(timeCopy / 60.);
    hwalltime->Fill(walltime_sec / 3600.);
    htimePlugin->Fill(timePlugin / 3600.);

    // compare CPU time with wall time
    gcput_walltime->SetPoint(nTotal-1,walltime_sec/3600.,cput_sec/3600.);

    // compare CPU time with plugin time
    gcput_timePlugin->SetPoint(nTotal-1,timePlugin/3600.,cput_sec/3600.);

    ///////////////////////////////////////////
    //                                       //
    //   Calculate processing time per event //
    //                                       //
    ///////////////////////////////////////////
    // set to -1 if # of events is 0 or -999 (nothing reported)
    Double_t timePlugin_per_event = nevents == 0 ? -1 : 1. * timePlugin / nevents;
    if(nevents == -999) timePlugin_per_event = -1;
    htimePlugin_per_event->Fill(timePlugin_per_event);
    // cout << timePlugin_per_event << endl;

    if(timePlugin_per_event > 0){
      timePluginPerEvent_average += timePlugin_per_event;
      nTimePluginPerEvent_average++;
    }

    if(timePlugin_per_event > 0 && timePlugin_per_event < 0.1){
      timePluginPerEvent_average_100ms += timePlugin_per_event;
      nTimePluginPerEvent_average_100ms++;
    }

    // some jobs have cput > 20 hours but plugin time less than 2 hours
    if(cput_sec/3600. > 20. && timePlugin/3600. < 2.){
      cout << "jobId  = " << jobId << ", run  = " << run
	   << ", file = " << file  << ", cput = " << cput_sec/3600.
	   << " hours, plugin time = " << timePlugin/3600.
	   << " hours, copy time = "   << timeCopy/3600.
	   << " hours, wall time = " << walltime_sec/3600.
	   << " hours, hostname = " << hostname << endl;
    }

    // ---   OBSOLETE   --- //
    // compare wall time against time spent in active
    // gwalltime_activeTime->SetPoint(nTotal-1,(utimeStagingOut - utimeActive)/3600.,walltime_sec/3600.);
    // hdiff_walltime_activeTime->Fill((utimeStagingOut - utimeActive) - walltime_sec);

    // How much time each job spent in each stage
    hStageTimesEachJob[0]->SetBinContent(nTotal,(utimeDependency - utimeSubmitted)/3600.);
    hStageTimesEachJob[1]->SetBinContent(nTotal,(utimePending - utimeDependency)/3600.);
    hStageTimesEachJob[2]->SetBinContent(nTotal,(utimeStagingIn - utimePending)/3600.);
    hStageTimesEachJob[3]->SetBinContent(nTotal,(utimeActive - utimeStagingIn)/3600.);
    hStageTimesEachJob[4]->SetBinContent(nTotal,(utimeStagingOut - utimeActive)/3600.);
    hStageTimesEachJob[5]->SetBinContent(nTotal,(utimeComplete - utimeStagingOut)/3600.);

    // How much time each job spent in each stage (sec)
    hStageTimesEachJob_sec[0]->SetBinContent(nTotal,utimeDependency - utimeSubmitted);
    hStageTimesEachJob_sec[1]->SetBinContent(nTotal,utimePending - utimeDependency);
    hStageTimesEachJob_sec[2]->SetBinContent(nTotal,utimeStagingIn - utimePending);
    hStageTimesEachJob_sec[3]->SetBinContent(nTotal,utimeActive - utimeStagingIn);
    hStageTimesEachJob_sec[4]->SetBinContent(nTotal,utimeStagingOut - utimeActive);
    hStageTimesEachJob_sec[5]->SetBinContent(nTotal,utimeComplete - utimeStagingOut);

    // if(errorCode==0){
    //   hstagingOut_active_noError->SetBinContent(nTotal,(utimeStagingOut - utimeActive)/3600.);
    // }else if(errorCode==1){
    //   hstagingOut_active_timedOut->SetBinContent(nTotal,(utimeStagingOut - utimeActive)/3600.);
    // }else if(errorCode==2){
    //   hstagingOut_active_resourceLimit->SetBinContent(nTotal,(utimeStagingOut - utimeActive)/3600.);
    // }

    // get hostnameNum
    // hostnameNum = atoi(hostname.substr(4).c_str());
    // cout << hostname.substr(4).c_str() << " " << hostnameNum << endl;
    // hhostnameNum->Fill(hostnameNum);
    // ghostnameNum_jobNum->SetPoint(nTotal-1,nTotal-1,hostnameNum);
    // if(hostnameNum < 140000)
    //   cout << nTotal-1 << " " << nTotal << " " << hostnameNum << endl;

  } // end of reading in each entry
  //_________________________________________________________________________________

  ///////////////////////////////////////////
  //                                       //
  //   Calculate processing time per event //
  //                                       //
  ///////////////////////////////////////////
  timePluginPerEvent_average /= nTimePluginPerEvent_average;
  timePluginPerEvent_average_100ms /= nTimePluginPerEvent_average_100ms;
  
  // Create a stacked histogram for how much time
  // each job spent at each stage
  THStack *hs = new THStack("hs","");
  for(Int_t i=0;i<NSTAGES-1;i++){
    hStageTimesEachJob[i]->SetFillColor(colors[i]);
    hStageTimesEachJob[i]->SetLineColor(colors[i]);
    hs->Add(hStageTimesEachJob[i]);

    hStageTimesEachJob_sec[i]->SetFillColor(colors[i]);
    hStageTimesEachJob_sec[i]->SetLineColor(colors[i]);
  }
  hs->Write();

  cout << "--------------------------------------------------" << endl;
  cout << "total                            : " << nTotal << endl;
  //_________________________________________________________________________________

  // Fill cumulative hists for times since launch.
  // These cumulative hists show the # of jobs that have
  // reached a certain stage since launch time.
  TH1F *hCumulativeTimeSinceLaunch[NSTAGES];
  for(Int_t i=0;i<NSTAGES;i++){
    sprintf(hname,"hCumulativeTimeSinceLaunch_%s",STAGE[i]);
    hCumulativeTimeSinceLaunch[i] = (TH1F*)hTimeSinceLaunch[i]->Clone(hname);
    hCumulativeTimeSinceLaunch[i]->Clear();

    // Set bins
    for(Int_t bin=0;bin<=hCumulativeTimeSinceLaunch[i]->GetNbinsX()+1;bin++){
      Double_t total = 0;
      for(Int_t bin2=0;bin2<=bin;bin2++){
	total += hTimeSinceLaunch[i]->GetBinContent(bin2);
      }
      hCumulativeTimeSinceLaunch[i]->SetBinContent(bin,total);
    }
  }
  //_________________________________________________________________________________

  // TGraph *gjobsRunningAtCheckTimes = new TGraph(NCHECKTIMES);
  // gjobsRunningAtCheckTimes->SetName("gjobsRunningAtCheckTimes");
  // for(Int_t i=0;i<NCHECKTIMES;i++){
  //   gjobsRunningAtCheckTimes->SetPoint(i,TIMESINCEINITFORCHECKTIMES[i] / 3600.,jobsRunningAtCheckTimes[i]);
  // }
  
  // Write output ROOT file
  gcput_walltime->Write();
  gcput_timePlugin->Write();

  // obsolete
  // ghostnameNum_jobNum->Write();
  // gwalltime_activeTime->Write();
  // gjobsRunningAtCheckTimes->Write();
  outfile->Write();
  //_________________________________________________________________________________

  ///////////////////////////////////////////
  //                                       //
  //              Draw results             //
  //                                       //
  ///////////////////////////////////////////
  Int_t plots = 1;

  TLatex *latex = new TLatex();
  latex->SetTextColor(kBlue);
  latex->SetTextSize(0.040);
  latex->SetTextFont(132);
  latex->SetTextAlign(12);
  latex->SetNDC(1);
  char text[400];

  TCanvas *canvas = new TCanvas("canvas","canvas",1600,800);
  canvas->SetTopMargin(0.035);
  canvas->SetRightMargin(0.02);
  canvas->SetLeftMargin(0.12);

  TLegend *legend = new TLegend(0.15,0.80,0.90,0.95);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->SetNColumns(3);
  legend->SetTextSize(0.045);
  //_________________________________________________________________________________

  // vmem
  hvmem->Draw();
  sprintf(hname,"figures/%3.3d______vmem.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______vmem.png",plots);
  canvas->SaveAs(hname);
  plots++;
  //_________________________________________________________________________________

  // mem
  hmem->Draw();
  sprintf(hname,"figures/%3.3d______mem.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______mem.png",plots);
  canvas->SaveAs(hname);
  plots++;
  //_________________________________________________________________________________

  // copy time
  hcopyTime->SetFillColor(kBlue);
  hcopyTime->SetLineColor(kBlue);
  hcopyTime->Draw();
  sprintf(hname,"figures/%3.3d______copyTime.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______copyTime.png",plots);
  canvas->SaveAs(hname);
  plots++;
  //_________________________________________________________________________________

  // wall time and plugin time
  legend->Clear();
  hwalltime->SetFillStyle(1001);
  hwalltime->SetFillColor(kRed);
  hwalltime->SetLineColor(kRed);
  if(htimePlugin->GetMaximum() > hwalltime->GetMaximum()) hwalltime->SetMaximum(htimePlugin->GetMaximum() * 1.05);
  hwalltime->SetAxisRange(0,5);
  hwalltime->Draw();
  legend->AddEntry(hwalltime,"wall time","L");

  htimePlugin->SetFillStyle(0);
  htimePlugin->SetFillColor(kBlue);
  htimePlugin->SetLineColor(kBlue);
  htimePlugin->Draw("same");
  legend->AddEntry(htimePlugin,"plugin time","L");
  legend->Draw("same");

  sprintf(hname,"figures/%3.3d______walltime.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______walltime.png",plots);
  canvas->SaveAs(hname);
  plots++;
  //_________________________________________________________________________________
  
  // cpu time vs walltime
  gcput_walltime->SetMarkerStyle(20);
  gcput_walltime->SetMarkerSize(0.5);
  gcput_walltime->SetMarkerColor(kBlue);
  gcput_walltime->SetTitle("");
  gcput_walltime->GetXaxis()->SetTitle("wall time (hrs)");
  gcput_walltime->GetYaxis()->SetTitle("cpu time (hrs)");

  TF1 *flinear[6];
  char fname[80];
  for(Int_t i=0;i<6;i++){
    sprintf(fname,"flinear_%d",i);
    flinear[i] = new TF1(fname,"[0]*x",0,25);
    flinear[i]->SetParameter(0,1.*(i+1.));
    flinear[i]->SetLineColor(colors[i]);
    flinear[i]->SetLineStyle(3);
  }

  // plugin time  hcopyTime->SetFillStyle(0);
  gcput_walltime->GetXaxis()->SetLimits(0,5);
  gcput_walltime->SetMinimum(0);
  gcput_walltime->SetMaximum(30);
  gcput_walltime->Draw("AP");

  for(Int_t i=0;i<6;i++){
    flinear[i]->Draw("same");
  }

  sprintf(hname,"figures/%3.3d______cput_walltime.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______cput_walltime.png",plots);
  canvas->SaveAs(hname);
  plots++;
  //_________________________________________________________________________________

  // cpu time vs timePlugin
  gcput_timePlugin->SetMarkerStyle(20);
  gcput_timePlugin->SetMarkerSize(0.5);
  gcput_timePlugin->SetMarkerColor(kBlue);
  gcput_timePlugin->SetTitle("");
  gcput_timePlugin->GetXaxis()->SetTitle("plugin time (hrs)");
  gcput_timePlugin->GetYaxis()->SetTitle("cpu time (hrs)");

  gcput_timePlugin->GetXaxis()->SetLimits(0,5);
  gcput_timePlugin->SetMaximum(30);
  gcput_timePlugin->Draw("AP");

  for(Int_t i=0;i<6;i++){
    flinear[i]->Draw("same");
  }

  sprintf(hname,"figures/%3.3d______cput_timePlugin.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______cput_timePlugin.png",plots);
  canvas->SaveAs(hname);
  plots++;
  //_________________________________________________________________________________

  // plugin time per event
  canvas->SetLogx(1);
  htimePlugin_per_event->SetAxisRange(0.001,10);
  htimePlugin_per_event->Draw();

  sprintf(text,"average (<0.1 s): %5.3f s",timePluginPerEvent_average_100ms);
  latex->DrawLatex(0.60,0.25,text);

  TPad *spad = new TPad("spad","spad",0.50,0.50,0.95,0.95);
  spad->SetBorderSize(0);
  spad->SetFillStyle(0);
  spad->AbsCoordinates(1);
  spad->Draw();
  spad->cd();
  spad->SetLogy(1);

  TH1F *htimePlugin_per_event___copy = (TH1F*)htimePlugin_per_event->Clone("htimePlugin_per_event___copy");
  htimePlugin_per_event___copy->Draw();

  sprintf(text,"average: %5.3f s",timePluginPerEvent_average);
  latex->SetTextSize(0.120);
  latex->DrawLatex(0.40,0.55,text);

  sprintf(hname,"figures/%3.3d______timePlugin_per_event.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______timePlugin_per_event.png",plots);
  canvas->SaveAs(hname);
  canvas->SetLogx(0);
  plots++;
  canvas->cd();
  canvas->Clear();
  //_________________________________________________________________________________

  // time since submission (shows # of jobs arriving at each stage against time)
  legend->SetX1NDC(0.42);
  legend->SetX2NDC(0.98);
  legend->SetY1NDC(0.15);
  legend->SetY2NDC(0.35);
  legend->Clear();

  for(Int_t i=0;i<NSTAGES;i++){
    hCumulativeTimeSinceLaunch[i]->SetMarkerStyle(20);
    hCumulativeTimeSinceLaunch[i]->SetMarkerSize(markersize[i]);
    hCumulativeTimeSinceLaunch[i]->SetMarkerColor(colors[i]);
    hCumulativeTimeSinceLaunch[i]->SetLineColor(colors[i]);
    hCumulativeTimeSinceLaunch[i]->GetXaxis()->SetTitle("time since start (hrs)");
    hCumulativeTimeSinceLaunch[i]->GetYaxis()->SetTitle("# jobs arriving at stage");
    if(i==0) hCumulativeTimeSinceLaunch[i]->Draw();
    else     hCumulativeTimeSinceLaunch[i]->Draw("same");
    legend->AddEntry(hCumulativeTimeSinceLaunch[i],STAGE[i],"P");
  }
  legend->Draw("same");

  sprintf(hname,"figures/%3.3d______CumulativetimeSinceLaunch.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______CumulativetimeSinceLaunch.png",plots);
  canvas->SaveAs(hname);
  plots++;
  //_________________________________________________________________________________

  // How much time each job spent at each stage
  legend->Clear();
  hs->Draw();
  hs->GetXaxis()->SetTitle("job #");
  hs->GetYaxis()->SetTitle("time (hrs)");
  hs->Draw();
  for(Int_t i=0;i<NSTAGES-1;i++){
    legend->AddEntry(hStageTimesEachJob[i],STAGE[i],"F");
  }
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______stageTimesEachJob_stacked.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______stageTimesEachJob_stacked.png",plots);
  canvas->SaveAs(hname);
  plots++;
  //_________________________________________________________________________________

  // Individual plots of how much time each job
  // spent at each stage
  legend->SetX1NDC(0.70);
  legend->SetX2NDC(0.90);
  legend->SetY1NDC(0.90);
  legend->SetY2NDC(0.95);

  for(Int_t i=0;i<NSTAGES-1;i++){
    legend->Clear();
    hStageTimesEachJob[i]->SetMinimum(0);
    hStageTimesEachJob[i]->Draw();
    legend->AddEntry(hStageTimesEachJob[i],STAGE[i],"F");
    legend->Draw("same");
    sprintf(hname,"figures/%3.3d______stageTimesEachJob_%s.pdf",plots,STAGE[i]);
    canvas->SaveAs(hname);
    sprintf(hname,"figures/%3.3d______stageTimesEachJob_%s.png",plots,STAGE[i]);
    canvas->SaveAs(hname);
    plots++;
  }
  //_________________________________________________________________________________

  // Individual plots of how much time each job
  // spent at each stage (in sec)
  for(Int_t i=0;i<NSTAGES-1;i++){
    legend->Clear();
    hStageTimesEachJob_sec[i]->SetMinimum(0);
    hStageTimesEachJob_sec[i]->Draw();
    legend->AddEntry(hStageTimesEachJob_sec[i],STAGE[i],"F");
    legend->Draw("same");
    sprintf(hname,"figures/%3.3d______stageTimesEachJob_%s_sec.pdf",plots,STAGE[i]);
    canvas->SaveAs(hname);
    sprintf(hname,"figures/%3.3d______stageTimesEachJob_%s_sec.png",plots,STAGE[i]);
    canvas->SaveAs(hname);
    plots++;
  }
  //_________________________________________________________________________________

  // plot active time against job #
  // legend->Clear();
  // hstagingOut_active_noError->SetLineColor(kBlue);
  // hstagingOut_active_noError->SetFillColor(kBlue);
  // hstagingOut_active_noError->SetMaximum(hstagingOut_active_timedOut->GetMaximum() * 1.05);
  // hstagingOut_active_noError->Draw();
  // legend->AddEntry(hstagingOut_active_noError,"no error","F");

  // hstagingOut_active_timedOut->SetLineColor(kBlue-10);
  // hstagingOut_active_timedOut->SetFillColor(kBlue-10);
  // hstagingOut_active_timedOut->Draw("same");
  // legend->AddEntry(hstagingOut_active_timedOut,"timed out","F");

  // plot active time against error
  // legend->SetX1NDC(0.15);
  // legend->SetX2NDC(0.90);

  // hstagingOut_active_resourceLimit->SetLineColor(kCyan);
  // hstagingOut_active_resourceLimit->SetFillColor(kCyan);
  // hstagingOut_active_resourceLimit->Draw("same");
  // legend->AddEntry(hstagingOut_active_resourceLimit,"resource limit","F");
  // legend->Draw("same");
  // sprintf(hname,"figures/%3.3d______activeByStatus.pdf",plots);
  // canvas->SaveAs(hname);
  // sprintf(hname,"figures/%3.3d______activeByStatus.png",plots);
  // canvas->SaveAs(hname);
  // plots++;

  /*
  // # of jobs at each stage at a given time is given by difference of each accumulated stage
  TH1F *hnumjobs_submit = (TH1F*)haccumulate_submitTimeSinceInit->Clone("hnumjobs_submit");
  hnumjobs_submit->Add(haccumulate_dependencyTimeSinceInit,-1.);

  TH1F *hnumjobs_dependency = (TH1F*)haccumulate_dependencyTimeSinceInit->Clone("hnumjobs_dependency");
  hnumjobs_dependency->Add(haccumulate_pendingTimeSinceInit,-1.);

  TH1F *hnumjobs_pending = (TH1F*)haccumulate_pendingTimeSinceInit->Clone("hnumjobs_pending");
  hnumjobs_pending->Add(haccumulate_stagingInTimeSinceInit,-1.);

  TH1F *hnumjobs_stagingIn = (TH1F*)haccumulate_stagingInTimeSinceInit->Clone("hnumjobs_stagingIn");
  hnumjobs_stagingIn->Add(haccumulate_activeTimeSinceInit,-1.);

  TH1F *hnumjobs_active = (TH1F*)haccumulate_activeTimeSinceInit->Clone("hnumjobs_active");
  hnumjobs_active->Add(haccumulate_stagingOutTimeSinceInit,-1.);

  TH1F *hnumjobs_stagingOut = (TH1F*)haccumulate_stagingOutTimeSinceInit->Clone("hnumjobs_stagingOut");
  hnumjobs_stagingOut->Add(haccumulate_completeTimeSinceInit,-1.);

  legend->Clear();
  legend->SetX1NDC(0.25);
  legend->SetX2NDC(0.95);
  legend->SetY1NDC(0.75);
  legend->SetY2NDC(0.90);
  hnumjobs_submit->SetMarkerStyle(20);
  hnumjobs_submit->SetMarkerSize(1.2);
  hnumjobs_submit->SetMarkerColor(colors[0]);
  hnumjobs_submit->SetLineColor(colors[0]);
  hnumjobs_submit->GetXaxis()->SetTitle("time since start (hrs)");
  hnumjobs_submit->GetYaxis()->SetTitle("# jobs at each stage");
  hnumjobs_submit->SetMinimum(0);
  hnumjobs_submit->SetMaximum(7000);
  hnumjobs_submit->Draw();
  legend->AddEntry(hnumjobs_submit,"submit","L");

  hnumjobs_dependency->SetMarkerStyle(20);
  hnumjobs_dependency->SetMarkerSize(0.6);
  hnumjobs_dependency->SetMarkerColor(colors[1]);
  hnumjobs_dependency->SetLineColor(colors[1]);
  hnumjobs_dependency->Draw("same");
  legend->AddEntry(hnumjobs_dependency,"dependency","L");

  hnumjobs_pending->SetMarkerStyle(20);
  hnumjobs_pending->SetMarkerSize(0.8);
  hnumjobs_pending->SetMarkerColor(colors[2]);
  hnumjobs_pending->SetLineColor(colors[2]);
  hnumjobs_pending->Draw("same");
  legend->AddEntry(hnumjobs_pending,"pending","L");

  hnumjobs_stagingIn->SetMarkerStyle(20);
  hnumjobs_stagingIn->SetMarkerSize(1.2);
  hnumjobs_stagingIn->SetMarkerColor(colors[3]);
  hnumjobs_stagingIn->SetLineColor(colors[3]);
  hnumjobs_stagingIn->Draw("same");
  legend->AddEntry(hnumjobs_stagingIn,"staging in","L");

  hnumjobs_active->SetMarkerStyle(20);
  hnumjobs_active->SetMarkerSize(0.6);
  hnumjobs_active->SetMarkerColor(colors[4]);
  hnumjobs_active->SetLineColor(colors[4]);
  hnumjobs_active->Draw("same");
  legend->AddEntry(hnumjobs_active,"active","L");

  hnumjobs_stagingOut->SetMarkerStyle(20);
  hnumjobs_stagingOut->SetMarkerSize(1.2);
  hnumjobs_stagingOut->SetMarkerColor(colors[5]);
  hnumjobs_stagingOut->SetLineColor(colors[5]);
  hnumjobs_stagingOut->Draw("same");
  legend->AddEntry(hnumjobs_stagingOut,"staging out","L");

  legend->Draw("same");

  sprintf(hname,"figures/%3.3d______numjobs_eachstage.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______numjobs_eachstage.png",plots);
  canvas->SaveAs(hname);
  plots++;
  */

  // jobs running at a given time
  // gjobsRunningAtCheckTimes->SetMarkerStyle(20);
  // gjobsRunningAtCheckTimes->SetMarkerSize(0.5);
  // gjobsRunningAtCheckTimes->SetMarkerColor(kBlue);
  // gjobsRunningAtCheckTimes->SetTitle("");
  // gjobsRunningAtCheckTimes->GetXaxis()->SetTitle("time since launch (hrs)");
  // gjobsRunningAtCheckTimes->GetYaxis()->SetTitle("# active jobs");

  // gjobsRunningAtCheckTimes->Draw("AP");

  // sprintf(hname,"figures/%3.3d______jobsRunningAtCheckTimes.pdf",plots);
  // canvas->SaveAs(hname);
  // sprintf(hname,"figures/%3.3d______jobsRunningAtCheckTimes.png",plots);
  // canvas->SaveAs(hname);
  // plots++;
  //_________________________________________________________________________________

  // used hostnames
  // hhostnameNum->SetTitle("");
  // hhostnameNum->GetXaxis()->SetTitle("hostname");
  // hhostnameNum->GetYaxis()->SetTitle("");
  // hhostnameNum->GetXaxis()->SetNoExponent(kTRUE);
  // hhostnameNum->Draw("");

  // sprintf(hname,"figures/%3.3d______hostnameNum.pdf",plots);
  // canvas->SaveAs(hname);
  // sprintf(hname,"figures/%3.3d______hostnameNum.png",plots);
  // canvas->SaveAs(hname);
  // plots++;
  //_________________________________________________________________________________

  // used hostname against job #
  // ghostnameNum_jobNum->SetTitle("");
  // ghostnameNum_jobNum->GetXaxis()->SetTitle("job #");
  // ghostnameNum_jobNum->GetYaxis()->SetTitle("hostname");
  // ghostnameNum_jobNum->SetMarkerColor(kBlue);
  // ghostnameNum_jobNum->SetMarkerSize(0.8);
  // ghostnameNum_jobNum->SetMarkerStyle(20);
  // ghostnameNum_jobNum->SetLineColor(kBlue);
  // ghostnameNum_jobNum->SetLineWidth(1);
  // ghostnameNum_jobNum->SetLineStyle(20);
  // ghostnameNum_jobNum->GetYaxis()->SetNoExponent(kTRUE);
  // ghostnameNum_jobNum->SetMinimum(140000);
  // ghostnameNum_jobNum->SetMaximum(140250);
  // ghostnameNum_jobNum->Draw("AP");

  // sprintf(hname,"figures/%3.3d______hostnameNum_jobNum.pdf",plots);
  // canvas->SaveAs(hname);
  // sprintf(hname,"figures/%3.3d______hostnameNum_jobNum.png",plots);
  // canvas->SaveAs(hname);
  // plots++;
  //_________________________________________________________________________________
}

Int_t convert_to_sec(std::string input, Bool_t debug){
  // input is of form HH:MM:SS
  if(debug) cout << "input = " << input << endl;

  if(input == "NULL"){
    if(debug) cout << "input was NULL" << endl;
    return -999 * 3600;
  }else if(input == "-999"){
    if(debug) cout << "input was -999" << endl;
    return -999 * 3600;
  }

  Int_t hour, min, sec;
  hour = atoi(input.substr(0,2).c_str());
  min = atoi(input.substr(3,2).c_str());
  sec = atoi(input.substr(6,2).c_str());

  if(debug){
    cout << "hour = " << hour << endl;
    cout << "min = " << min << endl;
    cout << "sec = " << sec << endl;
  }

  return 3600 * hour + 60 * min + sec;
}
