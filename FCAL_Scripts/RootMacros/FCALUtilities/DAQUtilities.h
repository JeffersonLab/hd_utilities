#if !defined(DAQ_H)
#define DAQL_H

#include "TTreeIndex.h"
#include "GeneralUtilities.h"



// Creat a container to hold the above histograms and TGraphs for each channel
struct Pedestal_Study{
  
  Pedestal_Study()  {}
  
  int crate;
  int slot;
  int channel;
  int x;
  int y;
  TString name;
  
  double spread;
  double spread_diff;
  
 
  double mean;
  int mean_max;
  int mean_min;
  double mean_diff;
  int mean_diff_max;
  int mean_diff_min;
  
  TGraphErrors* grE_mean_vs_event;
  TGraph* gr_mean_diff;
  
  TH1F* h_mean;
  TH1F* h_mean_diff;
  
  vector<TH1F*> h_ped_vec;

};

/*
Possible options in sortby:
nosort, channel, event, absnum, slot
*/

/*
  Run Modes:
  1: Raw ADC data samples
  2: Pulse Raw ADC data samples
  3: Pulse integral
  4: High-resolution time
  7: Pulse Integral + high-resolution time
  8: Raw ADC data samples + high-resolution time
*/

class DAQUtilities{
  public:
    

    static int first_event;
    static int last_event;
    
    static bool get_waveform;        
    static bool marks_timing;
    static bool get_coordinates;
    static bool custom_timing;

    static vector< ADC_Channel > sortADC(vector< ADC_Channel > adcList, TString sortby = "channel, event, slot, or absnum");

    static std::map<TString, TGraph*> adcValues_TGraph(TString fileName, int everyN, double pulser_freq, TString adc = "ped, max, charge, timing", bool compressed_mode = false, int threshold=100, int NSB=1, int NSA=1, bool manny_plugin=false);

    static std::vector< ADC_Channel > manny_waveform_loop(TString fileName, TString sortby);
    static std::vector< ADC_Channel > manny_plugin_loop(TString fileName, TString sortby);
    static void overflowFraction(TString fileName);
    static vector< Pedestal_Study >   manny_plugin_pedestalStudy(TString fileName, int nEvtsAvg=100);

    static std::vector< ADC_Channel > main_loop_compressedMode(TString fileName, TString sortby = "channel, event, slot, or absnum");

    static std::vector< ADC_Channel > main_loop(TString fileName, TString sortby = "channel, event, slot, or absnum", \
    bool mark_PedTimeCalc = true, bool debug = false, bool reduce = false, int threshold=130, int NSB=1, int NSA=2, bool compute_integral = false);

    static int calcTiming(std::vector<uint32_t> waveform, int rocid, int slot, int channel, bool debug);

    static double calcPedestal(std::vector<uint32_t> waveform, int rocid, int slot, int channel, bool debug);

    static double calcEnergySum(std::vector<uint32_t> waveform, int threshold, int NSB, int NSA, int PEDESTAL=0);

    static double calcTriggerPeak(std::vector<uint32_t> waveform, int threshold, int NSB, int NSA, int PEDESTAL=0);

    static void getCosmics(TString fileName, double triggerCharge, int nhitscut, int nColumns, bool ped_time);

    static std::vector< ADC_Channel > getMeanPedestal(std::vector< ADC_Channel > vec_adc);

    static void calibrateDAC(TString dac3200, TString dac3300, TString dir, TString version);
    
    //added by Adesh
    static std::vector< ADC_Channel > main_loop_intvspeak(TString fileName, TString sortby = "channel, event, slot, or absnum", \
    bool mark_PedTimeCalc = true, bool debug = false, bool reduce = false, int threshold=130, int NSB=1, int NSA=2, bool compute_integral = false);
    //

};
#endif
