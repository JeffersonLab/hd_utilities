{
  cout << "Loading Cosmetics" << endl;
  gStyle->SetFillColor(10);
  gStyle->SetCanvasColor(10);
  gStyle->SetPadColor(10);
  gStyle->SetFillStyle(1001);
  gStyle->SetCanvasBorderMode(0);

  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.5);

  //gStyle->SetPadColor(10);
  gStyle->SetPadLeftMargin(0.145);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetCanvasDefH(600.0);
  gStyle->SetCanvasDefW(600.0);

  gStyle->SetTitleOffset(1.5,"Y");
  
  const Int_t NRGBs = 5;
  const Int_t NCont = 255;
  
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);    
  gStyle->SetNumberContours(NCont);
  gStyle->SetTextColor(0);
    
  cout << "Loading Stats box with over/underflow shown" << endl;
  gStyle->SetOptStat(111111); 
  cout << "Loading Fit box with over/underflow shown" << endl;
  gStyle->SetOptFit(1110);
  cout << "Loading RootMacros/FCALUtilities/StringUtilities.C..." << endl;
  gROOT->LoadMacro("RootMacros/FCALUtilities/StringUtilities.C+");
  cout << "Loading RootMacros/FCALUtilities/GeneralUtilities.C..." << endl;
  gROOT->LoadMacro("RootMacros/FCALUtilities/GeneralUtilities.C+");
  cout << "Loading RootMacros/FCALUtilities/PlotUtilities.C..." << endl;
  gROOT->LoadMacro("RootMacros/FCALUtilities/PlotUtilities.C+");
  cout << "Loading RootMacros/FCALUtilities/DAQUtilities.C..." << endl;
  gROOT->LoadMacro("RootMacros/FCALUtilities/DAQUtilities.C+");
  cout << "Loading RootMacros/FCALUtilities/MonitoringUtilities.C..." << endl;
  gROOT->LoadMacro("RootMacros/FCALUtilities/MonitoringUtilities.C+");
}
