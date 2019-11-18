#if !defined(MONITORING_H)
#define MONITORING_H

#include "GeneralUtilities.h"

struct BaseStrand{
  BaseStrand(int a_AbsPos, int a_X, int a_Y, int a_Strand):absPos(a_AbsPos), X(a_X), Y(a_Y), Strand(a_Strand){}
  int absPos;
  int X;
  int Y;
  int Strand;
};

class MonitoringUtilities{
  public:
    
    static map<TString, TGraph*> Monitoring_adcVsTime(std::vector<ADC_Channel> adcList, string monitoringFile, int everyN, TString dir, string adc = "CAT, CUR, or TEM", bool plot = true, int j=0);
    
    static std::pair<int,int> CountDatesInFile(string fileName);
    
    static double DateToHours(TString date);
    
    // Reading Frome a text file and return a map of key= Crate Slot Channel and val= TGraph
    static std::vector<ADC_Channel> Monitoring_HV(string monitoringFile, int j=0);
    
    static vector<BaseStrand> MatchStrand();
    
    static std::pair<int,int> GetStrandPair(int strand, int Pos);
    
    static int GetStrand(int x, int y);
    
};
#endif