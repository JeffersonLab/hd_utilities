 
struct tpolHit_t {
  Double_t eDep;
  Double_t time;
  UInt_t sector;
  UInt_t w_samp1;
  UInt_t w_min;
  UInt_t w_max;
  UInt_t w_integral;
  Double_t phi;
  Double_t ePair;
  Double_t deltaE;
  Double_t E_lhit;
  Double_t E_rhit;
  Double_t t_lhit;
  Double_t t_rhit;
  Double_t eTag;
  Double_t eTagOut;
  bool gotTag;
  bool gotTagOut;
  vector<double> tagEVec;
  vector<double> tagEOutVec;

};

struct tpolHitToGo_t {
  UInt_t   nHits;
  UInt_t   nHitsTot;
  UInt_t   inTime[32];
  Double_t eDep[32];
  Double_t time[32];
  UInt_t   sector[32];
  UInt_t w_integral[32];
  UInt_t w_min[32];
  UInt_t w_max[32];
  UInt_t w_samp1[32];
  Double_t phi[32];
  Double_t ePair;
  Double_t deltaE;
  Double_t E_lhit;
  Double_t E_rhit;
  Double_t t_lhit;
  Double_t t_rhit;
  UInt_t nGammaIn;
  UInt_t nGammaOut;
  Double_t eGammaIn[20];
  Double_t eGammaOut[20];
};
void setBranchesHitToGo(TTree *outTree, tpolHitToGo_t *tpol1){
  outTree->Branch("nHits",&tpol1->nHits,"nHits/i");
  outTree->Branch("nHitsTot",&tpol1->nHitsTot,"nHitsTot/i");
  outTree->Branch("inTime",&tpol1->inTime,"inTime[nHits]/i");
  outTree->Branch("eDep",&tpol1->eDep,"eDep[nHits]/D");
  outTree->Branch("time",&tpol1->time,"time[nHits]/D");
  outTree->Branch("sector",&tpol1->sector,"sector[nHits]/i");
  outTree->Branch("w_integral",&tpol1->w_integral,"w_integral[nHits]/i");
  outTree->Branch("w_min",&tpol1->w_min,"w_min[nHits]/i");
  outTree->Branch("w_max",&tpol1->w_max,"w_max[nHits]/i");
  outTree->Branch("w_samp1",&tpol1->w_samp1,"w_samp1[nHits]/i");
  outTree->Branch("phi",tpol1->phi,"phi[nHits]/D");
  outTree->Branch("ePair",&tpol1->ePair,"ePair/D");
  outTree->Branch("deltaE",&tpol1->deltaE,"deltaE/D");
  outTree->Branch("E_lhit",&tpol1->E_lhit,"E_lhit/D");
  outTree->Branch("E_rhit",&tpol1->E_rhit,"E_rhit/D");
  outTree->Branch("t_lhit",&tpol1->t_lhit,"t_lhit/D");
  outTree->Branch("t_rhit",&tpol1->t_rhit,"t_rhit/D");
  outTree->Branch("nGammaIn",&tpol1->nGammaIn,"nGammaIn/i");
  outTree->Branch("nGammaOut",&tpol1->nGammaOut,"nGammaOut/i");
  outTree->Branch("eGammaIn",&tpol1->eGammaIn,"eGammaIn[nGammaIn]/D");
  outTree->Branch("eGammaOut",&tpol1->eGammaOut,"eGammaOut[nGammaOut]/D");
};
//////////////////


static const UShort_t nmax = 32;

struct tpol0_t {
  UShort_t nadc;
  ULong64_t eventnum;
  UShort_t rocid[nmax];
  UShort_t slot[nmax];
  UShort_t channel[nmax];
  UInt_t itrigger[nmax];
  UInt_t nsamples;
  UShort_t waveform[nmax][150];
  ULong64_t w_integral[nmax];
  UShort_t w_min[nmax];
  UShort_t w_max[nmax];
  UShort_t w_samp1[nmax];
  Double_t w_time[nmax];
  Double_t sector[nmax];
  Double_t phi[nmax];
  Double_t E_lhit[nmax];
  Double_t E_rhit[nmax];
  Double_t t_lhit[nmax];
  Double_t t_rhit[nmax];
  UShort_t ntag;
  Double_t E_tag[50];
  Double_t t_tag[50];
  bool is_tagm[50];
};



struct tpolTmp_t {
  UShort_t nadc;                
  ULong64_t eventnum;           
  UInt_t rocid[nmax];        
  UInt_t slot[nmax];         
  UInt_t channel[nmax];      
  UInt_t itrigger[nmax];     
  ULong64_t w_integral[nmax];   
  ULong64_t w_max[nmax];        
  ULong64_t w_min[nmax];        
  ULong64_t w_samp1[nmax];      
  UInt_t    sector[nmax];       
  Double_t  phi[nmax];
  ULong64_t ntpol; 
  UShort_t  waveform[4800]; 
  Double_t  PSenergy_lhit; 
  Double_t  PSenergy_rhit; 
  Double_t  PSCtime_lhit;
  Double_t  PSCtime_rhit;
  Double_t  PStime_lhit;  
  Double_t  PStime_rhit;  
  UShort_t  ntagh;
  Bool_t    TAGH_DBeam[50];
  Double_t  TAGHenergy[50];
  Double_t  TAGHtime[50];  
  UInt_t    TAGHcounter[50];
  UShort_t  ntagm;
  Bool_t    TAGM_DBeam[50]; 
  Double_t  TAGMenergy[50]; 
  Double_t  TAGMtime[50];   
  UInt_t    TAGMcolumn[50]; 
};

struct tpolTmp2_t {
  UShort_t nadc;            
  ULong64_t eventnum;       
  UShort_t rocid[nmax];        
  UShort_t slot[nmax];         
  UShort_t channel[nmax];      
  UInt_t itrigger[nmax];     
  ULong64_t w_integral[nmax];
  UShort_t w_max[nmax];      
  UShort_t w_min[nmax];      
  UShort_t w_samp1[nmax];    
  UInt_t    sector[nmax];    
  Double_t  phi[nmax];
  UShort_t ntpol; 
  UShort_t  waveform[4800];
  UShort_t        nPSC;
  Double_t        PSCtime_lhit[24]; 
  Double_t        PSCtime_rhit[24]; 
  Int_t           PSCmodule_lhit[24];
  Int_t           PSCmodule_rhit[24];
  UShort_t        nPS;
  Double_t        PSenergy_lhit[36]; 
  Double_t        PSenergy_rhit[36]; 
  Double_t        PStime_lhit[36];   
  Double_t        PStime_rhit[36];   
  Int_t           PScolumn_lhit[36]; 
  Int_t           PScolumn_rhit[36]; 
  UShort_t  ntagh;
  Bool_t    TAGH_DBeam[129];  
  Double_t  TAGHenergy[129];  
  Double_t  TAGHtime[129];   
  UInt_t    TAGHcounter[129];
  UShort_t  ntagm;
  Bool_t    TAGM_DBeam[129];
  Double_t  TAGMenergy[129];
  Double_t  TAGMtime[129];  
  UInt_t    TAGMcolumn[129];
};


void tpolStructConvert2(tpol0_t *tpol0, tpolTmp2_t *tpolTmp){

  tpol0->nadc = tpolTmp->nadc;
  tpol0->eventnum = tpolTmp->eventnum;
  for (int i = 0; i<nmax; i++) {
    tpol0->rocid[i] = tpolTmp->rocid[i];
    tpol0->slot[i] = tpolTmp->slot[i];
    tpol0->channel[i] = tpolTmp->channel[i];
    tpol0->itrigger[i] = tpolTmp->itrigger[i];
    tpol0->w_integral[i] = tpolTmp->w_integral[i];
    tpol0->w_min[i] = tpolTmp->w_min[i];
    tpol0->w_max[i] = tpolTmp->w_max[i];
    tpol0->w_samp1[i] = tpolTmp->w_samp1[i];
    tpol0->sector[i] = tpolTmp->sector[i];
    tpol0->phi[i] = tpolTmp->phi[i];
  }
  int j = 0;
  int k = 0;
  for (UShort_t i = 0; i < tpolTmp->ntpol; i++){
    tpol0->waveform[k][j] = tpolTmp->waveform[i];
    j++;
    if (j == 150){
      j = 0;
      k++;
    }
  }

  bool goodPS = false;
  if (tpolTmp->nPSC >= 1) {
    if (tpolTmp->nPS >= 1) {
      double pscTimeDiff = tpolTmp->PSCtime_lhit[0] - tpolTmp->PSCtime_rhit[0];
      if (fabs(pscTimeDiff)<1.3) goodPS = true;
    }
  }

  if (goodPS == true) {
    tpol0->E_lhit[0] = tpolTmp->PSenergy_lhit[0];
    tpol0->E_rhit[0] = tpolTmp->PSenergy_rhit[0];

    tpol0->t_lhit[0] = tpolTmp->PSCtime_lhit[0];
    tpol0->t_rhit[0] = tpolTmp->PSCtime_rhit[0];
  }
  double eDiffMax = 10.3;
  double tDiffMax = 15.0;
  int ntag_max = 1200;
  int htag = 0;
  double t_lhit = tpol0->t_lhit[0];
  double ePair = tpolTmp->PSenergy_lhit[0] + tpolTmp->PSenergy_rhit[0];
  for (int i = 0; i<tpolTmp->ntagh; i++) {
    if (fabs(ePair  - tpolTmp->TAGHenergy[i])<eDiffMax &&
        fabs(t_lhit - tpolTmp->TAGHtime[i])<tDiffMax &&
        htag < ntag_max){
      tpol0->E_tag[htag] = tpolTmp->TAGHenergy[i];
      tpol0->t_tag[htag] = tpolTmp->TAGHtime[i];
      tpol0->is_tagm[htag] = false;
      htag++;
    }
  }

  for (int i = 0; i<tpolTmp->ntagm; i++) {
    if (fabs(ePair  - tpolTmp->TAGMenergy[i])<eDiffMax &&
        fabs(t_lhit - tpolTmp->TAGMtime[i])<tDiffMax &&
        htag < ntag_max){
      tpol0->E_tag[htag] = tpolTmp->TAGMenergy[i];
      tpol0->t_tag[htag] = tpolTmp->TAGMtime[i];
      tpol0->is_tagm[htag] = true;
      htag++;
    }
  }

  tpol0->ntag = htag;
};

void getBranchesT0(TTree *inTree, tpol0_t *tpol0){
  inTree->SetBranchAddress("nadc",&tpol0->nadc);
  inTree->SetBranchAddress("eventnum",&tpol0->eventnum);
  inTree->SetBranchAddress("rocid",tpol0->rocid);
  inTree->SetBranchAddress("slot",tpol0->slot);
  inTree->SetBranchAddress("channel",tpol0->channel);
  inTree->SetBranchAddress("itrigger",tpol0->itrigger);
  inTree->SetBranchAddress("waveform",tpol0->waveform);
  inTree->SetBranchAddress("w_integral",tpol0->w_integral);
  inTree->SetBranchAddress("w_min",tpol0->w_min);
  inTree->SetBranchAddress("w_max",tpol0->w_max);
  inTree->SetBranchAddress("w_samp1",tpol0->w_samp1);
  inTree->SetBranchAddress("phi",tpol0->phi);
  inTree->SetBranchAddress("E_lhit",&tpol0->E_lhit);
  inTree->SetBranchAddress("E_rhit",&tpol0->E_rhit);
  inTree->SetBranchAddress("t_lhit",&tpol0->t_lhit);
  inTree->SetBranchAddress("t_rhit",&tpol0->t_rhit);

  inTree->SetBranchAddress("ntag",&tpol0->ntag);
  inTree->SetBranchAddress("E_tag",tpol0->E_tag);
  inTree->SetBranchAddress("t_tag",tpol0->t_tag);
  inTree->SetBranchAddress("is_tagm",tpol0->is_tagm);
};

void getBranchesT02(TTree *inTree, tpolTmp2_t *tpolTmp){
  inTree->SetBranchAddress("nadc",&tpolTmp->nadc);
  inTree->SetBranchAddress("eventnum",&tpolTmp->eventnum);
  inTree->SetBranchAddress("rocid",tpolTmp->rocid);
  inTree->SetBranchAddress("slot",tpolTmp->slot);
  inTree->SetBranchAddress("channel",tpolTmp->channel);
  inTree->SetBranchAddress("itrigger",tpolTmp->itrigger);

  inTree->SetBranchAddress("phi",tpolTmp->phi);

  inTree->SetBranchAddress("w_integral",tpolTmp->w_integral);
  inTree->SetBranchAddress("w_max",tpolTmp->w_max);
  inTree->SetBranchAddress("w_min",tpolTmp->w_min);
  inTree->SetBranchAddress("w_samp1",tpolTmp->w_samp1);
  inTree->SetBranchAddress("ntpol",&tpolTmp->ntpol);
  inTree->SetBranchAddress("waveform",tpolTmp->waveform);

  inTree->SetBranchAddress("nPSC",&tpolTmp->nPSC);
  inTree->SetBranchAddress("PSCtime_lhit",tpolTmp->PSCtime_lhit);
  inTree->SetBranchAddress("PSCtime_rhit",tpolTmp->PSCtime_rhit);
  inTree->SetBranchAddress("nPS",&tpolTmp->nPS);
  inTree->SetBranchAddress("PSenergy_lhit",tpolTmp->PSenergy_lhit);
  inTree->SetBranchAddress("PSenergy_rhit",tpolTmp->PSenergy_rhit);
  inTree->SetBranchAddress("PStime_lhit",&tpolTmp->PStime_lhit);
  inTree->SetBranchAddress("PStime_rhit",&tpolTmp->PStime_rhit);
  
  inTree->SetBranchAddress("ntagh",&tpolTmp->ntagh);
  inTree->SetBranchAddress("TAGH_DBeam",&tpolTmp->TAGH_DBeam);
  inTree->SetBranchAddress("TAGHenergy",&tpolTmp->TAGHenergy);
  inTree->SetBranchAddress("TAGHtime",&tpolTmp->TAGHtime);
  inTree->SetBranchAddress("TAGHcounter",&tpolTmp->TAGHcounter);

  inTree->SetBranchAddress("ntagm",&tpolTmp->ntagm);
  inTree->SetBranchAddress("TAGM_DBeam",&tpolTmp->TAGM_DBeam);
  inTree->SetBranchAddress("TAGMenergy",&tpolTmp->TAGMenergy);
  inTree->SetBranchAddress("TAGMtime",&tpolTmp->TAGMtime);
  inTree->SetBranchAddress("TAGMcolumn",&tpolTmp->TAGMcolumn);

};

class  MySpiceFun2 {
 public:
  void  SetSample(TH1D *inHisto) {hSample = inHisto;}
  double Evaluate(double *tPtr, double *parPtr) {
    double fitVal = 0.0;
    double tVal = *tPtr;
    //Get the histogram
    int i = hSample->GetXaxis()->FindBin(tVal + parPtr[1]*100.0);
    double hVal = hSample->GetBinContent(i);
    double wave = parPtr[3]*sin(tVal*2*3.14159/parPtr[4] + parPtr[5]);
    fitVal = hVal*fabs(parPtr[0]) + parPtr[2] + wave;
    return fitVal;
  }
  TH1D *hSample;
};

double signalTime(TH1D *hSignal, int nBins){
  double maxValSignal = hSignal->GetMaximum();
  double minValSignal = hSignal->GetMinimum();
  double deltaSignal = maxValSignal - minValSignal;
  double halfSignal = minValSignal + deltaSignal/2.0;
  double diffBest = 10000.0;
  double diffTest = -1.0;
  int iBin = -1;
  for (int i = 1; i< nBins; i++) {
    double testVal = hSignal->GetBinContent(i);
    diffTest = fabs(testVal - halfSignal);
    if (diffTest < diffBest) {
      diffBest = diffTest;
      iBin = i;
    }
  }
  double tValSignal = hSignal->GetBinCenter(iBin);
  return tValSignal;
}

double signalTimeSamp1(TH1D *hSignal, int nBins, double samp1){
  double maxValSignal = hSignal->GetMaximum();
  double minValSignal = hSignal->GetMinimum();
  if (samp1 > 110.0) minValSignal = 100.0;
  minValSignal = 100.0;
  double deltaSignal = maxValSignal - minValSignal;
  double halfSignal = minValSignal + deltaSignal/2.0;
  double diffBest = 10000.0;
  double diffTest = -1.0;
  int iBin = -1;
  for (int i = 1; i< nBins; i++) {
    double testVal = hSignal->GetBinContent(i);
    diffTest = fabs(testVal - halfSignal);
    if (diffTest < diffBest) {
      diffBest = diffTest;
      iBin = i;
    }
  }
  double tValSignal = hSignal->GetBinCenter(iBin);
  return tValSignal;
}


//////////////////
double sectorToPhi(int sector){
  double phiVal = -1.0;
  double deltaPhi = 360.0/32.0;
  if (sector >= 9) phiVal = (sector -  9)*deltaPhi + deltaPhi/2.0;
  if (sector <= 8) phiVal = (sector + 23)*deltaPhi + deltaPhi/2.0;
  return phiVal;
}
//////////////////
double slotChannelToSector4Slot(int slot, int channel){
  int sector = -1;
  if (slot == 13) sector = 25 - channel;
  if (slot == 14) sector = 17 - channel;
  if (slot == 15) sector = 9 - channel;
  if (slot == 16){
    if (channel == 0){
      sector = 1;
    }else{
      sector = 33 - channel;
    }
  }
  // fix cable swap                                                                                                      
  //if (sector == 9) sector = 6;
  //else if (sector == 6) sector = 9;

  int sectorOld = sector;
  //Cable swap
  if (sectorOld == 22) sector = 25;
  if (sectorOld == 25) sector = 22;

  return sector;
}

double slotChannelToSector(int slot, int channel){
  int sector = -1;
  if (slot == 13) sector = 25 - channel;
  if (slot == 14){
    if (channel <= 8) sector = 9 - channel;
    if (channel >= 9) sector = 41 - channel;
  }
  int sectorOld = sector;
  //Cable swap
  //if (sectorOld == 9) sector = 6;
  //if (sectorOld == 6) sector = 9;

  if (sectorOld == 22) sector = 25;
  if (sectorOld == 25) sector = 22;


  return sector;
}
//////////////////
double tCorrect(int sector){
  double tCorrVec[32];

  tCorrVec[0] = 4;
  tCorrVec[1] = 0;
  tCorrVec[2] = 0;
  tCorrVec[3] = 0;
  tCorrVec[4] = 0;
  tCorrVec[5] = 0;
  tCorrVec[6] = 0;
  tCorrVec[7] = 0;
  tCorrVec[8] = 0;
  tCorrVec[9] = -4;
  tCorrVec[10] = 0;
  tCorrVec[11] = 0;
  tCorrVec[12] = 4;
  tCorrVec[13] = 4;
  tCorrVec[14] = 4;
  tCorrVec[15] = 0;
  tCorrVec[16] = 4;
  tCorrVec[17] = 0;
  tCorrVec[18] = 4;
  tCorrVec[19] = 0;
  tCorrVec[20] = 0;
  tCorrVec[21] = 0;
  tCorrVec[22] = 4;
  tCorrVec[23] = 0;
  tCorrVec[24] = 4;
  tCorrVec[25] = 4;
  tCorrVec[26] = 4;
  tCorrVec[27] = 8;
  tCorrVec[28] = 4;
  tCorrVec[29] = 4;
  tCorrVec[30] = 8;
  tCorrVec[31] = 0;

  double retVal = tCorrVec[sector-1];
  return retVal;
}
//////////////////
double eCorrect(int sector){
  double mpvVec[32];
  
  mpvVec[0] = 1.13987;
  mpvVec[1] = 1.11457;
  mpvVec[2] = 1.15158;
  mpvVec[3] = 1.12327;
  mpvVec[4] = 1.13782;
  mpvVec[5] = 1.11003;
  mpvVec[6] = 1.11997;
  mpvVec[7] = 1.12088;
  mpvVec[8] = 1.12937;
  mpvVec[9] = 1.07425;
  mpvVec[10] = 1.10008;
  mpvVec[11] = 1.11187;
  mpvVec[12] = 1.12619;
  mpvVec[13] = 1.11441;
  mpvVec[14] = 1.12094;
  mpvVec[15] = 1.09684;
  mpvVec[16] = 1.11698;
  mpvVec[17] = 1.12961;
  mpvVec[18] = 1.13663;
  mpvVec[19] = 1.11414;
  mpvVec[20] = 1.15012;
  mpvVec[21] = 1.11363;
  mpvVec[22] = 1.12184;
  mpvVec[23] = 1.0988;
  mpvVec[24] = 1.14671;
  mpvVec[25] = 1.12821;
  mpvVec[26] = 1.13377;
  mpvVec[27] = 1.13968;
  mpvVec[28] = 1.14177;
  mpvVec[29] = 1.12761;
  mpvVec[30] = 1.09235;
  mpvVec[31] = 1.1045;

  double retVal = mpvVec[sector-1];
  return retVal;
}

TH1D *MakeSpiceHisto(void){
  double ltArray[600] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0.113898,0.241118,
    0.435056,0.800569,1.31994,2.00178,3.09584,4.70424,6.22882,8.3982,11.2118,13.9196,
    16.7483,20.9758,25.7056,29.9707,34.4737,39.6808,43.8675,50.5703,56.2855,62.1195,
    68.0723,75.2009,80.6983,87.0188,93.3022,102.144,107.846,113.414,118.847,124.145,
    129.309,134.338,138.514,142.418,149.821,153.32,156.683,159.912,165.035,167.964,
    172.06,174.116,177.988,179.805,183.198,185.281,188.178,190.01,192.12,193.589,
    195.173,197.103,198.448,199.689,200.827,201.912,202.901,203.8,204.611,205.352,
    206.086,206.512,207.102,207.686,207.949,208.42,208.626,209,209.248,209.435,
    209.613,209.776,209.921,209.982,210.082,210.153,210.189,210.22,210.231,210.226,
    210.208,210.195,210.159,210.108,210.059,209.967,209.909,209.847,209.782,209.713,
    209.564,209.497,209.404,209.312,209.219,209.124,209.026,208.927,208.724,208.595,
    208.491,208.386,208.28,208.173,208.065,207.957,207.848,207.705,207.591,207.476,
    207.361,207.245,207.13,207.013,206.897,206.782,206.652,206.522,206.392,206.261,
    206.131,206,205.848,205.714,205.581,205.447,205.314,205.181,205.047,204.914,
    204.911,204.778,204.644,204.511,204.378,204.245,204.112,203.976,203.843,203.71,
    203.577,203.444,203.312,203.179,203.047,203.044,203.044,203.044,203.044,202.115,
    202.115,202.115,202.115,202.115,202.115,202.115,201.191,201.191,201.191,201.191,
    201.191,201.191,201.191,201.191,200.271,200.271,200.271,200.271,200.271,200.271,
    200.271,199.354,199.354,199.354,199.354,199.354,199.354,199.354,199.354,198.442,
    198.442,198.442,198.442,198.442,198.442,198.442,197.534,197.534,197.534,197.534,
    197.534,197.534,197.534,197.534,196.63,196.63,196.63,196.63,196.63,196.63,
    196.63,195.73,195.73,195.73,195.73,195.73,195.73,195.73,195.73,194.834,
    194.834,194.834,194.834,194.834,194.834,194.834,193.942,193.942,193.942,193.942,
    193.942,193.942,193.942,193.942,193.054,193.054,193.054,193.054,193.054,193.054,
    193.054,192.17,192.17,192.17,192.17,192.17,192.17,192.17,192.17,191.289,
    191.289,191.289,191.289,191.289,191.289,191.289,190.413,190.413,190.413,190.413,
    190.413,190.413,190.413,190.413,189.541,189.541,189.541,189.541,189.541,189.541,
    189.541,188.672,188.672,188.672,188.672,188.672,188.672,188.672,188.672,187.807,
    187.807,187.807,187.807,187.807,187.807,187.807,186.946,186.946,186.946,186.946,
    186.946,186.946,186.946,186.946,186.089,186.089,186.089,186.089,186.089,186.089,
    186.089,185.236,185.236,185.236,185.236,185.236,185.236,185.236,185.236,184.387,
    184.387,184.387,184.387,184.387,184.387,184.387,183.541,183.541,183.541,183.541
  };
  TH1D *hLT;
  hLT = new TH1D("hLT","",600,0.0,2400);
  for (int ltIndex=0; ltIndex<600; ltIndex++) {
    hLT->SetBinContent(ltIndex+1,ltArray[ltIndex]);
  }
  return hLT;
}

