#if !defined(PLOT_H)
#define PLOT_H

#include "GeneralUtilities.h"


class PlotUtilities{
  public:
    
    static bool getFullList;
    static vector<TString> bad_adcList;
    
    static void Clear_bad_adcList();
    
    static void PlotAllHist_OLD(TString fileName, std::map<TString,TH1F*> hist_map, double xmin, double xmax, bool plot, bool fit, TString dir, TString xtitle, TString ytitle);
    
    static void PlotAllGraphs_OLD(TString fileName, std::map<TString,TGraph*> gr_map, double xmin, double xmax, bool plot, TString dir, TString xtitle, TString ytitle);
    
    static void PlotAllGraphs(TString fileName, std::map<TString,TGraph*> gr_map, bool plot, TString dir, TString xtitle, TString ytitle, double xmin, double xmax, TString adc_type);
    
    static void PlotSingleGraphs(TString fileName, std::map<TString,TGraph*> gr_map, double xmin, double xmax, bool plot, TString dir, TString xtitle, TString ytitle);
    
    static void Plot_8_UniqueChannelsPerCanvas(TString fileName, std::map<TString,TGraph*> gr_map1, std::map<TString,TGraph*> gr_map2, bool plot, TString dir);
    
    static void Plot_4_UniqueChannelsPerCanvas(TString fileName, std::vector<TString> adcList, std::map<TString,TGraph*> gr_map1, std::map<TString,TGraph*> gr_map2, std::map<TString,TGraph*> gr_map3, std::map<TString,TGraph*> gr_map4, TString dir, int n,  TString* replaced_array);
    
    static void Plot_MultiRuns_4Rows(TString fileName, std::vector<TString> adcList, \
    std::map< TString, std::map<TString,TGraph*> > gr_map1, \
    std::map< TString, std::map<TString,TGraph*> > gr_map2, \
    std::map< TString, std::map<TString,TGraph*> > gr_map3, \
    std::map< TString, std::map<TString,TGraph*> > gr_map4, TString dir);
    
    static void Plot_MultiRuns_6Rows(TString fileName, std::vector<TString> adcList, \
    std::map< TString, std::map<TString,TGraph*> > gr_map1, \
    std::map< TString, std::map<TString,TGraph*> > gr_map2, \
    std::map< TString, std::map<TString,TGraph*> > gr_map3, \
    std::map< TString, std::map<TString,TGraph*> > gr_map4, \
    std::map< TString, std::map<TString,TGraph*> > gr_map5, \
    std::map< TString, std::map<TString,TGraph*> > gr_map6, TString dir);
    
    static TH2F* Plot2D(std::map< std::pair<double,double>, double> detector_map, double xmin=-29.5, double xmax=29.5, double ymin=-29.5, double ymax=29.5, double zmin=0, double zmax=-1);
    static TH2F* Plot2D(std::map< std::pair<int,int>, double> detector_map, double xmin=-29.5, double xmax=29.5, double ymin=-29.5, double ymax=29.5, double zmin=0, double zmax=-1);
    
};
#endif
