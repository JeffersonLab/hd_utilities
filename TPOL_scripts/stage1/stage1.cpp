#include "TTree.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
using namespace std; 
#include "stage1.h"

void printUsage();

int main(int argc, char **argv){
   
  char  hId[80];
  char  inFileName[150];
  char  outFileName[150];
  char *argptr;
  
  //Set the default output file
  sprintf(outFileName,"./tpolOutFile.root");
  int runNumber = -99;
  int treeType = -99;
  int nToGo = -1;
  if (argc == 1) printUsage();
  for (int i=1; i<argc; i++) {
    argptr = argv[i];
    if (*argptr == '-') {
      argptr++;
      switch (*argptr) {
      case 'h':
        printUsage();
        break;
      case 'o':
        strcpy(outFileName,++argptr);
        break;
      case 'r':
        runNumber = atoi(++argptr);
        break;
      case 't':
        treeType = atoi(++argptr);
        break;
      case 'n':
        nToGo = atoi(++argptr);
        break;
      default:
        fprintf(stderr,"\nUnrecognized argument: [-%s]\n",argptr);
        printUsage();
        break;
      }
    } else {
      strcpy(inFileName,argptr);
    }
  }

  std::cout<<"runNumber = "<<runNumber<<std::endl;

  double tDiffCut         =   2.0;
  double tDiffCutOutLow   = -14.0;
  double tDiffCutOutHigh  =  -6.0;
  double tDiffCutOutLow2  =   6.0;
  double tDiffCutOutHigh2 =  14.0;

  //ENERGY MINIMUM CUT
  double eFloor = 0.0;   //Minimum energy deposition in keV

  //SETUP THE INPUT FILE
  TFile inFile(inFileName);

  //SETUP THE INPUT TREE, STRUCTURES AND GET BRANCHES
  TTree *inTree = (TTree*)inFile.Get("TPOL_tree");

  tpolTmp2_t tpolTmp2;
  tpol0_t tpol0;
  if (treeType == 1) {
    getBranchesT02(inTree,&tpolTmp2);
    cout<<"getBranch type New2"<<endl;
  }else{
    getBranchesT0(inTree,&tpol0);
  }
  //DEFINE OUTPUT FILE
  TFile outFile(outFileName,"RECREATE"); 
  //DEFINE OUTPUT TREE
  TTree outTree("tpolTree1","tpolRecon");
  //DEFINE OUTPUT-TREE STRUCTURE AND SET BRANCHES
  tpolHitToGo_t tpolHitToGo;

  setBranchesHitToGo(&outTree,&tpolHitToGo);
  tpolHitToGo.nHits    = 0;
  tpolHitToGo.nHitsTot = 0;
  tpolHitToGo.ePair    = 0;
  tpolHitToGo.E_lhit   = 0;
  tpolHitToGo.E_rhit   = 0;
  tpolHitToGo.t_lhit   = 0;
  tpolHitToGo.t_rhit   = 0;

  //DEFINE SOME HISTOGRAMS
  TH1D *hTDiff      = new TH1D("hTDiff","",400,-20.0,20.0);
  //TH1D *hSlot13      = new TH1D("hSlot13","",16,-0.5,15.5);
  //TH1D *hSlot14      = new TH1D("hSlot14","",16,-0.5,15.5);
  //TH1D *hSlot15      = new TH1D("hSlot15","",16,-0.5,15.5);
  //TH1D *hSlot16      = new TH1D("hSlot16","",16,-0.5,15.5);
  TH2D *h2Hit = new TH2D("h2Hit","",32,0.5,32.5,32,0.5,32.5);

  TH2D *hEDiffVsESum = new TH2D("hEDiffVsESum","",100,2.0,12.0,100,-4.0,4.0);

  TH1D *hEDep = new TH1D("hEDep","",1400,100,1500);

  double adcLow = 0.0;
  double adcHigh = 400.0;
  int nADCch = 100;
  if (runNumber < 30274 || runNumber > 40000) { 
    adcLow = 0.0;
    adcHigh = 600.0;
    nADCch = 150;
  }
  //TH1D *hphiTmp = new TH1D("hphiTmp","",32,0.0,360);
  TH1D *hADCTmp = new TH1D("hADCTmp","",nADCch,adcLow,adcHigh);
  TH1D *hADC[100];    //KEEPING 100 WAVEFORMS
  //std::cout<<"test:7"<<std::endl;
  for (Int_t nTmp = 0; nTmp < 100; nTmp++) {
    sprintf(hId,"hADC%d",nTmp+1);
    hADC[nTmp] = new TH1D(hId,"",nADCch,adcLow,adcHigh);
  }
  //GET THE LTSPICE WAVEFORM
  TH1D *hLT = MakeSpiceHisto();
  hLT->SetName("hLT");

  //FIND TIME AND MAXIMUM VALUE OF LTSPICE WAVEFORM
  double tValSpice = signalTime(hLT,400);
  double maxValSpice = hLT->GetMaximum();

  //DEFINE THE FIT FUNCTION
  MySpiceFun2 * fptr3 = new MySpiceFun2();
  fptr3->SetSample(hLT);
  int nPar = 6;
  TF1 *spiceFun2 = new TF1("spiceFun2",fptr3,&MySpiceFun2::Evaluate,0,1,nPar,		   
			   "MySpiceFun2","Evaluate");
  //DEFINE tpolHit
  tpolHit_t tpolHit;
  tpolHit.eDep       = 0;
  tpolHit.time       = 0;
  tpolHit.sector     = 0;
  tpolHit.phi        = 0;
  tpolHit.E_lhit     = 0;
  tpolHit.E_rhit     = 0;
  tpolHit.t_lhit     = 0;
  tpolHit.t_rhit     = 0;
  tpolHit.w_samp1    = 0;
  tpolHit.w_integral = 0;
  tpolHit.w_min      = 0;
  tpolHit.w_max      = 0;
 
  int count100 = 0;  
  int tmpCount = 0;
  if (nToGo <= 0) nToGo = inTree->GetEntries();
  std::cout<<"inTree->GetEntries() = "<<inTree->GetEntries()<<std::endl;
  std::cout<<"nToGo = "<<nToGo<<std::endl;
  for(Int_t i=0;i<nToGo;i++){
    if (i%100000 == 0) {
      std::cout <<"Event = "<<i<<std::endl;
    }
    inTree->GetEntry(i);

    //CONVERT INPUT-TREE STRUCTURE IF REQUIRED
    if (treeType == 1) {
      tpolStructConvert2(&tpol0,&tpolTmp2);
    }
    //DEFINE SOME tpolHit VECTORS
    vector<tpolHit_t> tpolHitAll;
    vector<double> tagEVec;
    vector<double> tagEOutVec;

    //GRAB SOME PS INFORMATION FROM THE INPUT TREE
    double eLHitOrig = tpol0.E_lhit[0];
    double eRHitOrig = tpol0.E_rhit[0];
    double ePair  = eLHitOrig + eRHitOrig;
    double deltaE = eLHitOrig - eRHitOrig;
    double tDiffPair = tpol0.t_lhit[0] - tpol0.t_rhit[0];
    double tPair = (tpol0.t_lhit[0] + tpol0.t_rhit[0])/2.0;

    //CHECK IF PAIR ARE WELL TIMED
    bool timedPair = false;
    if (tDiffPair > -1.3 && tDiffPair < 1.3) timedPair = true; 
    int nTagMatch = 0;
    int nTagOutMatch = 0;

    //SORT TAGGER HITS IN TERMS OF IN-TIME AND OUT-OF-TIME HITS
    for (int itag=0; itag< (int)tpol0.ntag; itag++) {
      double tDiff     = tPair - tpol0.t_tag[itag];
      tmpCount++;
      double eDiffCut = 10.3;
      double eTagDiff = ePair-tpol0.E_tag[itag];

      hTDiff->Fill(tDiff);

      //CENTRAL IN-TIME PEAK
      if (fabs(tDiff) <= tDiffCut) {
	if (fabs(eTagDiff) < eDiffCut && timedPair == true){ 
	  nTagMatch++;
	  tagEVec.push_back(tpol0.E_tag[itag]);
	}
      }

      //OUT-OF-TIME PEAKS
      bool psTagOutOfTime = false;
      if (tDiff >= tDiffCutOutLow && tDiff <= tDiffCutOutHigh) psTagOutOfTime = true;
      if (tDiff >= tDiffCutOutLow2 && tDiff <= tDiffCutOutHigh2) psTagOutOfTime = true;
      if (psTagOutOfTime == true) {
	if (fabs(eTagDiff) < eDiffCut && timedPair == true){  
	  nTagOutMatch++;
	  tagEOutVec.push_back(tpol0.E_tag[itag]);
	}
      }
    }
    int samp1 = -100;
    int wSamp1 = -100;
    int nTimeMax = nADCch;

    //LOOP SECTORS IN EVENT
    for (UInt_t iCh =0; iCh < tpol0.nadc; iCh++) {
      //SET THE SECTOR AND PHI VALUES
      int sector = slotChannelToSector(tpol0.slot[iCh],tpol0.channel[iCh]);
      if (runNumber > 71900) {
	sector = slotChannelToSector4Slot(tpol0.slot[iCh],tpol0.channel[iCh]);
      }


      //if (runNumber > 100491){
	//sector = -1;
	////Reset channels for slot 13
	//int channelPrime = tpol0.channel[iCh];
	//if (tpol0.slot[iCh] == 13 && tpol0.channel[iCh] == 8) channelPrime = 0; 
	//if (tpol0.slot[iCh] == 13 && tpol0.channel[iCh] == 9) channelPrime = 1; 
	//if (tpol0.slot[iCh] == 13 && tpol0.channel[iCh] == 10) channelPrime = 2; 
	
	//if (tpol0.slot[iCh] == 13) sector = 25 - channelPrime;
	//if (tpol0.slot[iCh] == 14) sector = 17 - tpol0.channel[iCh];
	//if (tpol0.slot[iCh] == 15) sector = 9 -  tpol0.channel[iCh];
	//if (tpol0.slot[iCh] == 16){
	//if (tpol0.channel[iCh] == 0){
	//  sector = 1;
	//}else{
	//  sector = 33 - tpol0.channel[iCh];
	//}
	//}
	//int sectorOld = sector;
	////Cable swap
	//if (sectorOld == 22) sector = 25;
	//if (sectorOld == 25) sector = 22;

	//if (tpol0.slot[iCh] == 13) hSlot13->Fill(channelPrime); 
	//if (tpol0.slot[iCh] == 14) hSlot14->Fill(tpol0.channel[iCh]); 
	//if (tpol0.slot[iCh] == 15) hSlot15->Fill(tpol0.channel[iCh]); 
	//if (tpol0.slot[iCh] == 16) hSlot16->Fill(tpol0.channel[iCh]); 
	//} 

      double phiVal = sectorToPhi(sector);

      samp1 = tpol0.w_samp1[iCh];
      wSamp1 = samp1;
      for (int adcCh = 0; adcCh < nTimeMax; adcCh++){
	int adcChSet = adcCh;
	hADCTmp->SetBinContent(adcCh+1,1.0*tpol0.waveform[iCh][adcChSet] - 1.0*samp1);
      }
      
      //MAKE AMPLITUDE CUTS
      double diff = hADCTmp->GetMaximum() - hADCTmp->GetMinimum();
      //if (runNumber > 71900 && runNumber < 100491) {
      //if (tpol0.slot[iCh]==15 && tpol0.channel[iCh] >= 6 && tpol0.channel[iCh] <= 7) {
      //  diff = diff/2.0;
      //  wSamp1 = wSamp1/2.0;
      //}
      //}
      if (diff > 53 && wSamp1 <= 133) {
	//KEEP SIGNAL HISTOGRAMS FOR THE FIRST 99 EVENTS AND ALSO THE FINAL EVENT 
	if (count100 >= 100) count100 = 99;
	for (int adcCh = 0; adcCh < nTimeMax; adcCh++){
	  double adcVal = 1.0*tpol0.waveform[iCh][adcCh];
	  hADC[count100]->SetBinContent(adcCh+1,adcVal);
	  double varTest = 9.0 + fabs(adcVal - 100)*0.07;
	  hADC[count100]->SetBinError(adcCh+1,sqrt(varTest));//NOT ACCURATE AT ALL!
	}

	//SEED FIT PARAMETERS
	double maxValSignal = hADC[count100]->GetMaximum() -samp1;
	double constVal = samp1;
        double tValSignal = signalTime(hADC[count100],150);
        if (samp1 > 90.0) {
          tValSignal =  signalTimeSamp1(hADC[count100],150,samp1);
          maxValSignal=hADC[count100]->GetMaximum()-100.0;
          constVal = 100.0;
        }
        double tAdd = tValSpice - tValSignal;
        double fVal = maxValSignal/maxValSpice;
          
	spiceFun2->SetParameter(0,fVal);
	spiceFun2->SetParameter(1,tAdd/100.0);
	spiceFun2->SetParameter(2,constVal);
	spiceFun2->SetParameter(3,10.0);
	spiceFun2->SetParameter(4,250);
	spiceFun2->SetParLimits(4,150,400);

	//SET FIT RANGE
	double fitLow = 0.0;
	double fitHigh = 400.0;
	if (runNumber < 30274 || runNumber > 40000) { //Sp16 Set3
	  fitLow = 0.0;
	  fitHigh = 600.0;
	}

        //FIT THE SIGNAL
	hADC[count100]->Fit("spiceFun2","BRQ","",fitLow,fitHigh);
	double tSigTest = tValSpice - spiceFun2->GetParameter(1)*100;
	tValSignal = tSigTest;

	//SET THE TIME
	double tValFit = tValSignal + tCorrect(sector);

	//SET THE ENERGY DEPOSITION
	double eDepFit = 1125*(spiceFun2->GetParameter(0))*(2000.0/4096)*eCorrect(sector);
	//if (runNumber > 71900 && runNumber < 100491) {
	//if (tpol0.slot[iCh]==15 && tpol0.channel[iCh] >= 6 && tpol0.channel[iCh] <= 7) {
	//  eDepFit = (1/2.0)*1125*(spiceFun2->GetParameter(0))*(2000.0/4096)*eCorrect(sector);
	//}
	//}

	//**********************************************

	count100++;

	//FILL THE tpolHit STRUCTURE
	tpolHit.eDep       = eDepFit;
	tpolHit.time       = tValFit;
	tpolHit.sector     = sector;
	tpolHit.phi        = phiVal;
	tpolHit.ePair      = ePair;
	tpolHit.deltaE     = deltaE;
	tpolHit.t_lhit     = tpol0.t_lhit[0];
	tpolHit.t_rhit     = tpol0.t_rhit[0];
	tpolHit.w_samp1    = wSamp1;
	tpolHit.w_integral = tpol0.w_integral[iCh];
	tpolHit.w_max      = tpol0.w_max[iCh];
	tpolHit.w_min      = tpol0.w_min[iCh];
	tpolHit.eTag       = -1.0;
	tpolHit.eTagOut    = -1.0;
	tpolHit.tagEVec    = tagEVec;
	tpolHit.tagEOutVec = tagEOutVec;
	//LOAD tpolHitAll VECTOR
	if (eDepFit > eFloor) {
	  tpolHitAll.push_back(tpolHit);
	}
      }
    }
    //FILL THE tpolHitToGo STRUCTURE
    int nTotAll         = tpolHitAll.size();
    int togoIndex = 0;
    for (int j=0; j<nTotAll; j++) {
      tpolHitToGo.eDep[togoIndex]       = tpolHitAll[j].eDep;
      tpolHitToGo.time[togoIndex]       = tpolHitAll[j].time;
      tpolHitToGo.sector[togoIndex]     = tpolHitAll[j].sector;
      tpolHitToGo.w_samp1[togoIndex]    = wSamp1;
      tpolHitToGo.w_integral[togoIndex] = tpolHitAll[j].w_integral;
      tpolHitToGo.w_min[togoIndex]      = tpolHitAll[j].w_min;
      tpolHitToGo.w_max[togoIndex]      = tpolHitAll[j].w_max;
      tpolHitToGo.phi[togoIndex]        = tpolHitAll[j].phi;
      tpolHitToGo.ePair                 = tpolHitAll[j].ePair;
      tpolHitToGo.deltaE                = tpolHitAll[j].deltaE;
      tpolHitToGo.E_lhit                = tpolHitAll[j].E_lhit;
      tpolHitToGo.E_rhit                = tpolHitAll[j].E_rhit;
      tpolHitToGo.t_lhit                = tpolHitAll[j].t_lhit;
      tpolHitToGo.t_rhit                = tpolHitAll[j].t_rhit;
      togoIndex++;
    }
    if (nTotAll == 2) {
      h2Hit->Fill(1.0*tpolHitAll[0].sector,1.0*tpolHitAll[1].sector);
      h2Hit->Fill(1.0*tpolHitAll[1].sector,1.0*tpolHitAll[0].sector);
    }
    if (nTotAll == 1) {
      //Fill some histograms
      hEDiffVsESum->Fill(tpolHitAll[0].ePair,tpolHitAll[0].deltaE);
      hEDep->Fill(tpolHitAll[0].eDep);
    }
    //FILL IN THE TAGGER INFORMATION
    int togoIndexNGamIn = 0;
    for (int igamma = 0; igamma < nTagMatch; igamma++) {
      tpolHitToGo.eGammaIn[togoIndexNGamIn] = tagEVec[igamma];
      togoIndexNGamIn++;
    }
    int togoIndexNGamOut = 0;
    for (int igamma = 0; igamma < nTagOutMatch; igamma++) {
      tpolHitToGo.eGammaOut[togoIndexNGamOut] = tagEOutVec[igamma];
      togoIndexNGamOut++;
    }
    tpolHitToGo.nHits = togoIndex;
    tpolHitToGo.nGammaIn = togoIndexNGamIn;
    tpolHitToGo.nGammaOut = togoIndexNGamOut;
    //FILL THE TREE
    if (togoIndex > 0) outTree.Fill();
  } //END OF EVENT LOOP


  //WRITE THE OUTPUT FILE
  outFile.Write();

  return 0;

}


void printUsage(){
  fprintf(stderr,"\nUsage:");
  fprintf(stderr,"\nstage1 [-switches] <inputFile>\n");
  fprintf(stderr,"\nSWITCHES:\n");
  fprintf(stderr,"-h\tPrint this message\n");
  fprintf(stderr,"-o<arg>\tOutFileName. Default is tpolOutFile.root\n");
  fprintf(stderr,"-r<arg>\tRun number. Default is -99\n");
  fprintf(stderr,"-t<arg>\tTree type. Default is -99\n\n");

  std::cout<<"The current default operation is equivalent to the command:"<<std::endl;
  std::cout<<"stage1 -otpolOutFile.root inputFile -r-99 -t-99,"<<std::endl;
  std::cout<<"where \"inputFile\" must be suplied by the user\n"<<std::endl;

  exit(0);
}
