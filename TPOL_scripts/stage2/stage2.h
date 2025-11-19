struct tpolHitToGo_t {
  UInt_t   nHits;
  UInt_t   nHitsTot;
  UInt_t   inTime[32];
  Double_t eDep[32];
  Double_t time[32];
  UInt_t   sector[32];
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

void getBranchesT1(TTree *inTree, tpolHitToGo_t *tpol0){
  inTree->SetBranchAddress("nHits",&tpol0->nHits);
  inTree->SetBranchAddress("nHitsTot",&tpol0->nHitsTot);
  inTree->SetBranchAddress("inTime",tpol0->inTime);
  inTree->SetBranchAddress("eDep",tpol0->eDep);
  inTree->SetBranchAddress("time",tpol0->time);
  inTree->SetBranchAddress("sector",tpol0->sector);
  inTree->SetBranchAddress("phi",tpol0->phi);
  inTree->SetBranchAddress("ePair",&tpol0->ePair);
  inTree->SetBranchAddress("deltaE",&tpol0->deltaE);
  inTree->SetBranchAddress("E_lhit",&tpol0->E_lhit);
  inTree->SetBranchAddress("E_rhit",&tpol0->E_rhit);
  inTree->SetBranchAddress("t_lhit",&tpol0->t_lhit);
  inTree->SetBranchAddress("t_rhit",&tpol0->t_rhit);
  inTree->SetBranchAddress("nGammaIn",&tpol0->nGammaIn);
  inTree->SetBranchAddress("nGammaOut",&tpol0->nGammaOut);
  inTree->SetBranchAddress("eGammaIn",tpol0->eGammaIn);
  inTree->SetBranchAddress("eGammaOut",tpol0->eGammaOut);
}

bool psCut(double eVal,double deltaE,int yVal){
  bool psCutVal = false;
  double mVal1 = 1.0;
  double bVal1 = -6.6;
  double mVal2 = -1.0;
  double bVal2 = 13.6;
  if (yVal == 2021) {
    bVal1 = -5.8;
    bVal2 =  12;
  }
  if (yVal == 2022) {
    bVal1 = -4;
    bVal2 =  8;
  }
  double yVal1 = mVal1*eVal + bVal1;
  double yVal2 = mVal2*eVal + bVal2;
  //std::cout<<"yVal = "<<yVal<<std::endl;
  if (fabs(deltaE) > yVal1) psCutVal = true;
  if (fabs(deltaE) > yVal2) psCutVal = true;
  return psCutVal;
}

class  MyPhiFitFun {
 public:
  double Evaluate(double *phiPtr, double *parPtr) {
    double fitVal = 0.0;
    double x  = *phiPtr;
    double p0 = parPtr[0];
    double p1 = parPtr[1];
    double p2 = parPtr[2];
    double p3 = parPtr[3];
    double p4 = parPtr[4];
    double degToRad = 3.14159/180.0;

    // [0]*(1-[1]*cos(2*(x-[2])*3.14/180))+[3]*[0]*cos((x-[4])*3.14/180)
    // [0]*(1-[1]*0.108*cos(2*(x-[2])*3.14/180))+[3]*[0]*cos((x-[4])*3.14/180)

    
    fitVal = p0*(1 - p1*cos(2*(x-p2)*degToRad)) + 
      p3*p0*cos((x-p4)*degToRad); 
    return fitVal;
  }
};

double eCorrect(int sector){
  double mpvVec[32];
  
  mpvVec[0] = 0.995601;
  mpvVec[1] = 1.01343;
  mpvVec[2] = 0.995601;
  mpvVec[3] = 0.995601;
  mpvVec[4] = 1.01343;
  mpvVec[5] = 1.00147;
  mpvVec[6] = 0.989796;
  mpvVec[7] = 0.989796;
  mpvVec[8] = 1.01343;
  mpvVec[9] = 1.05108;
  mpvVec[10] = 1.01343;
  mpvVec[11] = 1.01343;
  mpvVec[12] = 0.989796;
  mpvVec[13] = 1.00742;
  mpvVec[14] = 0.984058;
  mpvVec[15] = 1.00147;
  mpvVec[16] = 1.00742;
  mpvVec[17] = 1.00147;
  mpvVec[18] = 1.00147;
  mpvVec[19] = 1.00742;
  mpvVec[20] = 0.989796;
  mpvVec[21] = 1.00147;
  mpvVec[22] = 1.00147;
  mpvVec[23] = 1.00742;
  mpvVec[24] = 0.972779;
  mpvVec[25] = 0.967236;
  mpvVec[26] = 0.989796;
  mpvVec[27] = 0.984058;
  mpvVec[28] = 0.995601;
  mpvVec[29] = 0.995601;
  mpvVec[30] = 1.01343;
  mpvVec[31] = 1.00147;

  double retVal = mpvVec[sector-1];
  return retVal;
}

class  MyEnhFun {
 public:
  double Evaluate(double *xPtr, double *parPtr) {
    double fitVal = 0.0;
    double xVal = *xPtr;

    double p0 = parPtr[0];
    double p1 = parPtr[1] + parPtr[2]*(xVal-5.1);
    
    double sCenter = parPtr[3];
    double sWidth = parPtr[4];

    //double sFun = 1.0/(TMath::Exp((xVal - sCenter)*sWidth) + 1);
    double sFun = 1.0/(exp((xVal - sCenter)*sWidth) + 1);

    fitVal = p0 + p1*sFun;

    return fitVal;
  }
};


