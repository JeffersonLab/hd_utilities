
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

int analyze(Int_t TOTALTIME = 50, Bool_t debug = false){

  gStyle->SetOptStat(0);
  gStyle->SetTitleOffset(1.250,"Y");

  char filename[200];
  sprintf(filename,"formatted_jobs_data.txt");
  ifstream IN(filename);
  if(!IN){
    cout << "file " << filename << " does not exist" << endl;
    abort();
  }
  Int_t nlines = 0;

  system("wc -l formatted_jobs_data.txt > ___tmp_wc.txt");
  ifstream IN_wc("___tmp_wc.txt");
  Int_t NTOTAL;
  IN_wc >> NTOTAL;
  system("rm -f ___tmp_wc.txt");
  cout << "total of " << NTOTAL << " files to process..." << endl;

  char command[200];
  sprintf(command,"mkdir -p figures");
  system(command);

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

  Int_t nTotal = 0;
  const Int_t colors[7] = {kBlack, kRed, kYellow+2, kGreen+2, kBlue, kMagenta, 18};

  TFile *outfile = new TFile("analysis.root","recreate");
  TH1F *hmem = new TH1F("hmem",";mem used (MB);",1000,0, 3. * 1000.);
  TH1F *hvmem = new TH1F("hvmem",";vmem used (MB);",1000,0, 5. * 1000.);

  TH1F *hhostnameNum = new TH1F("hhostnameNum",";host name num;",150,140100,140250);
  TGraph *ghostnameNum_jobNum = new TGraph(NTOTAL);
  ghostnameNum_jobNum->SetName("ghostnameNum_jobNum");

  // Times:
  // 1. submitted
  // 2. dependency
  // 3. pending
  // 4. stagingIn
  // 5. active
  // 6. stagingOut
  // 7. complete
  
  // 7. - 1. (total)
  TH1F *hcomplete_submitted = new TH1F("hcomplete_submitted",";completed time - submitted time (hours);",400,0,40);
  
  // 2. - 1.
  TH1F *hdependency_submitted = new TH1F("hdependency_submitted",";dependency time - submitted time (hours);",400,0,40);

  // 3. - 2.
  TH1F *hpending_dependency = new TH1F("hpending_dependency",";pending time - dependency time (hours);",400,0,40);

  // 4. - 3.
  TH1F *hstagingIn_pending = new TH1F("hstagingIn_pending",";stagingIn time - pending time (hours);",400,0,40);

  // 5. - 4.
  TH1F *hactive_stagingIn = new TH1F("hactive_stagingIn",";active time - stagingIn time (hours);",400,0,40);

  // 6. - 5.
  TH1F *hstagingOut_active = new TH1F("hstagingOut_active",";stagingOut time - active time (hours);",400,0,40);

  // 7. - 6.
  TH1F *hcomplete_stagingOut = new TH1F("hcomplete_stagingOut",";complete time - stagingOut time (hours);",400,0,40);

  // These will be used as stacked histograms for each stage
  // 1. dependency
  TH1F *hdependency_submitted_forStack = new TH1F("hdependency_submitted_forStack",";job # ; time (hrs)",NTOTAL,0,NTOTAL);
  TH1F *hdependency_submitted_forStack___sec = new TH1F("hdependency_submitted_forStack___sec",";job # ; time (sec)",NTOTAL,0,NTOTAL);
  // 2. pending
  TH1F *hpending_dependency_forStack   = new TH1F("hpending_dependency_forStack",";job # ; time (hrs)",NTOTAL,0,NTOTAL);
  TH1F *hpending_dependency_forStack___sec   = new TH1F("hpending_dependency_forStack___sec",";job # ; time (sec)",NTOTAL,0,NTOTAL);
  // 3. stagingIn
  TH1F *hstagingIn_pending_forStack    = new TH1F("hstagingIn_pending_forStack",";job # ; time (hrs)",NTOTAL,0,NTOTAL);
  TH1F *hstagingIn_pending_forStack___sec    = new TH1F("hstagingIn_pending_forStack___sec",";job # ; time (sec)",NTOTAL,0,NTOTAL);
  // 4. active
  TH1F *hactive_stagingIn_forStack     = new TH1F("hactive_stagingIn_forStack",";job # ; time (hrs)",NTOTAL,0,NTOTAL);
  TH1F *hactive_stagingIn_forStack___sec     = new TH1F("hactive_stagingIn_forStack___sec",";job # ; time (sec)",NTOTAL,0,NTOTAL);
  // 5. stagingOut
  TH1F *hstagingOut_active_forStack    = new TH1F("hstagingOut_active_forStack",";job # ; time (hrs)",NTOTAL,0,NTOTAL);
  TH1F *hstagingOut_active_forStack___sec    = new TH1F("hstagingOut_active_forStack___sec",";job # ; time (sec)",NTOTAL,0,NTOTAL);

  TH1F *hstagingOut_active_noError = new TH1F("hstagingOut_active_noError",";job # ;active time (hours);",NTOTAL,0,NTOTAL);
  TH1F *hstagingOut_active_timedOut = new TH1F("hstagingOut_active_timedOut",";job # ;active time (hours);",NTOTAL,0,NTOTAL);
  TH1F *hstagingOut_active_resourceLimit = new TH1F("hstagingOut_active_resourceLimit",";job # ;active time (hours);",NTOTAL,0,NTOTAL);

  // 6. complete
  TH1F *hcomplete_stagingOut_forStack  = new TH1F("hcomplete_stagingOut_forStack",";job # ; time (hrs)",NTOTAL,0,NTOTAL);
  TH1F *hcomplete_stagingOut_forStack___sec  = new TH1F("hcomplete_stagingOut_forStack___sec",";job # ; time (sec)",NTOTAL,0,NTOTAL);

  // Total hours to look at since launch
  const Int_t NHOURS = 80;
  TH1F *hsubmitTimeSinceInit     = new TH1F("hsubmitTimeSinceInit",";submit time since t_{0};",NHOURS * 10,0,NHOURS);
  TH1F *hdependencyTimeSinceInit = new TH1F("hdependencyTimeSinceInit",";dependency time since t_{0};",NHOURS * 10,0,NHOURS);
  TH1F *hpendingTimeSinceInit    = new TH1F("hpendingTimeSinceInit",";pending time since t_{0};",NHOURS * 10,0,NHOURS);
  TH1F *hstagingInTimeSinceInit  = new TH1F("hstagingInTimeSinceInit",";staging in time since t_{0};",NHOURS * 10,0,NHOURS);
  TH1F *hactiveTimeSinceInit     = new TH1F("hactiveTimeSinceInit",";active time since t_{0};",NHOURS * 10,0,NHOURS);
  TH1F *hstagingOutTimeSinceInit = new TH1F("hstagingOutTimeSinceInit",";staging out time since t_{0};",NHOURS * 10,0,NHOURS);
  TH1F *hcompleteTimeSinceInit   = new TH1F("hcompleteTimeSinceInit",";complete time since t_{0};",NHOURS * 10,0,NHOURS);

  Long64_t INITIAL_SUBMIT;
  Long64_t submitTimeSinceInit, dependencyTimeSinceInit, pendingTimeSinceInit,
    stagingInTimeSinceInit, activeTimeSinceInit, stagingOutTimeSinceInit, completeTimeSinceInit;
  
  // Times we check how many jobs were running.
  // Check every 10 min, over 60 hours.
  const Int_t NCHECKTIMES = 360;
  Long64_t CHECKTIMES[NCHECKTIMES];
  Int_t TIMESINCEINITFORCHECKTIMES[NCHECKTIMES];
  Int_t jobsRunningAtCheckTimes[NCHECKTIMES];

  TGraph *gcput_walltime = new TGraph(NTOTAL);
  gcput_walltime->SetName("gcput_walltime");

  TGraph *gwalltime_activeTime = new TGraph(NTOTAL);
  gwalltime_activeTime->SetName("gwalltime_activeTime");
  TH1F *hdiff_walltime_activeTime = new TH1F("hdiff_walltime_activeTime",";diff of wall time - active time (sec);",300,-600,600);

  // histograms for walltime, timeCopy, timePlugin
  TH1F *hwalltime = new TH1F("hwalltime",";wall time (hrs);",300,0,25); // 25 hours/300 bins = 5 min. bins
  TH1F *htimeCopy = new TH1F("htimeCopy",";copy time (min);",120,0,120); // 120 min/120 bins = 1 min. bins
  TH1F *htimePlugin = new TH1F("htimePlugin",";plugin time (hrs);",300,0,25); // 25 hours/300 bins = 5 min. bins

  TGraph *gcput_timePlugin = new TGraph(NTOTAL);
  gcput_timePlugin->SetName("gcput_timePlugin");

  // processing time / event
  TH1F *htimePlugin_per_event = new TH1F("htimePlugin_per_event",";time to process / event (sec);",5000,0,10);

  Double_t average_timePlugin_per_event = 0;
  Int_t    n_average_timePlugin_per_event = 0;
  Double_t average_less0_1_timePlugin_per_event = 0;
  Int_t    n_average_less0_1_timePlugin_per_event = 0;

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
    if(nTotal % 1000 == 0) cout << "processed " << setw(5) << nTotal << " / 7300" << endl;

    if(nTotal==1){
      INITIAL_SUBMIT = utimeSubmitted;

      // Set up times to check how many jobs were running
      for(Int_t i=0;i<NCHECKTIMES;i++){
	CHECKTIMES[i] = INITIAL_SUBMIT + 600 * i;
	TIMESINCEINITFORCHECKTIMES[i] = 600 * i;
      }
    }

    // How many jobs were running at CHECKTIME[i]
    for(Int_t i=0;i<NCHECKTIMES;i++){
      if(utimeActive < CHECKTIMES[i] && CHECKTIMES[i] < utimeStagingOut)
	jobsRunningAtCheckTimes[i]++;
    }

    submitTimeSinceInit     = utimeSubmitted   - INITIAL_SUBMIT;
    dependencyTimeSinceInit = utimeDependency  - INITIAL_SUBMIT;
    pendingTimeSinceInit    = utimePending     - INITIAL_SUBMIT;
    stagingInTimeSinceInit  = utimeStagingIn   - INITIAL_SUBMIT;
    activeTimeSinceInit     = utimeActive      - INITIAL_SUBMIT;
    stagingOutTimeSinceInit = utimeStagingOut  - INITIAL_SUBMIT;
    completeTimeSinceInit   = utimeComplete    - INITIAL_SUBMIT;
    // cout << submitTimeSinceInit << "   " << completeTimeSinceInit << endl;

    hsubmitTimeSinceInit->Fill(submitTimeSinceInit / 3600.);
    hdependencyTimeSinceInit->Fill(dependencyTimeSinceInit / 3600.);
    hpendingTimeSinceInit->Fill(pendingTimeSinceInit / 3600.);
    hstagingInTimeSinceInit->Fill(stagingInTimeSinceInit / 3600.);
    hactiveTimeSinceInit->Fill(activeTimeSinceInit / 3600.);
    hstagingOutTimeSinceInit->Fill(stagingOutTimeSinceInit / 3600.);
    hcompleteTimeSinceInit->Fill(completeTimeSinceInit / 3600.);

    hmem->Fill(mem_int / 1024.);
    hvmem->Fill(vmem_int / 1024.);

    // total time
    hcomplete_submitted->Fill((utimeComplete - utimeSubmitted)/3600.);

    hdependency_submitted->Fill((utimeDependency - utimeSubmitted)/3600.);
    hpending_dependency->Fill((utimePending - utimeDependency)/3600.);
    hstagingIn_pending->Fill((utimeStagingIn - utimePending)/3600.);
    hactive_stagingIn->Fill((utimeActive - utimeStagingIn)/3600.);
    hstagingOut_active->Fill((utimeStagingOut - utimeActive)/3600.);
    hcomplete_stagingOut->Fill((utimeComplete - utimeStagingOut)/3600.);

    hdependency_submitted_forStack->SetBinContent(nTotal,(utimeDependency - utimeSubmitted)/3600.);
    hpending_dependency_forStack->SetBinContent(nTotal,(utimePending - utimeDependency)/3600.);
    hstagingIn_pending_forStack->SetBinContent(nTotal,(utimeStagingIn - utimePending)/3600.);
    hactive_stagingIn_forStack->SetBinContent(nTotal,(utimeActive - utimeStagingIn)/3600.);
    hstagingOut_active_forStack->SetBinContent(nTotal,(utimeStagingOut - utimeActive)/3600.);
    hcomplete_stagingOut_forStack->SetBinContent(nTotal,(utimeComplete - utimeStagingOut)/3600.);

    // in sec
    hdependency_submitted_forStack___sec->SetBinContent(nTotal,utimeDependency - utimeSubmitted);
    hpending_dependency_forStack___sec->SetBinContent(nTotal,utimePending - utimeDependency);
    hstagingIn_pending_forStack___sec->SetBinContent(nTotal,utimeStagingIn - utimePending);
    hactive_stagingIn_forStack___sec->SetBinContent(nTotal,utimeActive - utimeStagingIn);
    hstagingOut_active_forStack___sec->SetBinContent(nTotal,utimeStagingOut - utimeActive);
    hcomplete_stagingOut_forStack___sec->SetBinContent(nTotal,utimeComplete - utimeStagingOut);

    if(errorCode==0){
      hstagingOut_active_noError->SetBinContent(nTotal,(utimeStagingOut - utimeActive)/3600.);
    }else if(errorCode==1){
      hstagingOut_active_timedOut->SetBinContent(nTotal,(utimeStagingOut - utimeActive)/3600.);
    }else if(errorCode==2){
      hstagingOut_active_resourceLimit->SetBinContent(nTotal,(utimeStagingOut - utimeActive)/3600.);
    }

    // get hostnameNum
    hostnameNum = atoi(hostname.substr(4).c_str());
    // cout << hostname.substr(4).c_str() << " " << hostnameNum << endl;
    hhostnameNum->Fill(hostnameNum);
    ghostnameNum_jobNum->SetPoint(nTotal-1,nTotal-1,hostnameNum);
    if(hostnameNum < 140000)
      cout << nTotal-1 << " " << nTotal << " " << hostnameNum << endl;

    // convert walltime, cput into seconds
    Int_t hour, min, sec;
    walltime_sec = convert_to_sec(walltime);
    cput_sec = convert_to_sec(cput);

    gcput_walltime->SetPoint(nTotal-1,walltime_sec/3600.,cput_sec/3600.);

    // compare wall time against time spent in active
    gwalltime_activeTime->SetPoint(nTotal-1,(utimeStagingOut - utimeActive)/3600.,walltime_sec/3600.);
    hdiff_walltime_activeTime->Fill((utimeStagingOut - utimeActive) - walltime_sec);

    // histograms for wall time, copy time, plugin time
    hwalltime->Fill(walltime_sec / 3600.);
    htimeCopy->Fill(timeCopy / 60.);
    htimePlugin->Fill(timePlugin / 3600.);

    // compare CPU time with plugin time, not walltime
    gcput_timePlugin->SetPoint(nTotal-1,timePlugin/3600.,cput_sec/3600.);

    // processing time / event
    // set to -1 if # of events is 0 or -999 (nothing reported)
    Double_t timePlugin_per_event = nevents == 0 ? -1 : 1. * timePlugin / nevents;
    if(nevents == -999) timePlugin_per_event = -1;
    htimePlugin_per_event->Fill(timePlugin_per_event);
    // cout << timePlugin_per_event << endl;

    if(timePlugin_per_event > 0){
      average_timePlugin_per_event += timePlugin_per_event;
      n_average_timePlugin_per_event++;
    }

    if(timePlugin_per_event > 0 && timePlugin_per_event < 0.1){
      average_less0_1_timePlugin_per_event += timePlugin_per_event;
      n_average_less0_1_timePlugin_per_event++;
    }

    // some jobs have cput > 20 hours but plugin time less than 2 hours
    if(cput_sec/3600. > 20. && timePlugin/3600. < 2.){
      cout << "jobId = " << jobId << ", run = " << run << ", file = " << file << ", cput = " << cput_sec/3600. << " hours, plugin time = " << timePlugin/3600. << " hours, copy time = "
	   << timeCopy/3600. << " hours, wall time = " << walltime_sec/3600. << " hours, hostname = " << hostname << endl;
    }

  } // end of reading in each entry

  // calculate average time to process event
  average_timePlugin_per_event /= n_average_timePlugin_per_event;
  average_less0_1_timePlugin_per_event /= n_average_less0_1_timePlugin_per_event;
  
  // Create a stacked of histograms for each stage
  THStack *hs = new THStack("hs","");
  hdependency_submitted_forStack->SetFillColor(colors[0]);
  hdependency_submitted_forStack->SetLineColor(colors[0]);
  hs->Add(hdependency_submitted_forStack);
  hpending_dependency_forStack->SetFillColor(colors[1]);
  hpending_dependency_forStack->SetLineColor(colors[1]);
  hs->Add(hpending_dependency_forStack);
  hstagingIn_pending_forStack->SetFillColor(colors[2]);
  hstagingIn_pending_forStack->SetLineColor(colors[2]);
  hs->Add(hstagingIn_pending_forStack);
  hactive_stagingIn_forStack->SetFillColor(colors[3]);
  hactive_stagingIn_forStack->SetLineColor(colors[3]);
  hs->Add(hactive_stagingIn_forStack);
  hstagingOut_active_forStack->SetFillColor(colors[4]);
  hstagingOut_active_forStack->SetLineColor(colors[4]);
  hs->Add(hstagingOut_active_forStack);
  hcomplete_stagingOut_forStack->SetFillColor(colors[5]);
  hcomplete_stagingOut_forStack->SetLineColor(colors[5]);
  hs->Add(hcomplete_stagingOut_forStack);

  hs->Write();

  cout << "--------------------------------------------------" << endl;
  cout << "total                            : " << nTotal << endl;

  // fill accumulated hists for
  // hsubmitTimeSinceInit to hcompleteTimeSinceInit
  TH1F *haccumulate_submitTimeSinceInit = (TH1F*)hsubmitTimeSinceInit->Clone("haccumulate_submitTimeSinceInit");
  haccumulate_submitTimeSinceInit->Clear();
  for(Int_t i=0;i<=haccumulate_submitTimeSinceInit->GetNbinsX()+1;i++){
    Int_t total = 0;
    for(Int_t bin=0;bin<=i;bin++){
      total += hsubmitTimeSinceInit->GetBinContent(bin);
    }
    haccumulate_submitTimeSinceInit->SetBinContent(i,total);
  }

  TH1F *haccumulate_dependencyTimeSinceInit = (TH1F*)hdependencyTimeSinceInit->Clone("haccumulate_dependencyTimeSinceInit");
  for(Int_t i=0;i<=haccumulate_dependencyTimeSinceInit->GetNbinsX()+1;i++){
    Int_t total = 0;
    for(Int_t bin=0;bin<=i;bin++){
      total += hdependencyTimeSinceInit->GetBinContent(bin);
    }
    haccumulate_dependencyTimeSinceInit->SetBinContent(i,total);
  }

  TH1F *haccumulate_pendingTimeSinceInit = (TH1F*)hpendingTimeSinceInit->Clone("haccumulate_pendingTimeSinceInit");
  for(Int_t i=0;i<=haccumulate_pendingTimeSinceInit->GetNbinsX()+1;i++){
    Int_t total = 0;
    for(Int_t bin=0;bin<=i;bin++){
      total += hpendingTimeSinceInit->GetBinContent(bin);
    }
    haccumulate_pendingTimeSinceInit->SetBinContent(i,total);
  }

  TH1F *haccumulate_stagingInTimeSinceInit = (TH1F*)hstagingInTimeSinceInit->Clone("haccumulate_stagingInTimeSinceInit");
  for(Int_t i=0;i<=haccumulate_stagingInTimeSinceInit->GetNbinsX()+1;i++){
    Int_t total = 0;
    for(Int_t bin=0;bin<=i;bin++){
      total += hstagingInTimeSinceInit->GetBinContent(bin);
    }
    haccumulate_stagingInTimeSinceInit->SetBinContent(i,total);
  }

  TH1F *haccumulate_activeTimeSinceInit = (TH1F*)hactiveTimeSinceInit->Clone("haccumulate_activeTimeSinceInit");
  for(Int_t i=0;i<=haccumulate_activeTimeSinceInit->GetNbinsX()+1;i++){
    Int_t total = 0;
    for(Int_t bin=0;bin<=i;bin++){
      total += hactiveTimeSinceInit->GetBinContent(bin);
    }
    haccumulate_activeTimeSinceInit->SetBinContent(i,total);
  }

  TH1F *haccumulate_stagingOutTimeSinceInit = (TH1F*)hstagingOutTimeSinceInit->Clone("haccumulate_stagingOutTimeSinceInit");
  for(Int_t i=0;i<=haccumulate_stagingOutTimeSinceInit->GetNbinsX()+1;i++){
    Int_t total = 0;
    for(Int_t bin=0;bin<=i;bin++){
      total += hstagingOutTimeSinceInit->GetBinContent(bin);
    }
    haccumulate_stagingOutTimeSinceInit->SetBinContent(i,total);
  }

  TH1F *haccumulate_completeTimeSinceInit = (TH1F*)hcompleteTimeSinceInit->Clone("haccumulate_completeTimeSinceInit");
  for(Int_t i=0;i<=haccumulate_completeTimeSinceInit->GetNbinsX()+1;i++){
    Int_t total = 0;
    for(Int_t bin=0;bin<=i;bin++){
      total += hcompleteTimeSinceInit->GetBinContent(bin);
    }
    haccumulate_completeTimeSinceInit->SetBinContent(i,total);
  }

  TGraph *gjobsRunningAtCheckTimes = new TGraph(NCHECKTIMES);
  gjobsRunningAtCheckTimes->SetName("gjobsRunningAtCheckTimes");
  for(Int_t i=0;i<NCHECKTIMES;i++){
    gjobsRunningAtCheckTimes->SetPoint(i,TIMESINCEINITFORCHECKTIMES[i] / 3600.,jobsRunningAtCheckTimes[i]);
  }
  
  // Write output ROOT file
  gcput_walltime->Write();
  ghostnameNum_jobNum->Write();
  gwalltime_activeTime->Write();
  gjobsRunningAtCheckTimes->Write();
  gcput_timePlugin->Write();
  outfile->Write();

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
  hs->Draw();
  hs->GetXaxis()->SetTitle("job #");
  hs->GetYaxis()->SetTitle("time (hrs)");
  hs->Draw();
  TLegend *legend = new TLegend(0.15,0.80,0.90,0.95);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->SetNColumns(3);
  legend->SetTextSize(0.045);
  legend->AddEntry(hdependency_submitted_forStack,"submit","F");
  legend->AddEntry(hpending_dependency_forStack,"dependency","F");
  legend->AddEntry(hstagingIn_pending_forStack,"pending","F");
  legend->AddEntry(hactive_stagingIn_forStack,"stagingIn","F");
  legend->AddEntry(hstagingOut_active_forStack,"active","F");
  legend->AddEntry(hcomplete_stagingOut_forStack,"stagingOut","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______timeline.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______timeline.png",plots);
  canvas->SaveAs(hname);
  plots++;

  legend->SetX1NDC(0.70);
  legend->SetX2NDC(0.90);
  legend->SetY1NDC(0.90);
  legend->SetY2NDC(0.95);

  // individual
  hdependency_submitted_forStack->Draw();
  legend->Clear();
  legend->AddEntry(hdependency_submitted_forStack,"submit","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______submit.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______submit.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hpending_dependency_forStack->Draw();
  legend->Clear();
  legend->AddEntry(hpending_dependency_forStack,"dependency","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______dependency.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______dependency.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hstagingIn_pending_forStack->Draw();
  legend->Clear();
  legend->AddEntry(hstagingIn_pending_forStack,"pending","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______pending.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______pending.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hactive_stagingIn_forStack->Draw();
  legend->Clear();
  legend->AddEntry(hactive_stagingIn_forStack,"stagingIn","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______stagingIn.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______stagingIn.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hstagingOut_active_forStack->Draw();
  legend->Clear();
  legend->AddEntry(hstagingOut_active_forStack,"active","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______active.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______active.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hcomplete_stagingOut_forStack->Draw();
  legend->Clear();
  legend->AddEntry(hcomplete_stagingOut_forStack,"stagingOut","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______stagingOut.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______stagingOut.png",plots);
  canvas->SaveAs(hname);
  plots++;


  // individual (in sec)

  hdependency_submitted_forStack___sec->SetFillColor(colors[0]);
  hdependency_submitted_forStack___sec->SetLineColor(colors[0]);
  hpending_dependency_forStack___sec->SetFillColor(colors[1]);
  hpending_dependency_forStack___sec->SetLineColor(colors[1]);
  hstagingIn_pending_forStack___sec->SetFillColor(colors[2]);
  hstagingIn_pending_forStack___sec->SetLineColor(colors[2]);
  hactive_stagingIn_forStack___sec->SetFillColor(colors[3]);
  hactive_stagingIn_forStack___sec->SetLineColor(colors[3]);
  hstagingOut_active_forStack___sec->SetFillColor(colors[4]);
  hstagingOut_active_forStack___sec->SetLineColor(colors[4]);
  hcomplete_stagingOut_forStack___sec->SetFillColor(colors[5]);
  hcomplete_stagingOut_forStack___sec->SetLineColor(colors[5]);

  hdependency_submitted_forStack___sec->Draw();
  legend->Clear();
  legend->AddEntry(hdependency_submitted_forStack,"submit","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______sec.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______sec.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hpending_dependency_forStack___sec->Draw();
  legend->Clear();
  legend->AddEntry(hpending_dependency_forStack,"dependency","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______sec.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______sec.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hstagingIn_pending_forStack___sec->Draw();
  legend->Clear();
  legend->AddEntry(hstagingIn_pending_forStack,"pending","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______sec.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______sec.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hactive_stagingIn_forStack___sec->Draw();
  legend->Clear();
  legend->AddEntry(hactive_stagingIn_forStack,"stagingIn","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______sec.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______sec.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hstagingOut_active_forStack___sec->Draw();
  legend->Clear();
  legend->AddEntry(hstagingOut_active_forStack,"active","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______sec.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______sec.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hcomplete_stagingOut_forStack___sec->Draw();
  legend->Clear();
  legend->AddEntry(hcomplete_stagingOut_forStack,"stagingOut","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______sec.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______sec.png",plots);
  canvas->SaveAs(hname);
  plots++;

  // plot active time against job #
  legend->Clear();
  hstagingOut_active_noError->SetLineColor(kBlue);
  hstagingOut_active_noError->SetFillColor(kBlue);
  // hstagingOut_active_noError->SetMaximum(hstagingOut_active_timedOut->GetMaximum() * 1.05);
  hstagingOut_active_noError->Draw();
  legend->AddEntry(hstagingOut_active_noError,"no error","F");

  hstagingOut_active_timedOut->SetLineColor(kBlue-10);
  hstagingOut_active_timedOut->SetFillColor(kBlue-10);
  hstagingOut_active_timedOut->Draw("same");
  legend->AddEntry(hstagingOut_active_timedOut,"timed out","F");

  // plot active time against error
  legend->SetX1NDC(0.15);
  legend->SetX2NDC(0.90);

  hstagingOut_active_resourceLimit->SetLineColor(kCyan);
  hstagingOut_active_resourceLimit->SetFillColor(kCyan);
  hstagingOut_active_resourceLimit->Draw("same");
  legend->AddEntry(hstagingOut_active_resourceLimit,"resource limit","F");
  legend->Draw("same");
  sprintf(hname,"figures/%3.3d______activeByStatus.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______activeByStatus.png",plots);
  canvas->SaveAs(hname);
  plots++;

  // memory usage
  hvmem->Draw();
  sprintf(hname,"figures/%3.3d______vmem.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______vmem.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hmem->Draw();
  sprintf(hname,"figures/%3.3d______mem.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______mem.png",plots);
  canvas->SaveAs(hname);
  plots++;

  // time since submission
  legend->SetX1NDC(0.25);
  legend->SetX2NDC(0.95);
  legend->SetY1NDC(0.15);
  legend->SetY2NDC(0.35);
  legend->Clear();
  haccumulate_submitTimeSinceInit->SetMarkerStyle(20);
  haccumulate_submitTimeSinceInit->SetMarkerSize(1.2);
  haccumulate_submitTimeSinceInit->SetMarkerColor(colors[0]);
  haccumulate_submitTimeSinceInit->GetXaxis()->SetTitle("time since start (hrs)");
  haccumulate_submitTimeSinceInit->GetYaxis()->SetTitle("# jobs arriving at stage");
  haccumulate_submitTimeSinceInit->Draw("P");
  legend->AddEntry(haccumulate_submitTimeSinceInit,"submit","P");

  haccumulate_dependencyTimeSinceInit->SetMarkerStyle(20);
  haccumulate_dependencyTimeSinceInit->SetMarkerSize(0.6);
  haccumulate_dependencyTimeSinceInit->SetMarkerColor(colors[1]);
  haccumulate_dependencyTimeSinceInit->Draw("Psame");
  legend->AddEntry(haccumulate_dependencyTimeSinceInit,"dependency","P");

  haccumulate_pendingTimeSinceInit->SetMarkerStyle(20);
  haccumulate_pendingTimeSinceInit->SetMarkerSize(0.8);
  haccumulate_pendingTimeSinceInit->SetMarkerColor(colors[2]);
  haccumulate_pendingTimeSinceInit->Draw("Psame");
  legend->AddEntry(haccumulate_pendingTimeSinceInit,"pending","P");

  haccumulate_stagingInTimeSinceInit->SetMarkerStyle(20);
  haccumulate_stagingInTimeSinceInit->SetMarkerSize(1.2);
  haccumulate_stagingInTimeSinceInit->SetMarkerColor(colors[3]);
  haccumulate_stagingInTimeSinceInit->Draw("Psame");
  legend->AddEntry(haccumulate_stagingInTimeSinceInit,"staging in","P");

  haccumulate_activeTimeSinceInit->SetMarkerStyle(20);
  haccumulate_activeTimeSinceInit->SetMarkerSize(0.6);
  haccumulate_activeTimeSinceInit->SetMarkerColor(colors[4]);
  haccumulate_activeTimeSinceInit->Draw("Psame");
  legend->AddEntry(haccumulate_activeTimeSinceInit,"active","P");

  haccumulate_stagingOutTimeSinceInit->SetMarkerStyle(20);
  haccumulate_stagingOutTimeSinceInit->SetMarkerSize(1.2);
  haccumulate_stagingOutTimeSinceInit->SetMarkerColor(colors[5]);
  haccumulate_stagingOutTimeSinceInit->Draw("Psame");
  legend->AddEntry(haccumulate_stagingOutTimeSinceInit,"staging out","P");

  haccumulate_completeTimeSinceInit->SetMarkerStyle(20);
  haccumulate_completeTimeSinceInit->SetMarkerSize(0.6);
  haccumulate_completeTimeSinceInit->SetMarkerColor(colors[6]);
  haccumulate_completeTimeSinceInit->Draw("Psame");
  legend->AddEntry(haccumulate_completeTimeSinceInit,"complete","P");

  legend->Draw("same");

  sprintf(hname,"figures/%3.3d______accumulate_time.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______accumulate_time.png",plots);
  canvas->SaveAs(hname);
  plots++;

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

  // wall time, copy time, plugin time
  legend->Clear();
  hwalltime->SetFillStyle(1001);
  hwalltime->SetFillColor(kRed);
  hwalltime->SetLineColor(kRed);
  if(htimePlugin->GetMaximum() > hwalltime->GetMaximum()) hwalltime->SetMaximum(htimePlugin->GetMaximum() * 1.05);
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

  htimeCopy->SetFillStyle(0);
  htimeCopy->SetFillColor(kBlue);
  htimeCopy->SetLineColor(kBlue);
  htimeCopy->Draw();
  sprintf(hname,"figures/%3.3d______timeCopy.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______timeCopy.png",plots);
  canvas->SaveAs(hname);
  plots++;
  
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

  gcput_walltime->GetXaxis()->SetLimits(0,25);
  gcput_walltime->Draw("AP");

  for(Int_t i=0;i<6;i++){
    flinear[i]->Draw("same");
  }

  sprintf(hname,"figures/%3.3d______cput_walltime.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______cput_walltime.png",plots);
  canvas->SaveAs(hname);
  plots++;

  gcput_walltime->GetXaxis()->SetLimits(0,8);
  gcput_walltime->SetMaximum(32);
  gcput_walltime->Draw("AP");

  for(Int_t i=0;i<6;i++){
    flinear[i]->Draw("same");
  }

  sprintf(hname,"figures/%3.3d______cput_walltime_close.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______cput_walltime_close.png",plots);
  canvas->SaveAs(hname);
  plots++;

  // cpu time vs timePlugin
  gcput_timePlugin->SetMarkerStyle(20);
  gcput_timePlugin->SetMarkerSize(0.5);
  gcput_timePlugin->SetMarkerColor(kBlue);
  gcput_timePlugin->SetTitle("");
  gcput_timePlugin->GetXaxis()->SetTitle("plugin time (hrs)");
  gcput_timePlugin->GetYaxis()->SetTitle("cpu time (hrs)");

  gcput_timePlugin->GetXaxis()->SetLimits(0,25);
  gcput_timePlugin->Draw("AP");

  for(Int_t i=0;i<6;i++){
    flinear[i]->Draw("same");
  }

  sprintf(hname,"figures/%3.3d______cput_timePlugin.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______cput_timePlugin.png",plots);
  canvas->SaveAs(hname);
  plots++;

  gcput_timePlugin->GetXaxis()->SetLimits(0,8);
  gcput_timePlugin->SetMaximum(32);
  gcput_timePlugin->Draw("AP");

  for(Int_t i=0;i<6;i++){
    flinear[i]->Draw("same");
  }

  sprintf(hname,"figures/%3.3d______cput_timePlugin_close.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______cput_timePlugin_close.png",plots);
  canvas->SaveAs(hname);
  plots++;

  canvas->SetLogx(1);
  htimePlugin_per_event->Draw();

  sprintf(text,"average (<0.1 s): %5.3f s",average_less0_1_timePlugin_per_event);
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

  sprintf(text,"average: %5.3f s",average_timePlugin_per_event);
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

  // jobs running at a given time
  gjobsRunningAtCheckTimes->SetMarkerStyle(20);
  gjobsRunningAtCheckTimes->SetMarkerSize(0.5);
  gjobsRunningAtCheckTimes->SetMarkerColor(kBlue);
  gjobsRunningAtCheckTimes->SetTitle("");
  gjobsRunningAtCheckTimes->GetXaxis()->SetTitle("time since launch (hrs)");
  gjobsRunningAtCheckTimes->GetYaxis()->SetTitle("# active jobs");

  gjobsRunningAtCheckTimes->Draw("AP");

  sprintf(hname,"figures/%3.3d______jobsRunningAtCheckTimes.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______jobsRunningAtCheckTimes.png",plots);
  canvas->SaveAs(hname);
  plots++;

  hhostnameNum->SetTitle("");
  hhostnameNum->GetXaxis()->SetTitle("hostname");
  hhostnameNum->GetYaxis()->SetTitle("");
  hhostnameNum->GetXaxis()->SetNoExponent(kTRUE);
  hhostnameNum->Draw("");

  sprintf(hname,"figures/%3.3d______hostnameNum.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______hostnameNum.png",plots);
  canvas->SaveAs(hname);
  plots++;

  ghostnameNum_jobNum->SetTitle("");
  ghostnameNum_jobNum->GetXaxis()->SetTitle("job #");
  ghostnameNum_jobNum->GetYaxis()->SetTitle("hostname");
  ghostnameNum_jobNum->SetMarkerColor(kBlue);
  ghostnameNum_jobNum->SetMarkerSize(0.8);
  ghostnameNum_jobNum->SetMarkerStyle(20);
  ghostnameNum_jobNum->SetLineColor(kBlue);
  ghostnameNum_jobNum->SetLineWidth(1);
  ghostnameNum_jobNum->SetLineStyle(20);
  ghostnameNum_jobNum->GetYaxis()->SetNoExponent(kTRUE);
  ghostnameNum_jobNum->SetMinimum(140000);
  ghostnameNum_jobNum->SetMaximum(140250);
  ghostnameNum_jobNum->Draw("AP");

  sprintf(hname,"figures/%3.3d______hostnameNum_jobNum.pdf",plots);
  canvas->SaveAs(hname);
  sprintf(hname,"figures/%3.3d______hostnameNum_jobNum.png",plots);
  canvas->SaveAs(hname);
  plots++;
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
