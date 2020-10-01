#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <utility>
#include <iostream>

#include "AMPTOOLS_DATAIO/ROOTDataReader.h"
#include "AMPTOOLS_DATAIO/ROOTDataWriter.h"

#include "TLorentzVector.h"

#include "TH1F.h"
using namespace std;

#define DEFTREENAME "kin"

void Usage()
{
  cout << "Usage:\n  split_mass <infile> <outputBase> <lowMass> <highMass> <nBins> <lowtMass> <hightMass> <ntBins> [maxEvents]\n";
  cout << "  split_mass <infile> <outputBase> <lowMass> <highMass> <nBins> <lowtMass> <hightMass> <ntBins> -T [tree name]\n\n";
  cout << "   overwrites the default ROOT tree name (\"kin\") in output and/or input files\n";
  cout << "   To specify input and output names delimit with \':\' ex. -T inKin:outKin\n";
  cout << "   Use -t to update existing files with new tree, instead of overwritting.\n";
  exit(1);
}


pair <string,string> GetTreeNames(char* treeArg)
{
  pair <string,string> treeNames(DEFTREENAME,"");
  string treeArgStr(treeArg);
  size_t delimPos=treeArgStr.find(':',1);

  if (delimPos != string::npos){
    treeNames.first=treeArgStr.substr(0,delimPos);
    treeNames.second=treeArgStr.substr(delimPos+1);
  }else
    treeNames.second=treeArgStr;

  return treeNames;
}


int main( int argc, char* argv[] ){
  
  unsigned int maxEvents = 4294967000; //close to 4byte int range
  
  //string treeName( "kin" );
  pair <string,string> treeNames(DEFTREENAME,DEFTREENAME);

  bool recreate=true;

  if( argc < 9 ) Usage();
  
  string outBase( argv[2] );
  
  double lowMass = atof( argv[3] );
  double highMass = atof( argv[4] );
  int numBins = atoi( argv[5] );
  double lowt = atof( argv[6] );
  double hight = atof( argv[7] );
  int numtBins = atoi( argv[8] );
  
  // A somewhat convoluted way to allow tree name specification
  // via "-t [name]" in the arg. list after the standard args
  if( argc > 9 ) {
    for(int i=9; i<=10 && i<argc ; ++i){
      string arg=argv[i];
      if (arg == "-t"){
	if ((i+1 == argc) || (argv[i+1][0] == '-')) Usage();
	else{
	  treeNames = GetTreeNames(argv[++i]);
	  recreate=false;
	}
      }else if (arg == "-T"){
	if ((i+1 == argc) || (argv[i+1][0] == '-')) Usage();
	else{
	  treeNames = GetTreeNames(argv[++i]);
	  recreate=true;
	}
      }else
	if(i==9) maxEvents = atoi( arg.c_str() );
	else Usage();
    }
  }

  
  vector< string > dataReaderArgs;
  dataReaderArgs.push_back( argv[1] );
  dataReaderArgs.push_back( treeNames.first );
  
  // open reader
  ROOTDataReader in( dataReaderArgs );
  
  enum { kMaxBins = 1000 };
  assert( numBins < kMaxBins );
  assert( numtBins < kMaxBins );
  
  double step = ( highMass - lowMass ) / numBins;
  double stept = ( hight - lowt ) / numtBins;
  
  //ORIGINAL
  //  ROOTDataWriter* outFile[kMaxBins][kMaxBins];
  //First try  
  //ROOTDataWriter*** outFile= new (ROOTDataWriter**)[kMaxBins];
  //for( int i = 0; i < numBins; ++i ){
  //outFile[i]=new (OOTDataWriter*)[kMaxBins];  
  //}
  //ORIGINAL                                                                                                                                                                                
   ROOTDataWriter* outFile[kMaxBins*kMaxBins]; 

  for( int i = 0; i < numBins; ++i ){
  for( int j = 0; j < numtBins; ++j ){
    
    ostringstream outName;
    outName << outBase << "_" << i<< "_" << j << ".root";
    outFile[i*numtBins+j+1] = new ROOTDataWriter( outName.str(),
				     treeNames.second.c_str(), 
				     recreate, in.hasWeight());
  }
  }
  unsigned int eventCount = 0;
  
  Kinematics* event;
  while( ( event = in.getEvent() ) != NULL && eventCount++ < maxEvents ){
    
    vector< TLorentzVector > fs = event->particleList();
    
  
   // the first entry in this list is the beam
    TLorentzVector Beam(fs[0]);
    // the first two entries in this list are the beam and the recoil
    // skip them in computing the mass

  TLorentzVector x;

    for( vector< TLorentzVector >::iterator particle = fs.begin() + 2;
	 particle != fs.end(); ++particle ){
      
      x += *particle;
    }

  double t = -1 * (Beam - x).M2();
    
    int bin = static_cast< int >( floor( ( x.M() - lowMass ) / step ) );
    int bint = static_cast< int >( floor( ( t - lowt ) / stept ) );
    if( ( bin < numBins ) && ( bin >= 0 ) && ( bint < numtBins ) && ( bint >= 0 )){

      //cout<<"  bin*numtBins+bint+1 =  "<< bin*numtBins+bint+1<<" bin=  "<<bin<<" bint= "<<bint<<" numtBins=    "<<numtBins<<"   numMbins"<<numBins<<endl;      
      outFile[bin*numtBins+bint+1]->writeEvent( *event );
      delete event;

    }
  }
  
  for( int i = 0; i < numBins; ++i ){
    for( int j = 0; j < numtBins; ++j ){
    
      delete outFile[i*numtBins+j+1];
  }
  }

  return 0;
}
