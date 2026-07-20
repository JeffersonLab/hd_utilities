#include "TTree.h"
#include "TFile.h"
#include "TH2.h"
#include "TF1.h"
#include "stage2.h"

using namespace std;

void printUsage(); 
int main(int argc, char **argv){
  char  hId[80];
  char  inFileName[80];
  char  outFileName[80];
  char *argptr;

  //Set the default output file
  sprintf(outFileName,"./tpolOutFile.root");
  int runNumber = -99;
  int polAngleInt = -1;
  bool doECorr = false;
  int yVal = 1;
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
      case 'e':
        doECorr = true;
        break;
      case 'r':
	runNumber = atoi(++argptr);
        break;
      case 'p':
	polAngleInt = atoi(++argptr);
        break;
      case 'y':
        yVal = atoi(++argptr);
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
  cout<<"runNumber = "<<runNumber<<endl;

  double polAngle = 9.5;
  if (polAngleInt > 0){
    polAngle = polAngleInt + 9.5;
  }



  
  //SETUP OUTPUT FILE
  TFile *outFile = new TFile(outFileName,"RECREATE");

  //HISTOGRAMS
  int runNumberLow = 11000;
  int runNumberHigh = 11700;
  if (runNumber >= 30274){
    runNumberLow = 30200;
    runNumberHigh = 31100;
  }
  if (runNumber >= 41000){
    runNumberLow = 41100;
    runNumberHigh = 42600;
  }
  if (runNumber >= 42600){
    runNumberLow = 50400;
    runNumberHigh = 52000;
  }
  if (runNumber >= 71300){
    runNumberLow = 71300;
    runNumberHigh = 73300;
  }
  if (runNumber >= 100490){
    runNumberLow = 100490;
    runNumberHigh = 101623;
  }
  if (yVal == 2021){
    runNumberLow = 90033;
    runNumberHigh = 90633;
  }

  double rLow = runNumberLow  - 0.5;
  double rHigh = runNumberHigh + 0.5;
  int nBinsRuns = runNumberHigh - runNumberLow + 1;

  int nEgBins = 30;
  int nEgBinsFB = 20;
  double coEdge = 8.8;
  double coEdgeLow = coEdge - 0.60;

  double eFBLow = 7.8;
  double eFBHigh = 8.8;

  double anaPower = 0.1904;
  
  //Run period 2019-11 batch 1 through 12                                       
  if (runNumber >= 71350 && runNumber <= 73266) {
    coEdge = 8.6;
    coEdgeLow = coEdge - 0.60;
  }
  //Run period 2021-11
  if (runNumber >= 90033 && runNumber <= 90633) {
    coEdge = 8.5;
    coEdgeLow = coEdge - 0.60;
  }
  //Run period 2022-05
  if (runNumber >= 100491 && runNumber <= 101622) {
    coEdge = 5.75;
    coEdgeLow = coEdge - 0.40;
    eFBLow = coEdge - 0.7;
    eFBHigh = coEdge + 0.30;
    nEgBinsFB = 10;
    anaPower = 0.108;
  }
  double centerShift = 0.0;
  if (yVal == 2021){
    centerShift = 0.1;
  }
  
  bool lowTest = false;

  TH1D *hEgammaFull   = new TH1D("hEgammaFull","",1200,0.0,12.0);
  TH1D *hEgammaFullCut1   = new TH1D("hEgammaFullCut1","",1200,0.0,12.0);
  TH1D *hEgammaFullCut2   = new TH1D("hEgammaFullCut2","",1200,0.0,12.0);
  TH1D *hEgammaFullCut3   = new TH1D("hEgammaFullCut3","",1200,0.0,12.0);
  TH1D *hEg           = new TH1D("hEg","",nEgBins,-3.0,3.0);
  TH1D *hEgFB         = new TH1D("hEgFB","",nEgBinsFB,eFBLow,eFBHigh);
  TH1D *hEgFB2        = new TH1D("hEgFB2","",10*nEgBinsFB,eFBLow,eFBHigh);
  TH1D *hEgFBAMO         = new TH1D("hEgFBAMO","",nEgBinsFB,eFBLow,eFBHigh);
  TH1D *hEgFB2AMO        = new TH1D("hEgFB2AMO","",10*nEgBinsFB,eFBLow,eFBHigh);
  TH1D *hEgFBENH         = new TH1D("hEgFBENH","",nEgBinsFB,eFBLow,eFBHigh);
  TH1D *hEgFB2ENH        = new TH1D("hEgFB2ENH","",10*nEgBinsFB,eFBLow,eFBHigh);
  TH1D *hAnaFB_N         = new TH1D("hAnaFB_N","",nEgBinsFB,eFBLow,eFBHigh);
  TH1D *hAnaFB_D         = new TH1D("hAnaFB_D","",nEgBinsFB,eFBLow,eFBHigh);

 TH2D *hEDiffVsESum = new TH2D("hEDiffVsESum","",100,2.0,12.0,100,-4.0,4.0);
 TH2D *hEDiffVsESumCut = new TH2D("hEDiffVsESumCut","",100,2.0,12.0,100,-4.0,4.0);

 TH1D *hPull         = new TH1D("hPull","",200,-10.0,10.0);
  
  TH2D *hPhiCutVsRunAtEg[nEgBins]; 
  for (Int_t nTmp = 0; nTmp < nEgBins; nTmp++) {
    sprintf(hId,"hPhiCutVsRunAtEg%d",nTmp + 1);
    hPhiCutVsRunAtEg[nTmp] = new TH2D(hId,"",nBinsRuns,rLow,rHigh,32,0.0,360.0);
    hPhiCutVsRunAtEg[nTmp]->Sumw2();
  }

  TH2D *hPhiCutVsRunAtEgFB[nEgBinsFB]; 
  for (Int_t nTmp = 0; nTmp < nEgBinsFB; nTmp++) {
    sprintf(hId,"hPhiCutVsRunAtEgFB%d",nTmp + 1);
    hPhiCutVsRunAtEgFB[nTmp] = new TH2D(hId,"",nBinsRuns,rLow,rHigh,32,0.0,360.0);
    hPhiCutVsRunAtEgFB[nTmp]->Sumw2();
  }

  TH1D *hTime           = new TH1D("hTime","",600,0.0,600.0);
  TH2D *hTimeVsPhi      = new TH2D("hTimeVsPhi","",32,0.0,360.0,600,0.0,600.0);
  TH1D *hSector         = new TH1D("hSector","",32,0.5,32.5);
  TH1D *hSector1        = new TH1D("hSector1","",32,0.5,32.5);
  TH1D *hSector2        = new TH1D("hSector2","",32,0.5,32.5);
  TH1D *hSector3        = new TH1D("hSector3","",32,0.5,32.5);
  TH2D *hEgFBVsRun      = new TH2D("hEgFBVsRun","",nBinsRuns,rLow,rHigh,nEgBinsFB,eFBLow,eFBHigh);

  TH1D *hEnh2VsRun     = new TH1D("hEnh2VsRun","",nBinsRuns,rLow,rHigh);
  TH1D *hEnhVsRun     = new TH1D("hEnhVsRun","",nBinsRuns,rLow,rHigh);

  TH1D *hEnh      = new TH1D("hEnh","",600,5.3,5.9);
  
  TH2D *hSectorVsRun    = new TH2D("hSectorVsRun","",nBinsRuns,rLow,rHigh,32,0.5,32.5);
  TH2D *hSectorVsRunCut = new TH2D("hSectorVsRunCut","",nBinsRuns,rLow,rHigh,32,0.5,32.5);
  hSectorVsRun->Sumw2();
  hSectorVsRunCut->Sumw2();

  TH2D *hPhiVsSector = new TH2D("hPhiVsSector","",32,0.5,32.5,32,0.0,360.0);

  TH2D *hEDepVsSector = new TH2D("hEDepVsSector","",32,0.5,32.5,750,0.0,1500.0);
  TH2D *hEDepCutVsSector = new TH2D("hEDepCutVsSector","",32,0.5,32.5,750,0.0,1500.0);

  TH1D *hPhi         = new TH1D("hPhi","",32,0.0,360.0);
  TH1D *hPhi1         = new TH1D("hPhi1","",32,0.0,360.0);
  TH1D *hPhi2         = new TH1D("hPhi2","",32,0.0,360.0);
  TH1D *hPhi3         = new TH1D("hPhi3","",32,0.0,360.0);
  TH2D *hPhiVsRun    = new TH2D("hPhiVsRun","",nBinsRuns,rLow,rHigh,32,0.0,360.0);
  TH1D *hPhiCut      = new TH1D("hPhiCut","",32,0.0,360.0);
  TH1D *hPhiCutOut1  = new TH1D("hPhiCutOut1","",32,0.0,360.0);
  TH1D *hPhiCutOut2  = new TH1D("hPhiCutOut2","",32,0.0,360.0);
  TH2D *hPhiCutVsRun = new TH2D("hPhiCutVsRun","",nBinsRuns,rLow,rHigh,32,0.0,360.0);
  hPhi->Sumw2();
  hPhiVsRun->Sumw2();
  hPhiCut->Sumw2();
  hPhiCutOut1->Sumw2();
  hPhiCutOut2->Sumw2();
  hPhiCutVsRun->Sumw2();

  TH1D *hAveVsRun       = new TH1D("hAveVsRun","",nBinsRuns,rLow,rHigh);      hAveVsRun->Sumw2();
  TH1D *hPolVsRun       = new TH1D("hPolVsRun","",nBinsRuns,rLow,rHigh);     hPolVsRun->Sumw2();
  TH1D *hPhiOffsetVsRun = new TH1D("hPhiOffsetVsRun","",nBinsRuns,rLow,rHigh);hPhiOffsetVsRun->Sumw2();
  TH1D *hM1VsRun        = new TH1D("hM1VsRun","",nBinsRuns,rLow,rHigh);       hM1VsRun->Sumw2();
  TH1D *hM1OffsetVsRun  = new TH1D("hM1OffsetVsRun","",nBinsRuns,rLow,rHigh); hM1OffsetVsRun->Sumw2();
  TH1D *hFracInOutVsRun = new TH1D("hFracInOutVsRun","",nBinsRuns,rLow,rHigh);

  TH1D *hPolVsRunFB1       = new TH1D("hPolVsRunFB1","",nBinsRuns,rLow,rHigh);     hPolVsRunFB1->Sumw2();
  TH1D *hPolVsRunFB2       = new TH1D("hPolVsRunFB2","",nBinsRuns,rLow,rHigh);     hPolVsRunFB2->Sumw2();
  TH1D *hPolVsRunFB3       = new TH1D("hPolVsRunFB3","",nBinsRuns,rLow,rHigh);     hPolVsRunFB3->Sumw2();
  TH1D *hPolVsRunFB4       = new TH1D("hPolVsRunFB4","",nBinsRuns,rLow,rHigh);     hPolVsRunFB4->Sumw2();
  TH1D *hPolVsRunFB5       = new TH1D("hPolVsRunFB5","",nBinsRuns,rLow,rHigh);     hPolVsRunFB5->Sumw2();
  TH1D *hPolVsRunFB6       = new TH1D("hPolVsRunFB6","",nBinsRuns,rLow,rHigh);     hPolVsRunFB6->Sumw2();
  TH1D *hPolVsRunFB7       = new TH1D("hPolVsRunFB7","",nBinsRuns,rLow,rHigh);     hPolVsRunFB7->Sumw2();
  TH1D *hPolVsRunFB8       = new TH1D("hPolVsRunFB8","",nBinsRuns,rLow,rHigh);     hPolVsRunFB8->Sumw2();
  TH1D *hPolVsRunFB9       = new TH1D("hPolVsRunFB9","",nBinsRuns,rLow,rHigh);     hPolVsRunFB9->Sumw2();
  TH1D *hPolVsRunFB10      = new TH1D("hPolVsRunFB10","",nBinsRuns,rLow,rHigh);    hPolVsRunFB10->Sumw2();

  TH1D *hPolVsRunFB4Cut       = new TH1D("hPolVsRunFB4Cut","",nBinsRuns,rLow,rHigh);     hPolVsRunFB4->Sumw2();
  
  TH1D *hPhiCutFB1      = new TH1D("hPhiCutFB1","",32,0.0,360.0);
  TH1D *hPhiCutFB2      = new TH1D("hPhiCutFB2","",32,0.0,360.0);
  TH1D *hPhiCutFB3      = new TH1D("hPhiCutFB3","",32,0.0,360.0);
  TH1D *hPhiCutFB4      = new TH1D("hPhiCutFB4","",32,0.0,360.0);
  TH1D *hPhiCutFB5      = new TH1D("hPhiCutFB5","",32,0.0,360.0);
  TH1D *hPhiCutFB6      = new TH1D("hPhiCutFB6","",32,0.0,360.0);
  TH1D *hPhiCutFB7      = new TH1D("hPhiCutFB7","",32,0.0,360.0);
  TH1D *hPhiCutFB8      = new TH1D("hPhiCutFB8","",32,0.0,360.0);
  TH1D *hPhiCutFB9      = new TH1D("hPhiCutFB9","",32,0.0,360.0);
  TH1D *hPhiCutFB10     = new TH1D("hPhiCutFB10","",32,0.0,360.0);
    
  
  TH1D *hPol       = new TH1D("hPol","",100,-1.0,1.0);

  TH1D *hPolFB1       = new TH1D("hPolFB1","",100,-1.0,1.0);
  TH1D *hPolFB2       = new TH1D("hPolFB2","",100,-1.0,1.0);
  TH1D *hPolFB3       = new TH1D("hPolFB3","",100,-1.0,1.0);
  TH1D *hPolFB4       = new TH1D("hPolFB4","",100,-1.0,1.0);
  TH1D *hPolFB5       = new TH1D("hPolFB5","",100,-1.0,1.0);
  TH1D *hPolFB6       = new TH1D("hPolFB6","",100,-1.0,1.0);
  TH1D *hPolFB7       = new TH1D("hPolFB7","",100,-1.0,1.0);
  TH1D *hPolFB8       = new TH1D("hPolFB8","",100,-1.0,1.0);
  TH1D *hPolFB9       = new TH1D("hPolFB9","",100,-1.0,1.0);
  TH1D *hPolFB10      = new TH1D("hPolFB10","",100,-1.0,1.0);
  
  TH1D *hPhiOffset = new TH1D("hPhiOffset","",720,-360.0,360.0);

  //  [0]*(1+[1]*cos(2*(3.14/16)*(x-[2])))+[3]*cos(3.14*(x-[4])/16)
  //  [0]*(1+[1]*cos(2*(3.14/180)*(x-[2])))+[3]*cos(3.14*(x-[4])/180)

  TH1D *hA0    = new TH1D("hA0","",30,6.0,12.0);
  TH1D *hA2    = new TH1D("hA2","",30,6.0,12.0);
  TH1D *hB2    = new TH1D("hB2","",30,6.0,12.0);
  TH1D *hA2A0  = new TH1D("hA2A0","",30,6.0,12.0);
  TH1D *hA2B2  = new TH1D("hA2B2","",30,6.0,12.0);
  TH1D *hB2A0  = new TH1D("hB2A0","",30,6.0,12.0);
  hA0->Sumw2();
  hA2->Sumw2();
  hB2->Sumw2();
  hA2A0->Sumw2();
  hA2B2->Sumw2();
  hB2A0->Sumw2();

  TH2D *hA0RunTag   = new TH2D("hA0RunTag","",nBinsRuns,rLow,rHigh,30,6.0,12.0);
  TH2D *hA2RunTag   = new TH2D("hA2RunTag","",nBinsRuns,rLow,rHigh,30,6.0,12.0);
  TH2D *hB2RunTag   = new TH2D("hB2RunTag","",nBinsRuns,rLow,rHigh,30,6.0,12.0);
  TH2D *hA2A0RunTag = new TH2D("hA2A0RunTag","",nBinsRuns,rLow,rHigh,30,6.0,12.0);
  TH2D *hA2B2RunTag = new TH2D("hA2B2RunTag","",nBinsRuns,rLow,rHigh,30,6.0,12.0);
  TH2D *hB2A0RunTag = new TH2D("hB2A0RunTag","",nBinsRuns,rLow,rHigh,30,6.0,12.0);
  hA0RunTag->Sumw2();
  hA2RunTag->Sumw2();
  hB2RunTag->Sumw2();
  hA2A0RunTag->Sumw2();
  hA2B2RunTag->Sumw2();
  hB2A0RunTag->Sumw2();

  //ENERGY DEPOSITION CUT
  double eCut = 230.0;//IN keV

  //TIMING CUTS
  double tOutLow2  = 80;
  double tOutHigh2 = 0;
  double tLow      = 112;
  double tHigh     = 212;//100 wide
  double tOutLow   = 220;
  double tOutHigh  = 320;//100 wide
  double outWeight = -1.0;

  //Changes
  double cutT = 50.0;
  tLow      = 124-8;
  tHigh     = 194+8;
  if (cutT > 0) {
    tOutLow   = tHigh;
    tOutHigh  = tOutLow+cutT;
    double deltaInT = tHigh-tLow;
    double deltaCutT = tOutHigh-tOutLow;
    outWeight = -deltaInT/deltaCutT;
  } else {
    tOutHigh  = tLow;
    tOutLow   = tOutHigh+cutT;
    double deltaInT = tHigh-tLow;
    double deltaCutT = tOutHigh-tOutLow;
    outWeight = -deltaInT/deltaCutT;
  }

  if (runNumber < 30274) { //Sp16 Set3
    tLow      = 140-8;
    tHigh     = 210+8;
    if (cutT > 0) {
      tOutLow   = tHigh;
      tOutHigh  = tOutLow+cutT;
      double deltaInT = tHigh-tLow;
      double deltaCutT = tOutHigh-tOutLow;
      outWeight = -deltaInT/deltaCutT;
    } else {
      tOutHigh  = tLow;
      tOutLow   = tOutHigh+cutT;
      double deltaInT = tHigh-tLow;
      double deltaCutT = tOutHigh-tOutLow;
      outWeight = -deltaInT/deltaCutT;
    }
  }

  if (runNumber > 41000) { //Sp18                                               
    tLow      = 280-8;
    tHigh     = 340+8;//Changed from 330+8 to 340+8 on 11-17-2019
    cutT      = 60;   //Changed from 50 to 60 on 11-17-2019
    if (cutT > 0) {
      tOutLow   = tHigh;
      tOutHigh  = tOutLow+cutT;
      double deltaInT = tHigh-tLow;
      double deltaCutT = tOutHigh-tOutLow;
      outWeight = -deltaInT/deltaCutT;
    } else {
      tOutHigh  = tLow;
      tOutLow   = tOutHigh+cutT;
      double deltaInT = tHigh-tLow;
      double deltaCutT = tOutHigh-tOutLow;
      outWeight = -deltaInT/deltaCutT;
    }
  }


  double accFrac = 4.0;

  //Setup the input tree                                                                                      
  TFile* inFile=new TFile(inFileName);
  TTree *inTree = (TTree*)inFile->Get("tpolTree1");
  //tpolHitToGo_t tpol00;         
  tpolHitToGo_t tpol0;         
  getBranchesT1(inTree,&tpol0);
  //
  //TRANSLATION FROM tpol00 TO tpol0
  //tpol0 = tpol00;
  //LOOPER
  for(Int_t i=0;i<inTree->GetEntries();i++){
  //for(Int_t i=0;i<100000;i++){ 

    if (i%100000 == 0) cout <<"Event = "<<i<<endl;
    inTree->GetEntry(i);
    double ePair  = tpol0.ePair;
    double deltaE = tpol0.deltaE;
    int nGammaIn  = tpol0.nGammaIn;
    int nGammaOut = tpol0.nGammaOut;
    int nHits     = tpol0.nHits;
    //cout<<"test:1 ePair, deltaE = "<<ePair<<" , "<<deltaE<<endl;
    bool psCutPair = psCut(ePair,deltaE,yVal);

    hEDiffVsESum->Fill(ePair,deltaE);
    if (psCutPair == false) hEDiffVsESumCut->Fill(ePair,deltaE);
    
    for (int iTag = 0; iTag < nGammaIn; iTag++) {
      hEgammaFull->Fill(tpol0.eGammaIn[iTag]);
      if (psCutPair == false) hEgammaFullCut1->Fill(tpol0.eGammaIn[iTag]);
      if (lowTest == true) hEgammaFullCut2->Fill(tpol0.eGammaIn[iTag]);
      if (nHits == 1) hEgammaFullCut3->Fill(tpol0.eGammaIn[iTag]);
    }
    
    
    //cout<<"nHits = "<<nHits<<endl;
    if (psCutPair == true && lowTest == false) continue;
    if (nHits != 1) continue;
    
    double eCorr = 1.0;

    if (doECorr == true) eCorr = eCorrect(tpol0.sector[0]);

    //cout<<"doECorr = "<<doECorr<<endl;

    double eDep = tpol0.eDep[0]*eCorr;
    double time = tpol0.time[0];
    double phi  = tpol0.phi[0]*3.14159/180.0;
    double phiDeg = tpol0.phi[0];
    
    double cw2 = cos(2*phi);
    double sw2 = sin(2*phi);

    hTime->Fill(time);
    hTimeVsPhi->Fill(phiDeg,time);
    hSectorVsRun->Fill(1.0*runNumber,tpol0.sector[0]*1.0);
    hSector->Fill(tpol0.sector[0]*1.0);
    hPhiVsRun->Fill(1.0*runNumber,phiDeg);
    hPhi->Fill(phiDeg);
    hPhiVsSector->Fill(tpol0.sector[0]*1.0,phiDeg);

    hEDepVsSector->Fill(tpol0.sector[0]*1.0,eDep);
    //cout<<"tpol0.sector[0]*1.0 , eDep = "<<tpol0.sector[0]*1.0<<" , "<<eDep<<endl;//asdf

    //CHECK IF SIGNAL IS WITHIN TIMING WINDOWS
    bool inTPOLTime = false;
    bool inSideBand = false;
    bool inSideBand1 = false;
    bool inSideBand2 = false;
    if (time > tLow     && time < tHigh)     inTPOLTime = true;
    if (time > tOutLow  && time < tOutHigh)  inSideBand1 = true;
    if (time > tOutLow2 && time < tOutHigh2) inSideBand2 = true;
    if (inSideBand1 == true) inSideBand = true; 
    if (inSideBand2 == true) inSideBand = true; 

    if (eDep > eCut && (inTPOLTime == true || inSideBand == true)) {

      //TAGGER IN TIME WITH PS
      for (int iTag = 0; iTag < nGammaIn; iTag++) {
	double eGammaIn = tpol0.eGammaIn[iTag];
	double weight = 1.0;
	//hEgammaFull->Fill(eGammaIn);
	if (inSideBand == true) weight = outWeight;
	hA0->Fill(eGammaIn,weight);
	hA2->Fill(eGammaIn,cw2*weight);
	hB2->Fill(eGammaIn,sw2*weight);
	hA2A0->Fill(eGammaIn,cw2*weight);
	hA2B2->Fill(eGammaIn,cw2*sw2*weight);
	hB2A0->Fill(eGammaIn,sw2*weight);
	
	//cout<<"test:1"<<endl;
	//int egIndex = hEg->FindBin(eGammaIn-coEdge) - 1;
	int egIndex = hEg->FindBin(eGammaIn-8.8+centerShift) - 1;


	
	//cout<<"test:2"<<endl;
	//cout<<"egIndex = "<<egIndex<<endl;
	if (egIndex >=0 && egIndex <=29) hPhiCutVsRunAtEg[egIndex]->Fill(1.0*runNumber,phiDeg,weight);

	hEgFB->Fill(eGammaIn);
	hEgFBVsRun->Fill(1.0*runNumber,eGammaIn);
	hEgFB2->Fill(eGammaIn);
	int egIndexFB = hEgFB->FindBin(eGammaIn) - 1;
	if (egIndexFB >=0 && egIndexFB <nEgBinsFB && eGammaIn >= eFBLow && eGammaIn <= eFBHigh){
	  hPhiCutVsRunAtEgFB[egIndexFB]->Fill(1.0*runNumber,phiDeg,weight);
	  hAnaFB_D->Fill(eGammaIn,weight);
	  double anaFac = 0.1307 - 0.00395734*eGammaIn;
	  hAnaFB_N->Fill(eGammaIn,weight*anaFac);

	  hEDepCutVsSector->Fill(tpol0.sector[0]*1.0,eDep,weight);
	  if (egIndexFB + 1 == 1)  hPhiCutFB1->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 2)  hPhiCutFB2->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 3)  hPhiCutFB3->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 4)  hPhiCutFB4->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 5)  hPhiCutFB5->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 6)  hPhiCutFB6->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 7)  hPhiCutFB7->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 8)  hPhiCutFB8->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 9)  hPhiCutFB9->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 10) hPhiCutFB10->Fill(phiDeg,weight); 
	}

	//cout<<"test:3"<<endl;
	if (eGammaIn > coEdgeLow && eGammaIn < coEdge){
	  hSectorVsRunCut->Fill(1.0*runNumber,tpol0.sector[0]*1.0,weight);
	  hPhiCutVsRun->Fill(1.0*runNumber,phiDeg,weight);

	  hPhiCut->Fill(phiDeg,weight);
	  

	  if (inTPOLTime == true){ 
	    hPhiCutOut1->Fill(phiDeg,weight);
	    hPhiCutOut2->Fill(phiDeg,weight);
	  }
	  if (inSideBand1 == true) hPhiCutOut1->Fill(phiDeg,2.0*weight);
	  if (inSideBand2 == true) hPhiCutOut2->Fill(phiDeg,2.0*weight);

	  hSector1->Fill(tpol0.sector[0]*1.0,weight);
	  hPhi1->Fill(phiDeg,weight);
	  hSector3->Fill(tpol0.sector[0]*1.0,weight);
	  hPhi3->Fill(phiDeg,weight);
	}
      }//tagger in-time loop
      
      //TAGGER OUT OF TIME WITH PS
      for (int iTag = 0; iTag < nGammaOut; iTag++) {
	double eGammaOut = tpol0.eGammaOut[iTag];
	double weight = -1.0/accFrac;//In time with tpol
	if (inSideBand == true) weight = -1.0*outWeight/accFrac;
	hA0->Fill(eGammaOut,weight);
	hA2->Fill(eGammaOut,cw2*weight);
	hB2->Fill(eGammaOut,sw2*weight);
	hA2A0->Fill(eGammaOut,cw2*weight);
	hA2B2->Fill(eGammaOut,cw2*sw2*weight);
	hB2A0->Fill(eGammaOut,sw2*weight);

	//cout<<"test:4"<<endl;
	//int egIndex = hEg->FindBin(eGammaOut - coEdge) - 1;
	int egIndex = hEg->FindBin(eGammaOut - 8.8+centerShift) - 1;
	//cout<<"test:5 egIndex = "<<egIndex<<endl;
	if (egIndex >=0 && egIndex <=29) hPhiCutVsRunAtEg[egIndex]->Fill(1.0*runNumber,phiDeg,weight);

	int egIndexFB = hEgFB->FindBin(eGammaOut) - 1;
	if (egIndexFB >=0 && egIndexFB <nEgBinsFB && eGammaOut >= eFBLow && eGammaOut <= eFBHigh){
	  hPhiCutVsRunAtEgFB[egIndexFB]->Fill(1.0*runNumber,phiDeg,weight);
          hAnaFB_D->Fill(eGammaOut,weight);
	  double anaFac = 0.1307 - 0.00395734*eGammaOut;
          hAnaFB_N->Fill(eGammaOut,weight*anaFac);

	  if (egIndexFB + 1 == 1)  hPhiCutFB1->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 2)  hPhiCutFB2->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 3)  hPhiCutFB3->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 4)  hPhiCutFB4->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 5)  hPhiCutFB5->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 6)  hPhiCutFB6->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 7)  hPhiCutFB7->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 8)  hPhiCutFB8->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 9)  hPhiCutFB9->Fill(phiDeg,weight);
	  if (egIndexFB + 1 == 10) hPhiCutFB10->Fill(phiDeg,weight); 

	}

	//cout<<"test:6"<<endl;

	if (eGammaOut > coEdgeLow && eGammaOut < coEdge){
	  hSectorVsRunCut->Fill(1.0*runNumber,tpol0.sector[0]*1.0,weight);
	  hPhiCutVsRun->Fill(1.0*runNumber,phiDeg,weight);
	  hPhiCut->Fill(phiDeg,weight);

	  if (inTPOLTime == true){ 
	    hPhiCutOut1->Fill(phiDeg,weight);
	    hPhiCutOut2->Fill(phiDeg,weight);
	  }
	  if (inSideBand1 == true) hPhiCutOut1->Fill(phiDeg,2.0*weight);
	  if (inSideBand2 == true) hPhiCutOut2->Fill(phiDeg,2.0*weight);

	  hSector2->Fill(tpol0.sector[0]*1.0,weight);
	  hPhi2->Fill(phiDeg,weight);
	  hSector3->Fill(tpol0.sector[0]*1.0,weight);
	  hPhi3->Fill(phiDeg,weight);
	}
      }//tagger out-of-time loop

    } //eDep > eCut 
  } //LOOPER END

  double egFBVec[10];
  egFBVec[0] = 706353;
  egFBVec[1] = 755082;
  egFBVec[2] = 695717;
  egFBVec[3] = 697861;
  egFBVec[4] = 815633;
  egFBVec[5] = 716457;
  egFBVec[6] = 943527;
  egFBVec[7] = 724611;
  egFBVec[8] = 762498;
  egFBVec[9] = 296851;

  double egFB2Vec[100];
  egFB2Vec[0] = 54410;
  egFB2Vec[1] = 160197;
  egFB2Vec[2] = 32515;
  egFB2Vec[3] = 20582;
  egFB2Vec[4] = 55196;
  egFB2Vec[5] = 56701;
  egFB2Vec[6] = 125337;
  egFB2Vec[7] = 92871;
  egFB2Vec[8] = 65516;
  egFB2Vec[9] = 43028;
  egFB2Vec[10] = 24050;
  egFB2Vec[11] = 39665;
  egFB2Vec[12] = 201805;
  egFB2Vec[13] = 55898;
  egFB2Vec[14] = 42909;
  egFB2Vec[15] = 50759;
  egFB2Vec[16] = 42821;
  egFB2Vec[17] = 30504;
  egFB2Vec[18] = 221644;
  egFB2Vec[19] = 45027;
  egFB2Vec[20] = 17576;
  egFB2Vec[21] = 36673;
  egFB2Vec[22] = 59101;
  egFB2Vec[23] = 45879;
  egFB2Vec[24] = 193291;
  egFB2Vec[25] = 58157;
  egFB2Vec[26] = 52667;
  egFB2Vec[27] = 15075;
  egFB2Vec[28] = 32363;
  egFB2Vec[29] = 184935;
  egFB2Vec[30] = 109426;
  egFB2Vec[31] = 26933;
  egFB2Vec[32] = 49754;
  egFB2Vec[33] = 67161;
  egFB2Vec[34] = 23413;
  egFB2Vec[35] = 223110;
  egFB2Vec[36] = 58121;
  egFB2Vec[37] = 46583;
  egFB2Vec[38] = 28697;
  egFB2Vec[39] = 64663;
  egFB2Vec[40] = 61828;
  egFB2Vec[41] = 195971;
  egFB2Vec[42] = 54707;
  egFB2Vec[43] = 69754;
  egFB2Vec[44] = 54954;
  egFB2Vec[45] = 29323;
  egFB2Vec[46] = 102977;
  egFB2Vec[47] = 183684;
  egFB2Vec[48] = 29534;
  egFB2Vec[49] = 32901;
  egFB2Vec[50] = 67798;
  egFB2Vec[51] = 66127;
  egFB2Vec[52] = 189590;
  egFB2Vec[53] = 49911;
  egFB2Vec[54] = 76751;
  egFB2Vec[55] = 52340;
  egFB2Vec[56] = 30367;
  egFB2Vec[57] = 65786;
  egFB2Vec[58] = 91779;
  egFB2Vec[59] = 26008;
  egFB2Vec[60] = 58241;
  egFB2Vec[61] = 60999;
  egFB2Vec[62] = 63413;
  egFB2Vec[63] = 150121;
  egFB2Vec[64] = 122669;
  egFB2Vec[65] = 66878;
  egFB2Vec[66] = 40915;
  egFB2Vec[67] = 45703;
  egFB2Vec[68] = 81094;
  egFB2Vec[69] = 253494;
  egFB2Vec[70] = 29275;
  egFB2Vec[71] = 61948;
  egFB2Vec[72] = 77501;
  egFB2Vec[73] = 45645;
  egFB2Vec[74] = 52931;
  egFB2Vec[75] = 223200;
  egFB2Vec[76] = 85490;
  egFB2Vec[77] = 32849;
  egFB2Vec[78] = 37939;
  egFB2Vec[79] = 77833;
  egFB2Vec[80] = 136767;
  egFB2Vec[81] = 151596;
  egFB2Vec[82] = 53864;
  egFB2Vec[83] = 80246;
  egFB2Vec[84] = 54352;
  egFB2Vec[85] = 47148;
  egFB2Vec[86] = 203969;
  egFB2Vec[87] = 34556;
  egFB2Vec[88] = 0;
  egFB2Vec[89] = 0;
  egFB2Vec[90] = 0;
  egFB2Vec[91] = 6201;
  egFB2Vec[92] = 166330;
  egFB2Vec[93] = 0;
  egFB2Vec[94] = 0;
  egFB2Vec[95] = 0;
  egFB2Vec[96] = 0;
  egFB2Vec[97] = 28745;
  egFB2Vec[98] = 95575;
  egFB2Vec[99] = 0;
  cout<<"test:1"<<endl;
  for (int i = 1; i <= 10; ++i){
    hEgFBAMO->SetBinContent(i,egFBVec[i-1]);
    //cout<<"i = "<<i<<endl;
  }
  cout<<"test:2"<<endl;

  for (int i = 1; i <= 100; ++i){
    hEgFB2AMO->SetBinContent(i,egFB2Vec[i-1]);
  }
  cout<<"test:3"<<endl;
  hEgFBENH->Sumw2();
  hEgFBENH->Divide(hEgFB,hEgFBAMO);
  hEgFB2ENH->Divide(hEgFB2,hEgFB2AMO);

  //double enhMax = hEgFBENH->GetBinCenter(hEgFBENH->GetMaximumBin());
  double enh2Max = hEgFB2ENH->GetBinCenter(hEgFB2ENH->GetMaximumBin());

  double x0ENHVal = hEgFBENH->GetBinContent(1);
  double minENHVal = hEgFBENH->GetMinimum();

  //DEFINE THE FIT FUNCTION
  MyEnhFun * fptrEnh = new MyEnhFun();
  int nParEnh = 5;
  TF1 *enhFun = new TF1("enhFun",fptrEnh,&MyEnhFun::Evaluate,5.0,7.0,nParEnh,
                           "MyEnhFun","Evaluate");
  
  enhFun->FixParameter(0,minENHVal);
  enhFun->FixParameter(1,x0ENHVal-minENHVal);
  enhFun->SetParameter(2,0.16); enhFun->SetParLimits(2,0.1,0.5);
  enhFun->SetParameter(3,5.8);  enhFun->SetParLimits(3,5.4,5.9);
  enhFun->SetParameter(4,25);  enhFun->SetParLimits(4,10.0,100.0);

  //enhFun->FixParameter(0,2.90801e-02);
  //enhFun->FixParameter(1,7.64840e-02);
  //enhFun->FixParameter(2,3.32259e-01);
  //enhFun->FixParameter(3,5.73377e+00);
  //enhFun->FixParameter(4,3.40915e+01);
  
  hEgFBENH->Fit("enhFun","IB","R",5.0,6.2);
  
  cout<<"test:4"<<endl;
  //Get fraction of in-time to out-of-time
  int iLow  = hTime->FindBin(tLow+0.0001);
  int iHigh = hTime->FindBin(tHigh-0.0001);
  int iOutLow  = hTime->FindBin(tOutLow+0.0001);
  int iOutHigh = hTime->FindBin(tOutHigh-0.0001);
  double fracInOut = hTime->Integral(iLow,iHigh)/hTime->Integral(iOutLow,iOutHigh);

  //DEFINE THE FIT FUNCTION AND RUN THE FIT
  MyPhiFitFun * fptr1 = new MyPhiFitFun();
  int nPar = 5;
  TF1 *phiFun = new TF1("phiFun",fptr1,&MyPhiFitFun::Evaluate,0.0,360.0,nPar,
                           "MyPhiFitFun","Evaluate");
  double sumVal = hPhiCut->Integral();
  double aveVal = sumVal/32.0;
  phiFun->SetParameter(0,aveVal);//Ave
  phiFun->SetParameter(1,0.0);   //pSig
  phiFun->SetParameter(2,polAngle);   //phiOffset
  phiFun->SetParameter(3,0.0);   //m1
  phiFun->SetParameter(4,0.0);   //m1Offset

  phiFun->SetParLimits(3,0.0,10.0);   //M1

  if (polAngleInt < 0) phiFun->FixParameter(2,0); 
  
  //hPhiCut->SetBinError(14,0);
  //hPhiCut->SetBinError(15,0);
  //hPhiCut->SetBinError(16,0);
  hPhiCut->Fit("phiFun","IR","",0.0,360.0);

  double p2Val = phiFun->GetParameter(2);
  double p2Err = phiFun->GetParError(2);

  phiFun->FixParameter(2,polAngle);   //phiOffset
  if (polAngleInt < 0) phiFun->FixParameter(2,0); 
  
  hPhiCut->Fit("phiFun","IR","",0.0,360.0);

  double p0Val = phiFun->GetParameter(0);
  double p1Val = phiFun->GetParameter(1);
  double p3Val = phiFun->GetParameter(3);
  double p4Val = phiFun->GetParameter(4);
  double p0Err = phiFun->GetParError(0);
  double p1Err = phiFun->GetParError(1);
  double p3Err = phiFun->GetParError(3);
  double p4Err = phiFun->GetParError(4);
  int rBinNumber = hAveVsRun->FindBin(runNumber);

  hFracInOutVsRun->SetBinContent(rBinNumber,fracInOut);

  double enhEdge = enhFun->GetParameter(3);
  
  hEnhVsRun->SetBinContent(rBinNumber,enhFun->GetParameter(3));
  hEnh2VsRun->SetBinContent(rBinNumber,enh2Max);

  hEnhVsRun->SetBinError(rBinNumber,enhFun->GetParError(3));

  hEnh->Fill(enhFun->GetParameter(3));
  
  //hAveVsRun->SetBinContent(rBinNumber,enhFun->GetParError(3));
  //hAveVsRun->SetBinError(rBinNumber,p0Err);
 
  hPull->Fill(p1Val/p1Err);
  
  hPolVsRun->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRun->SetBinError(rBinNumber,p1Err/anaPower);

  hPol->Fill(p1Val/anaPower);

  //FB1
  hPhiCutFB1->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB1->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB1->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB1->Fill(p1Val/anaPower);
  
  //FB2
  hPhiCutFB2->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB2->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB2->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB2->Fill(p1Val/anaPower);
  
  //FB3
  hPhiCutFB3->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB3->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB3->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB3->Fill(p1Val/anaPower);
  
  //FB4
  hPhiCutFB4->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB4->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB4->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB4->Fill(p1Val/anaPower);
  //if (enhEdge > 5.76 && enhEdge < 5.78){
  if (p1Val/anaPower > 0.6 && p1Val/anaPower < 0.80){
    hPolVsRunFB4Cut->SetBinContent(rBinNumber,p1Val/anaPower);
    hPolVsRunFB4Cut->SetBinError(rBinNumber,p1Err/anaPower);
  }

  
  //FB5
  hPhiCutFB5->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB5->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB5->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB5->Fill(p1Val/anaPower);
  
  //FB6
  hPhiCutFB6->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB6->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB6->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB6->Fill(p1Val/anaPower);
  
  //FB7
  hPhiCutFB7->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB7->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB7->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB7->Fill(p1Val/anaPower);
  
  //FB8
  hPhiCutFB8->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB8->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB8->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB8->Fill(p1Val/anaPower);  
  
  //FB9
  hPhiCutFB9->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB9->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB9->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB9->Fill(p1Val/anaPower);
  
  //FB10
  hPhiCutFB10->Fit("phiFun","IR","",0.0,360.0); p1Val = phiFun->GetParameter(1);p1Err = phiFun->GetParError(1);
  hPolVsRunFB10->SetBinContent(rBinNumber,p1Val/anaPower);
  hPolVsRunFB10->SetBinError(rBinNumber,p1Err/anaPower);
  hPolFB10->Fill(p1Val/anaPower);
  
  //cout<<"rBinNumber, p1Val, anaPower = "<<rBinNumber<<" , "<<p1Val<<" , "<<anaPower<<endl;

  hPhiOffsetVsRun->SetBinContent(rBinNumber,p2Val);
  hPhiOffsetVsRun->SetBinError(rBinNumber,p2Err);
  hPhiOffset->Fill(p2Val);
  hM1VsRun->SetBinContent(rBinNumber,p3Val);
  hM1VsRun->SetBinError(rBinNumber,p3Err);
  hM1OffsetVsRun->SetBinContent(rBinNumber,p4Val);
  hM1OffsetVsRun->SetBinError(rBinNumber,p4Err);

  //RUN BINNING
  int nBinsX = hA0->GetNbinsX();
  int runBin = hA0RunTag->GetXaxis()->FindBin(1.0*runNumber);
  for (int iBin = 1; iBin <= nBinsX; iBin++){
    double a0ValTag = hA0->GetBinContent(iBin);
    double a0ErrTag = hA0->GetBinError(iBin);
    double a2ValTag = hA2->GetBinContent(iBin);
    double a2ErrTag = hA2->GetBinError(iBin);
    double b2ValTag = hB2->GetBinContent(iBin);
    double b2ErrTag = hB2->GetBinError(iBin);
    double a2a0ValTag = hA2A0->GetBinContent(iBin);
    double a2a0ErrTag = hA2A0->GetBinError(iBin);
    double a2b2ValTag = hA2B2->GetBinContent(iBin);
    double a2b2ErrTag = hA2B2->GetBinError(iBin);
    double b2a0ValTag = hB2A0->GetBinContent(iBin);
    double b2a0ErrTag = hB2A0->GetBinError(iBin);
    hA0RunTag->SetBinContent(runBin,iBin,a0ValTag);
    hA0RunTag->SetBinError(runBin,iBin,a0ErrTag);
    hA2RunTag->SetBinContent(runBin,iBin,a2ValTag);
    hA2RunTag->SetBinError(runBin,iBin,a2ErrTag);
    hB2RunTag->SetBinContent(runBin,iBin,b2ValTag);
    hB2RunTag->SetBinError(runBin,iBin,b2ErrTag);
    hA2A0RunTag->SetBinContent(runBin,iBin,a2a0ValTag);
    hA2A0RunTag->SetBinError(runBin,iBin,a2a0ErrTag);
    hA2B2RunTag->SetBinContent(runBin,iBin,a2b2ValTag);
    hA2B2RunTag->SetBinError(runBin,iBin,a2b2ErrTag);
    hB2A0RunTag->SetBinContent(runBin,iBin,b2a0ValTag);
    hB2A0RunTag->SetBinError(runBin,iBin,b2a0ErrTag);
  }
  cout<<"test:8"<<endl;
  //hEg->Delete();
  outFile->cd();
  cout<<"test:9"<<endl;
  outFile->Write();
  cout<<"test:10"<<endl;
  delete outFile;
  cout<<"test:11"<<endl;
  return 0;
}

void printUsage(){
  fprintf(stderr,"\nUsage:");
  fprintf(stderr,"\nstage2 [-switches] <inputFile>\n");
  fprintf(stderr,"\nSWITCHES:\n");
  fprintf(stderr,"-h\tPrint this message\n");
  fprintf(stderr,"-r\tRun number\n");
  fprintf(stderr,"-o<arg>\tOutFileName. Default is tpolOutFile.root\n\n");

  cout<<"The current default operation is equivalent to the command:"<<endl;
  cout<<"stage2 -otpolOutFile.root inputFile ,"<<endl;
  cout<<"where \"inputFile\" must be suplied by the user\n"<<endl;

  exit(0);
}




