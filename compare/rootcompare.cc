
/*****************************************************************************
 *
 * 2014/11/17 Kei Moriya
 *
 * Program that compares histograms from different ROOT files.
 * Type "make" to compile. For user gluex, need to set ROOTSYS:
 * source /home/gluex/setup_jlab_commissioning.csh
 *
 * For 1D plots, they will all be plotted on top of each other
 * with different colors. Optionally, all histograms can be
 * normalized to have the same area.
 *
 * For 2D plots, the percentage difference between the histograms
 * in the first and second files will be shown.
 *
 * Usage:
 * rootcompare -o [outfile] -f "file1,file2,...fileN" -r
 * Options:
 * -o  :   Output ROOT file name
 * -f  :   List of file names. Each file should be separated by a ","
 * -r  :   Renormalize plots. For 1D histogram comparisons, all
 *         histograms will be normalized to have the same *area*
 *          as the histgoram from the first file.
 * -d  :   Spit out debug comments
 *
 * This file is a compiled and modified version of
 * rootCompare.C which was provided by Mike Staib.
 *
 *****************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <vector>
#include <string.h>

#include "TROOT.h"
#include "TFile.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TAttFill.h"
#include "TTree.h"
#include "TKey.h"
#include "Riostream.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TError.h"

TList *FileList;
TFile *Target;

void MergeRootfile( TDirectory *target, TList *sourcelist, Bool_t doRenormalize = false );
vector< string > stringSplit(const string& str, const string& delimiters );

//Set Rebin number, how many bins to merge together
// int rebin = 2;

// Counts how many unique canvases we create
int unique = 0;

// TStyle* gStyle;

const Int_t NMAX = 10;
const Int_t colors[NMAX] = {kBlack, kRed, kYellow-7,kGreen-9,kGreen+2, kCyan, kBlue-9, kBlue, kMagenta-9,kMagenta};
TH1 *h[NMAX];
char text[800];

char motheroutdir[800];
char command[800];
char canvasname[800];

Bool_t debug;

using namespace std;

int main(int argc, char **argv) {

  gErrorIgnoreLevel=kError;

  // Set up options
  std::string outfile = "";
  std::string filelist = "";
  Bool_t doRenormalize = false;
  debug = false;

  char *progName = argv[0];
  extern char* optarg;
  // Check command line arguments                                                                                       
  int c;
  while((c = getopt(argc,argv,"ho:f:rd")) != -1){
    switch(c){
    case 'h':
      cout << "rootcompare" << endl;
      cout << "Usage:" << endl;
      cout << "rootcompare -o [outfile name] -f [file list (separated by ,)] -r (do renorm)" << endl;
      return 0;
      break;
    case 'o':
      // Specify outfile name.                                                                                           
      outfile = optarg;
      cout << "outfile name: " << outfile << endl;
      break;
    case 'f':
      // Specify infile list.                                                                                           
      filelist = optarg;
      cout << "filelist: " << filelist << endl;
      break;
    case 'r':
      // Do renormalize to 1st TH1
      doRenormalize = true;
      cout << "will renormalize TH1 plots to 0th file" << endl;
      break;
    case 'd':
      // Debug
      debug = true;
      cout << "will spit out debug comments" << endl;
      break;
    default:
      break;
    }
  }

  gStyle->SetOptStat(0);

  if(outfile=="" || filelist==""){
    cout << "Usage:" << endl
	 << "rootcompare -o [outfile name], -f [string with list of files separated by \",\"]" << endl;
    abort();
  }

  Target = TFile::Open( outfile.c_str(), "RECREATE" );
  FileList = new TList();

  // Decompose filelist into list of files
  // First, remove all spaces
  Bool_t found = true;
  while(found){
    size_t pos = filelist.find(" ");
    if(pos!=std::string::npos){
      filelist.replace(pos,1,"");
    }else{
      found = false;
    }
  }
  // Separate files by commas
  // This is done by the function stringSplit
  std::vector< std::string> files = stringSplit(filelist,",");
  for(Int_t i=0;i<files.size();i++){
    cout << "file " << setw(3) << i << " is " << files[i] << endl;
    FileList->Add( TFile::Open(files[i].c_str()) );
  }
  cout << "-----------------" << endl;

  // Create output dir for figurs
  system("echo $PWD > tmp___PWD.txt");
  ifstream pwdfile("tmp___PWD.txt");
  char pwd[800];
  pwdfile >> pwd;
  system("rm -f tmp___PWD.txt");
  sprintf(motheroutdir,"%s/figures/%s",pwd,filelist.c_str());
  sprintf(command,"mkdir -p %s",motheroutdir);
  system(command);

  // Compare runs
  MergeRootfile( Target, FileList, doRenormalize );

  return 0;
}
//_______________________________________________________________________________________________

void MergeRootfile( TDirectory *target, TList *sourcelist, Bool_t doRenormalize ) {
  gROOT->SetBatch();

  // strstr(char *s1, char *s2) returns the
  // first occurence of s2 in s1
  TString path( (char*)strstr( target->GetPath(), ":" ) );
  if(debug){
    cout << "Target path: " << target->GetPath() << endl;
    cout << "strstr result = " << strstr( target->GetPath(), ":" ) << endl;
     cout << "start of MergeRootfile, path = " << path << endl;
  }
  path.Remove( 0, 2 );

  //int unique = 0;
  TFile *first_source = (TFile*)sourcelist->First();

  if(debug) cout << "Within first_source, cd into : " << path << endl;
  first_source->cd( path );
  TDirectory *current_sourcedir = gDirectory;

  // do not add the objects in the list in memory
  Bool_t status = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);

  // loop over all keys in this directory
  TChain *globChain = 0;
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key, *oldkey=0;

  TCanvas * c1;

  /////////////////////////////////////////////////////
  //                                                 //
  //          Start of looping over keys             //
  //                                                 //
  /////////////////////////////////////////////////////
  while ( (key = (TKey*)nextkey())) {

    //keep only the highest cycle number for each key
    if (oldkey && !strcmp(oldkey->GetName(),key->GetName())) continue;

    // read object from first source file
    first_source->cd( path );
    TObject *obj = key->ReadObj();

    // skip tagm for now, takes too much time
    if(path.Contains("tagm")){
      cout << "skipping tagm" << endl;
      continue;
    }
    if(path.Contains("CDC_expert")){
      cout << "skipping CDC_expert" << endl;
      continue;
    }

    // Since we just want to draw the two histograms to the same canvas
    TString objName = obj->GetName();
    if(debug){
      cout << "entering " << objName.Data() << endl;
      cout << "unique = " << unique << endl;
    }

    // Create new canvas to save all hists of same name
    c1 = new TCanvas(Form("%s_%i", objName.Data(),unique), objName.Data(), 1600, 800);

    /////////////////////////////////////////////////////
    //                                                 //
    //            Start of is TH1 object               //
    //                                                 //
    /////////////////////////////////////////////////////
    if ( obj->IsA()->InheritsFrom( TH1::Class() ) ) {
      // descendant of TH1

      /////////////////////////////////////////////////////
      //                                                 //
      //            Start of is TH2 object               //
      //                                                 //
      /////////////////////////////////////////////////////
      if ( obj->IsA()->InheritsFrom( TH2::Class() ) ) {
	if(debug) cout << "Entering TH2" << endl;

	c1->cd();
	c1->SetCanvasSize(1600,800);

	// Skip NumThrown
	TH2 *h11 = (TH2*)obj;
	TString name = h11->GetName();
	if(debug) cout << "Entering " << name.Data() <<endl;
	if (!name.Contains("NumThrown")){
	  Int_t nbinsx = h11->GetNbinsX();
	  Int_t nbinsy = h11->GetNbinsY();

	  /*	  
	  std::vector<Float_t>values;
	  for(Int_t binx=0;binx<=h11->GetNbinsX()+1;binx++){
	    for(Int_t biny=0;biny<=h11->GetNbinsY()+1;biny++){
	      values.push_back(h11->GetBinContent(binx,biny));
	    }
	  }
	  */

	  //TKey *keyThrown = (TKey*)gDirectory->GetListOfKeys()->FindObject("NumThrown");
	  //TH2 *thrown = (TH2*) keyThrown->ReadObj();
	 
	  TH2 *hnew = (TH2*)h11->Clone("hnew");
	  hnew->Reset("M");
	  // loop over all source files and add the content of the
	  // correspondant histogram to the one pointed to by "h1"
	  TFile *nextsource = (TFile*)sourcelist->After( first_source );

	  // std::vector<std::vector <Float_t> >values_next;
	  // std::vector<Float_t> values_this;
	  Int_t nfiles = 0;
	  while ( nextsource ) {
	    // make sure we are at the correct directory level by cd'ing to path
	    nextsource->cd( path );
	    TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h11->GetName());
	    if(debug) cout << "Name: " << h11->GetName() << " key = " << key2 << endl ;
	    if (key2) {
	      
	      TH2 *h22 = (TH2*)key2->ReadObj();

	      // Normalize total area of each hist to 1
	      h11->Scale(1. / h11->Integral());
	      h22->Scale(1. / h22->Integral());

	      for(int i = 0; i <= nbinsx+1; i++){
		for(int j = 0; j <= nbinsy+1; j++){
		  if ( h11->GetBinContent(i,j) != 0 && h22->GetBinContent(i,j) != 0){
                
		    // values_this.push_back(h22->GetBinContent(i,j));

		    Double_t percentDiff = (h22->GetBinContent(i,j) - h11->GetBinContent(i,j)) / h11->GetBinContent(i,j) * 100;
		    hnew->SetBinContent(i,j,percentDiff);
		
                    //Double_t ratio = h22->GetBinContent(i,j) / h11->GetBinContent(i,j);
		    //hnew->SetBinContent(i,j,ratio);
		  }
		  // In case the value is zero, set to -Inf to clear the bin
		  else {hnew->SetBinContent(i,j,-1.0/0.0);}
		}
	      }
	      // if(values_this.size()>0) values_next.push_back(values_this);

	      //hnew->Add(h22, -1);
	      //hnew->Scale(-1);
	      //hnew->Divide(h22);
	      //h22->Divide(h11);
	      //h1->Draw();
	      hnew->SetMinimum(-100);
	      hnew->SetMaximum(100);
	      sprintf(text,"Percent Diff. %s %s",first_source->GetName(),nextsource->GetName());
	      hnew->SetName(text);
	      hnew->Draw("colz");

	      // create output dir
	      // If variable path contains parentheses, change to "_"
	      TString path_copy = path;
	      path_copy.ReplaceAll(" ","");
	      path_copy.ReplaceAll("(","_");
	      path_copy.ReplaceAll(")","_");
	      sprintf(command,"mkdir -p %s/%s",motheroutdir, path_copy.Data());
	      if(debug) cout << "creating dir " << command << endl;
	      system(command);

	      TString objName_copy = objName;
	      objName_copy.ReplaceAll(" ","");
	      objName_copy.ReplaceAll("[","_");
	      objName_copy.ReplaceAll("]","_");
	      sprintf(canvasname,"%s/%s/%3.3i_%s.pdf", motheroutdir, path_copy.Data(), unique, objName_copy.Data());
	      if(debug) cout << "creating file: " << canvasname << endl;
	      c1->SaveAs(canvasname);
	      //Not sure this is needed for this case
	      //delete h2;

	      // Can only do comparison with 1st and 2nd file
	      break;
	    }

	    /*
	    // Compare bin contents
	    for(Int_t i=0;i<1;i++){
	      for(Int_t bin=0;bin<values.size();bin++){
		if(values_next.size() > 0){
		  if(fabs(values[bin] - values_next[0][bin]) > 1.e-5){
		    // cout << "found difference for " << unique << " " << h11->GetName() << endl;
		    // cout << "for bin " << setw(3) << bin << " first = " << values[bin] << " next = " << values_next[i][bin] << endl;

		    for(Int_t ii=0;ii<1;ii++){
		      for(Int_t bin2=0;bin2<values.size();bin2++){
			// cout << "bin = " << setw(5) << bin2 << "old = " << setw(8) << values[bin2] << " new = " << setw(8) << values_next[0][bin2] << endl;
		      }
		    }
		    // abort();
		  }
		}
	      }
	    }
	    */

	    nextsource = (TFile*)sourcelist->After( nextsource );
	  } // end of while(nextsource)
	} // end of name does not contain NumThrown

	else {
	  cout << "Found TH2 with NumThrown" << endl;
	  cout << "hist name is " << h11->GetName() << endl;
          //TH2 *h11 = (TH2*)obj;
	  h11->Draw("colz");
	}
      }
      /////////////////////////////////////////////////////
      //                                                 //
      //             End of is TH2 object                //
      //                                                 //
      /////////////////////////////////////////////////////

      /////////////////////////////////////////////////////
      //                                                 //
      //           Start of NOT TH2 object               //
      //                                                 //
      /////////////////////////////////////////////////////
      else{
	if(debug){
	  cout << "starting here for " << objName.Data() << endl;
	  cout << "printing current dir:";
	  gDirectory->pwd();
	}

	c1->cd();
	c1->SetTopMargin(0.05);
	c1->SetRightMargin(0.02);
	c1->SetCanvasSize(1600,800);
	if(debug) cout << "Merging histogram " << obj->GetName() << endl;
	h[0] = (TH1*)obj;
	h[0]->SetMinimum(0);
	if(debug){
	  cout << h[0]->GetName() << endl;
	  cout << "counts: " << h[0]->GetEntries() << endl;
	  cout << "limits: " << h[0]->GetXaxis()->GetXmin() << " " << h[0]->GetXaxis()->GetXmax() << endl;
	  cout << "minmax: " << h[0]->GetMinimum() << " " << h[0]->GetMaximum() << endl;
	}

	TLegend * legend = new TLegend(0.20, 0.80, 0.90, 0.93, "", "brNDC");
	legend->SetFillColor(0);
	legend->SetNColumns(2);
	legend->SetFillStyle(0);
	legend->SetBorderSize(0);
	legend->SetX1NDC(0.20);
	legend->SetX2NDC(0.90);
	legend->SetY1NDC(0.80);
	legend->SetY2NDC(0.93);
	    
	Int_t color = 0;
	h[0]->SetFillStyle(0);
	h[0]->SetFillColor(colors[color]);
	h[0]->SetLineColor(colors[color]);
	h[0]->SetLineWidth(2);
	h[0]->Draw();

	/*
	std::vector<Float_t>values;
	for(Int_t bin=0;bin<=h[0]->GetNbinsX()+1;bin++){
	  values.push_back(h[0]->GetBinContent(bin));
	}
	*/

	// create output dir
	// If variable path contains parentheses, change to "_"
	TString path_copy = path;
	path_copy.ReplaceAll(" ","");
	path_copy.ReplaceAll("(","_");
	path_copy.ReplaceAll(")","_");
	sprintf(command,"mkdir -p %s/%s",motheroutdir, path_copy.Data());
	if(debug) cout << "creating dir " << command << endl;
	system(command);
	TString objName_copy = objName;
	objName_copy.ReplaceAll(" ","");
	objName_copy.ReplaceAll("[","_");
	objName_copy.ReplaceAll("]","_");

	sprintf(text,"%s (%d)",first_source->GetName(), (Int_t)h[0]->GetEntries());
	legend->AddEntry(h[0], text, "lpf");

	color = 1;

	// std::vector<std::vector <Float_t> >values_next;

	// Start of loop over other source files
	TFile *nextsource = (TFile*)sourcelist->After( first_source );
	while ( nextsource ) {
	  if(debug) cout << "within " << nextsource->GetName() << endl;

	  // make sure we are at the correct directory level by cd'ing to path
	  nextsource->cd( path );
	  TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h[0]->GetName());
	  if (key2) {
	    h[color] = (TH1*)key2->ReadObj();

	    /*
	    std::vector<Float_t> values_this;
	    for(Int_t bin=0;bin<=h[0]->GetNbinsX()+1;bin++){
	      values_this.push_back(h[0]->GetBinContent(bin));
	    }
	    values_next.push_back(values_this);
	    */

	    // renormalize to area
	    if(doRenormalize==true){
	      // If first file has no entries, don't change anything
	      Double_t scale = h[0]->Integral() == 0 ? 1 : h[0]->Integral() / h[color]->Integral();
	      h[color]->Scale(scale);
	    }
	    h[color]->SetFillStyle(3001 + color);
	    h[color]->SetFillColor(colors[color]);
	    h[color]->SetLineColor(colors[color]);
	    h[color]->Draw("SAME");

	    sprintf(text,"%s (%d)",nextsource->GetName(), (Int_t)h[color]->GetEntries());
	    legend->AddEntry(h[color],text,"lpf");

	    color++;

	    //Not sure this is needed for this case
	    // delete h2;
	  }

	  if(color == NMAX){
	    cout << "Reached maximum number of repetitions..." << endl;
	    cout << "breaking..." << endl;
	    break;
	  }
	  nextsource = (TFile*)sourcelist->After( nextsource );
	} // end of loop over source files

	// Draw legend after looping over all files
	legend->Draw("SAME");

	/*
	// Compare bin contents
	for(Int_t i=0;i<1;i++){
	  for(Int_t bin=0;bin<values.size();bin++){
	    // cout << "for bin " << setw(3) << bin << " first = " << values[bin] << " next = " << values_next[i][bin] << endl;
	    if(fabs(values[bin] - values_next[0][bin]) > 1.e-5){
	      cout << "found difference for " << h[0]->GetName() << endl;
	      cout << "for bin " << setw(3) << bin << " first = " << values[bin] << " next = " << values_next[i][bin] << endl;
	      abort();
	    }
	  }
	}
	*/

	sprintf(canvasname,"%s/%s/%3.3i_%s.pdf", motheroutdir, path_copy.Data(), unique, objName_copy.Data());
	c1->SaveAs(canvasname);
	if(debug) cout << "creating file: " << canvasname << endl;
      }
      /////////////////////////////////////////////////////
      //                                                 //
      //            End of NOT TH2 object                //
      //                                                 //
      /////////////////////////////////////////////////////
    }
    /////////////////////////////////////////////////////
    //                                                 //
    //             End of is TH1 object                //
    //                                                 //
    /////////////////////////////////////////////////////
    
    /////////////////////////////////////////////////////
    //                                                 //
    //          Start of is TTree object               //
    //                                                 //
    /////////////////////////////////////////////////////
    else if ( obj->IsA()->InheritsFrom( TTree::Class() ) ) {

      // loop over all source files create a chain of Trees "globChain"
      const char* obj_name= obj->GetName();

      globChain = new TChain(obj_name);
      globChain->Add(first_source->GetName());
      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      //      const char* file_name = nextsource->GetName();
      // cout << "file name  " << file_name << endl;
      while ( nextsource ) {

	globChain->Add(nextsource->GetName());
	nextsource = (TFile*)sourcelist->After( nextsource );
      }

    }
    /////////////////////////////////////////////////////
    //                                                 //
    //            End of is TTree object               //
    //                                                 //
    /////////////////////////////////////////////////////

    /////////////////////////////////////////////////////
    //                                                 //
    //        Start of is TDirectory object            //
    //                                                 //
    /////////////////////////////////////////////////////
    else if ( obj->IsA()->InheritsFrom( TDirectory::Class() ) ) {
      // it's a subdirectory
      
      cout << "Found subdirectory " << obj->GetName() << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      MergeRootfile( newdir, sourcelist, doRenormalize );
      // cout << "finished and leaving " << obj->GetName() << endl;
    } 
    /////////////////////////////////////////////////////
    //                                                 //
    //         End of is TDirectory object             //
    //                                                 //
    /////////////////////////////////////////////////////
    
    /////////////////////////////////////////////////////
    //                                                 //
    //          Start of is unknown object             //
    //                                                 //
    /////////////////////////////////////////////////////
    else {

      // object is of no type that we know or can handle
      cout << "Unknown object type, name: "
	   << obj->GetName() << " title: " << obj->GetTitle() << endl;
    }
    /////////////////////////////////////////////////////
    //                                                 //
    //           End of is unknown object              //
    //                                                 //
    /////////////////////////////////////////////////////

    // now write the merged histogram (which is "in" obj) to the target file
    // note that this will just store obj in the current directory level,
    // which is not persistent until the complete directory itself is stored
    // by "target->Write()" below
    if ( obj ) {
      target->cd();
      
      //!!if the object is a tree, it is stored in globChain...
      if(obj->IsA()->InheritsFrom( TTree::Class() ))
	globChain->Merge(target->GetFile(),0,"keep");
      else if( obj->IsA()->InheritsFrom( TH1::Class() ) ) {
	//	TLegend * legend = new TLegend(0.70, 0.70, 0.95, 0.95, "", "brNDC");
	//legend->SetFillColor(0);
	//c1->cd();
	//legend->Draw();
	c1->Write( key->GetName() );
      }
      else{
	obj->Write( key->GetName() );
        //c1->Write( key->GetName() ); 
      }
      
    }
    unique++;
  } // while ( ( TKey *key = (TKey*)nextkey() ) )
  /////////////////////////////////////////////////////
  //                                                 //
  //           End of looping over keys              //
  //                                                 //
  /////////////////////////////////////////////////////

  // save modifications to target file
  target->SaveSelf(kTRUE);
  TH1::AddDirectory(status);

  gROOT->SetBatch(kFALSE);
} // end of MergeRootFile

vector< string > stringSplit(const string& str, const string& delimiters )
{
  vector< string > tokens;
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  string::size_type pos     = str.find_first_of(delimiters, lastPos);
  while (string::npos != pos || string::npos != lastPos){
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    lastPos = str.find_first_not_of(delimiters, pos);
    pos = str.find_first_of(delimiters, lastPos);
  }
  
  return tokens;
}
