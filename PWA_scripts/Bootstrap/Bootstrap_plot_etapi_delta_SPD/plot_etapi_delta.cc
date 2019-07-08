#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <unistd.h>

#include "IUAmpTools/FitResults.h"

using namespace std;

int main( int argc, char* argv[] ){
    
    // these params should probably come in on the command line
    double lowMass = 0.8;
    double highMass = 2.0;
    enum{ kNumBins = 60 };
    enum{ kNumBins_Bootstrap = 50};

    string fitDir( "EtaPi_fit/" );
    
    // set default parameters
    
    string outfileName("");
    
    // parse command line
    
    for (int i = 1; i < argc; i++){
        
        string arg(argv[i]);
        
        if (arg == "-o"){
            if ((i+1 == argc) || (argv[i+1][0] == '-')) arg = "-h";
            else  outfileName = argv[++i]; }
        
        if (arg == "-h"){
            cout << endl << " Usage for: " << argv[0] << endl << endl;
            cout << "\t -o <file>\t Ouput text file" << endl;
            exit(1);}
        
        
    }
    
    if (outfileName.size() == 0){
        cout << "No output file specified" << endl;
        exit(1);
    }
    ofstream outfile[kNumBins];
    // descend into the directory that contains the bins
    chdir( fitDir.c_str() );
    for (int j=0; j<kNumBins; j++) {
        ostringstream dir;
        dir << "bin_" << j;
        chdir( dir.str().c_str() );
        cout<<"Getting results from bin "<<j<<endl;

        //ofstream outfile;
        //cout<<"File opening..."<<endl;
        outfile[j].open( outfileName.c_str() );
        //cout<<"Opened file"<<endl;
        for( int i = 0; i < kNumBins_Bootstrap; ++i ){
            cout<<"Results from bootstrap "<<i<<endl;
            
            ostringstream resultsFile;
	    resultsFile.str("");
            //cout<<"Created resultsFile"<<endl;
	    resultsFile << "bin_bs_" << i << ".fit";
            //cout<<"Naming results file"<<endl;
            FitResults results( resultsFile.str().c_str() );
            //cout<<"Named results file"<<endl;
            // if( !results.valid() ){
             
             //chdir( ".." );
             //continue;
             //}
             
            // print out the bin center
            double step = ( highMass - lowMass ) / kNumBins;
            outfile[j] << lowMass + step * j + step / 2. << "\t";
            
	    //cout<<"S"<<endl;
            vector<string> etaPiS0;
            etaPiS0.push_back("EtaPi::Negative::S0-");
            pair< double, double > etaPiS0Int = results.intensity( etaPiS0 );
            outfile[j] << etaPiS0Int.first << "\t" << etaPiS0Int.second << "\t";
            //cout<<"P0"<<endl;
            vector<string> etaPiP0;
            etaPiP0.push_back("EtaPi::Negative::P0-");
            pair< double, double > etaPiP0Int = results.intensity( etaPiP0 );
            outfile[j] << etaPiP0Int.first << "\t" << etaPiP0Int.second << "\t";
            //cout<<"P1-"<<endl;
            vector<string> etaPiP1m;
            etaPiP1m.push_back("EtaPi::Negative::P1-");
            pair< double, double > etaPiP1mInt = results.intensity( etaPiP1m );
            outfile[j] << etaPiP1mInt.first << "\t" << etaPiP1mInt.second << "\t";
            //cout<<"D0"<<endl;
            vector<string> etaPiD0;
            etaPiD0.push_back("EtaPi::Negative::D0-");
            pair< double, double > etaPiD0Int = results.intensity( etaPiD0 );
            outfile[j] << etaPiD0Int.first << "\t" << etaPiD0Int.second << "\t";
            //cout<<"D1-"<<endl;
            vector<string> etaPiD1;
            etaPiD1.push_back("EtaPi::Negative::D1-");
            pair< double, double > etaPiD1Int = results.intensity( etaPiD1 );
            outfile[j] << etaPiD1Int.first << "\t" << etaPiD1Int.second << "\t";
            
            
            //P1+
            //cout<<"P1+"<<endl;
            vector<string> etaPiP1p;
            etaPiP1p.push_back("EtaPi::Positive::P1+");
            pair< double, double > etaPiP1pInt = results.intensity( etaPiP1p );
            outfile[j] << etaPiP1pInt.first << "\t" << etaPiP1pInt.second << "\t";
            //D1+
            //cout<<"D1+"<<endl;
            vector<string> etaPiD1p;
            etaPiD1p.push_back("EtaPi::Positive::D1+");
            pair< double, double > etaPiD1pInt = results.intensity( etaPiD1p );
            outfile[j] << etaPiD1pInt.first << "\t" << etaPiD1pInt.second << "\t";
            
            vector< string > all;
            all.push_back( "EtaPi::Negative::S0-" );
            all.push_back( "EtaPi::Negative::P0-" );
            all.push_back( "EtaPi::Negative::P1-" );
            all.push_back( "EtaPi::Negative::D0-" );
            all.push_back( "EtaPi::Negative::D1-" );
            
            all.push_back( "EtaPi::Positive::P1+" );
            all.push_back( "EtaPi::Positive::D1+" );
            
            pair< double, double > allInt = results.intensity( all );
            outfile[j] << allInt.first << "\t" << allInt.second << "\t";
            
            outfile[j] << endl;
         //cout<<"end bs bin "<<i<<endl;    
        }
	outfile[j].close();
        //cout<<"end fit bin "<<j<<endl;
        chdir( ".." );

    }
    
    
    return 0;
}
