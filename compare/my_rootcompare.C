
/*****************************************************************************
 *
 * 2014/10/30 Kei Moriya
 *
 * ROOT macro that merges histograms from different ROOT files.
 *
 * Usage:
 * root 'my_rootcompare.C("output.root", "file1,file2,...fileN")'
 * 
 * Use strings to specify output file name, and a chain of
 * files separated by commas. Each TH1 hist (up to 10) will
 * be drawn to a canvas with different colors.
 *
 *****************************************************************************/

#include <string.h>
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TAttFill.h"
#include "TTree.h"
#include "TKey.h"
#include "Riostream.h"

TList *FileList;
TFile *Target;

void MergeRootfile( TDirectory *target, TList *sourcelist );

//Set Rebin number, how many bins to merge together
// int rebin = 2;

// Counts how many unique canvases we create
int unique = 0;

gStyle->SetOptStat(0);
gErrorIgnoreLevel=kError;

const Int_t NMAX = 10;
const Int_t colors[NMAX] = {kBlack, kRed, kYellow-7,kGreen-9,kGreen+2, kCyan, kBlue-9, kBlue, kMagenta-9,kMagenta};
TH1 *h[NMAX];
char text[400];

void my_rootcompare(std::string outfile = "", std::string filelist = "") {

  if(outfile=="" || filelist==""){
    cout << "Usage:" << endl
	 << "root my_rootcompare.C([outfile name], [string with list of files separated by \",\"])" << endl;
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
    cout << "file is " << files[i] << endl;
    FileList->Add( TFile::Open(files[i].c_str()) );
  }
  MergeRootfile( Target, FileList );
}

void MergeRootfile( TDirectory *target, TList *sourcelist ) {
  gROOT->SetBatch();
  // cout << "Target path: " << target->GetPath() << endl;

  // strstr(char *s1, char *s2) returns the
  // first occurence of s2 in s1
  // cout << "target->GetPath() = " << target->GetPath() << endl;
  // cout << "strstr result = " << strstr( target->GetPath(), ":" ) << endl;
  TString path( (char*)strstr( target->GetPath(), ":" ) );
  // cout << "path = " << path << endl;
  path.Remove( 0, 2 );

  //int unique = 0;
  TFile *first_source = (TFile*)sourcelist->First();

  // cout << "Within first_source, cd into : " << path << endl;
  first_source->cd( path );
  TDirectory *current_sourcedir = gDirectory;

  //gain time, do not add the objects in the list in memory
  Bool_t status = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);

  // loop over all keys in this directory
  TChain *globChain = 0;
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key, *oldkey=0;

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

    // Since we just want to draw the two histograms to the same canvas
    TString objName = obj->GetName();

    // Create new canvas to save all hists of same name
    TCanvas * c1 = new TCanvas(Form("%s_%i", objName.Data(),unique), objName, 800, 600);

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
	//cout << "Entering TH2" << endl;

	// Skip NumThrown
	TH2 *h11 = (TH2*)obj;
	TString name = h11->GetName();
	// cout << "Entering " << name.Data() <<endl;
	if (!name.Contains("NumThrown")){
	  //cout << h11->GetName() << " does not equal NumThrown" << endl ;
         
	  Int_t nbinsx = h11->GetNbinsX();
	  Int_t nbinsy = h11->GetNbinsY();

	  //TKey *keyThrown = (TKey*)gDirectory->GetListOfKeys()->FindObject("NumThrown");
	  //TH2 *thrown = (TH2*) keyThrown->ReadObj();
	 
	  TH2 *hnew = (TH2*)h11->Clone("hnew");
	  hnew->Reset("M");
	  // loop over all source files and add the content of the
	  // correspondant histogram to the one pointed to by "h1"
	  TFile *nextsource = (TFile*)sourcelist->After( first_source );
	  while ( nextsource ) {
	    // make sure we are at the correct directory level by cd'ing to path
	    nextsource->cd( path );
	    TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h11->GetName());
	    //cout << "Name: " << h11->GetName() << " key = " << key2 << endl ;
	    if (key2) {
	      
	      TH2 *h22 = (TH2*)key2->ReadObj();
	      for(int i = 1; i <= nbinsx; i++){
		for(int j = 1; j <= nbinsy; j++){
		  //if (thrown->GetBinContent(i,j) > 10){
		  if ( h11->GetBinContent(i,j) != 0 && h22->GetBinContent(i,j) != 0){
                
		    Double_t percentDiff = (h22->GetBinContent(i,j) - h11->GetBinContent(i,j)) / h11->GetBinContent(i,j) * 100;
		    hnew->SetBinContent(i,j,percentDiff);
		
                    //Double_t ratio = h22->GetBinContent(i,j) / h11->GetBinContent(i,j);
		    //hnew->SetBinContent(i,j,ratio);
		  }
		  // In case the value is zero, set to -Inf to clear the bin
		  else {hnew->SetBinContent(i,j,-1.0/0.0);}
		  //}
		}
	      }
	      //hnew->Add(h22, -1);
	      //hnew->Scale(-1);
	      //hnew->Divide(h22);
	      //h22->Divide(h11);
	      //h1->Draw();
	      hnew->SetMinimum(-100);
	      hnew->SetMaximum(100);
	      hnew->SetName("Percent Diff. (1200-1500)/1500");
	      //hnew->SetName("Ratio 1200A/1500A");
	      hnew->Draw("colz");

	      //Not sure this is needed for this case
	      //delete h2;
	    }
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
	c1->SetTopMargin(0.05);
	c1->SetRightMargin(0.02);
	//      cout << "Merging histogram " << obj->GetName() << endl;
	h[0] = (TH1*)obj;
	// h[0]->Rebin(rebin, "", 0);
	// loop over all source files and add the content of the
	// correspondant histogram to the one pointed to by "h[0]"
	TFile *nextsource = (TFile*)sourcelist->After( first_source );

	TLegend * legend = new TLegend(0.20, 0.80, 0.90, 0.93, "", "brNDC");
	legend->SetFillColor(0);
	legend->SetNColumns(2);
	legend->SetFillStyle(0);
	c1->cd();
	    
	Int_t color = 1;
	h[0]->SetFillStyle(3001 + color);
	h[0]->SetFillColor(colors[color]);
	h[0]->SetLineColor(colors[color]);
	h[0]->Draw();
	sprintf(text,"%s (%d)",nextsource->GetName(), (Int_t)h[0]->GetEntries());
	legend->AddEntry(h[0], text, "lpf");

	// Start of loop over source files
	while ( nextsource ) {
	  // make sure we are at the correct directory level by cd'ing to path
	  nextsource->cd( path );
	  TKey *key2 = (TKey*)gDirectory->GetListOfKeys()->FindObject(h[0]->GetName());
	  if (key2) {
	    h[color] = (TH1*)key2->ReadObj();
	    // h[color]->Rebin(rebin,"",0);
	    
	    h[color]->Draw("SAME");
	    sprintf(text,"%s (%d)",nextsource->GetName(), (Int_t)h[color]->GetEntries());
	    legend->AddEntry(h[color],text,"lpf");

	    //Not sure this is needed for this case
	    // delete h2;
	  }

	  color++;
	  if(color == NMAX){
	    cout << "Reached maximum number of repetitions..." << endl;
	    cout << "breaking..." << endl;
	    break;
	  }
	  nextsource = (TFile*)sourcelist->After( nextsource );
	} // end of loop over source files

	// Draw legend after looping over all files
	legend->Draw("SAME");
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
      MergeRootfile( newdir, sourcelist );

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

vector< string > stringSplit(const string& str, const string& delimiters ) const
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
