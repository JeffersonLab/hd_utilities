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
    double lowMass = 0.7;
    double highMass = 3.0;
    enum{ kNumBins = 45 };
    double lowt = 0;
    double hight = 1.2;
    enum{ kNumBinst = 4 };
    string fitDir("etaprimepi0");
                     
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
    
    double step = ( highMass - lowMass ) / kNumBins;
    double stept = ( hight - lowt ) / kNumBinst;
    
    ofstream outfile;
    outfile.open( outfileName.c_str() );
    
    // descend into the directory that contains the bins
    chdir( fitDir.c_str() );
    
    for( int i = 0; i < kNumBins;i++ ){
      for( int j = 0; j < kNumBinst; j++ ){  
	cout<<"bin "<<i<<"_"<<j<<endl;
        
        ostringstream dir;
        dir << "bin_" << i<<"_"<<j;
        chdir( dir.str().c_str() );
        
        ostringstream resultsFile;
        resultsFile << "bin_" << i <<"_"<<j<<".fit";
        
       	/*
	  ifstream file(resultsFile.str());
	cout<<resultsFile.str()<<" not found"<<"   !file=  "<<file.fail()<<endl;
	if(file.fail()) {chdir( ".." );continue;}   
	*/

        FitResults results( resultsFile.str() );
        
        if( !results.valid() ){
	  outfile<< lowMass + step * i + step / 2. << "\t";
	  outfile<< lowt + stept * j + stept / 2. << "\t";
	  outfile <<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"
                  <<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"
                  <<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"
                  <<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"
                  <<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"<<0<<"\t"
<<0<<"\t"<<0<<"\t"<<0<<endl;            
            chdir( ".." );
            continue;
        }
        
        // print out the bin center
        outfile << lowMass + step * i + step / 2. << "\t";
	outfile << lowt + stept * j + stept / 2. << "\t";
	vector<string> etaPiS0pRe;
        etaPiS0pRe.push_back("EtaPrimePi0::PositiveRe::S0+");
        pair< double, double > etaPiS0pReInt = results.intensity( etaPiS0pRe );
        outfile << etaPiS0pReInt.first << "\t" << etaPiS0pReInt.second << "\t";

	vector<string> etaPiS0pIm;
        etaPiS0pIm.push_back("EtaPrimePi0::PositiveIm::S0+");
        pair< double, double > etaPiS0pImInt = results.intensity( etaPiS0pIm );
        outfile << etaPiS0pImInt.first << "\t" << etaPiS0pImInt.second << "\t";
	
	vector<string> etaPiP0pRe;
        etaPiP0pRe.push_back("EtaPrimePi0::PositiveRe::P0+");
        pair< double, double > etaPiP0pReInt = results.intensity( etaPiP0pRe );
        outfile << etaPiP0pReInt.first << "\t" << etaPiP0pReInt.second << "\t";

	vector<string> etaPiP0pIm;
        etaPiP0pIm.push_back("EtaPrimePi0::PositiveIm::P0+");
        pair< double, double > etaPiP0pImInt = results.intensity( etaPiP0pIm );
        outfile << etaPiP0pImInt.first << "\t" << etaPiP0pImInt.second << "\t";


	vector<string> etaPiP1pRe;
        etaPiP1pRe.push_back("EtaPrimePi0::PositiveRe::P1+");
        pair< double, double > etaPiP1pReInt = results.intensity( etaPiP1pRe );
        outfile << etaPiP1pReInt.first << "\t" << etaPiP1pReInt.second << "\t";

	vector<string> etaPiP1pIm;
        etaPiP1pIm.push_back("EtaPrimePi0::PositiveIm::P1+");
        pair< double, double > etaPiP1pImInt = results.intensity( etaPiP1pIm );
        outfile << etaPiP1pImInt.first << "\t" << etaPiP1pImInt.second << "\t";


	vector<string> etaPiD0pRe;
        etaPiD0pRe.push_back("EtaPrimePi0::PositiveRe::D0+");
        pair< double, double > etaPiD0pReInt = results.intensity( etaPiD0pRe );
        outfile << etaPiD0pReInt.first << "\t" << etaPiD0pReInt.second << "\t";

	vector<string> etaPiD0pIm;
        etaPiD0pIm.push_back("EtaPrimePi0::PositiveIm::D0+");
        pair< double, double > etaPiD0pImInt = results.intensity( etaPiD0pIm );
        outfile << etaPiD0pImInt.first << "\t" << etaPiD0pImInt.second << "\t";

	vector<string> etaPiD1pRe;
        etaPiD1pRe.push_back("EtaPrimePi0::PositiveRe::D1+");
        pair< double, double > etaPiD1pReInt = results.intensity( etaPiD1pRe );
        outfile << etaPiD1pReInt.first << "\t" << etaPiD1pReInt.second << "\t";

	vector<string> etaPiD1pIm;
        etaPiD1pIm.push_back("EtaPrimePi0::PositiveIm::D1+");
        pair< double, double > etaPiD1pImInt = results.intensity( etaPiD1pIm );
        outfile << etaPiD1pImInt.first << "\t" << etaPiD1pImInt.second << "\t";

	vector<string> etaPiD2pRe;
        etaPiD2pRe.push_back("EtaPrimePi0::PositiveRe::D2+");
        pair< double, double > etaPiD2pReInt = results.intensity( etaPiD2pRe );
        outfile << etaPiD2pReInt.first << "\t" << etaPiD2pReInt.second << "\t";

	vector<string> etaPiD2pIm;
        etaPiD2pIm.push_back("EtaPrimePi0::PositiveIm::D2+");
        pair< double, double > etaPiD2pImInt = results.intensity( etaPiD2pIm );
        outfile << etaPiD2pImInt.first << "\t" << etaPiD2pImInt.second << "\t";

	vector<string> etaPiG0pRe;
        etaPiG0pRe.push_back("EtaPrimePi0::PositiveRe::G0+");
        pair< double, double > etaPiG0pReInt = results.intensity( etaPiG0pRe );
        outfile << etaPiG0pReInt.first << "\t" << etaPiG0pReInt.second << "\t";

	vector<string> etaPiG0pIm;
        etaPiG0pIm.push_back("EtaPrimePi0::PositiveIm::G0+");
        pair< double, double > etaPiG0pImInt = results.intensity( etaPiG0pIm );
        outfile << etaPiG0pImInt.first << "\t" << etaPiG0pImInt.second << "\t";

	vector<string> etaPiG1pRe;
	etaPiG1pRe.push_back("EtaPrimePi0::PositiveRe::G1+");
	pair< double, double > etaPiG1pReInt = results.intensity( etaPiG1pRe );
	outfile << etaPiG1pReInt.first << "\t" << etaPiG1pReInt.second << "\t";

	vector<string> etaPiG1pIm;
	etaPiG1pIm.push_back("EtaPrimePi0::PositiveIm::G1+");
	pair< double, double > etaPiG1pImInt = results.intensity( etaPiG1pIm );
	outfile << etaPiG1pImInt.first << "\t" << etaPiG1pImInt.second << "\t";




             
        vector< string > all;
	all.push_back( "EtaPrimePi0::PositiveRe::S0+" );
        all.push_back( "EtaPrimePi0::PositiveRe::P0+" );
	all.push_back( "EtaPrimePi0::PositiveRe::P1+" );
	all.push_back( "EtaPrimePi0::PositiveRe::D0+" );
	all.push_back( "EtaPrimePi0::PositiveRe::D1+" );
	all.push_back( "EtaPrimePi0::PositiveRe::D2+" );
	all.push_back( "EtaPrimePi0::PositiveRe::G0+" );
	all.push_back( "EtaPrimePi0::PositiveRe::G1+" );
	all.push_back( "EtaPrimePi0::PositiveIm::S0+" );
	all.push_back( "EtaPrimePi0::PositiveIm::P0+" );
	all.push_back( "EtaPrimePi0::PositiveIm::P1+" );
	all.push_back( "EtaPrimePi0::PositiveIm::D0+" );
	all.push_back( "EtaPrimePi0::PositiveIm::D1+" );
	all.push_back( "EtaPrimePi0::PositiveIm::D2+" );
	all.push_back( "EtaPrimePi0::PositiveIm::G0+" );
	all.push_back( "EtaPrimePi0::PositiveIm::G1+" );

        pair< double, double > allInt = results.intensity( all );
        outfile << allInt.first << "\t" << allInt.second << "\t";
	/*        
        pair< double, double > phaseD0 =
        results.phaseDiff( "EtaPi::Negative::S0-",
                          "EtaPi::Negative::D0-");
        
        outfile << phaseD0.first << "\t" << phaseD0.second << "\t";
        
        pair< double, double > phaseP0 =
        results.phaseDiff( "EtaPi::Negative::S0-",
                          "EtaPi::Negative::P0-");
        
        outfile << phaseP0.first << "\t" << phaseP0.second << "\t";
        
        pair< double, double > phaseP1 =
        results.phaseDiff( "EtaPi::Negative::S0-",
                          "EtaPi::Negative::P1-");
        
        outfile << phaseP1.first << "\t" << phaseP1.second << "\t";
        
        pair< double, double > phaseD1 =
        results.phaseDiff( "EtaPi::Negative::S0-",
                          "EtaPi::Negative::D1-");
        outfile << phaseD1.first << "\t" << phaseD1.second << "\t";
        */
	
	pair< double, double > phaseP0S0 =
	results.phaseDiff( "EtaPrimePi0::PositiveRe::P0+",
			     "EtaPrimePi0::PositiveRe::S0+");
        outfile << phaseP0S0.first << "\t" << phaseP0S0.second << "\t";

	pair< double, double > phaseP0P1 =
	results.phaseDiff( "EtaPrimePi0::PositiveRe::P0+",
			     "EtaPrimePi0::PositiveRe::P1+");
        outfile << phaseP0P1.first << "\t" << phaseP0P1.second << "\t";
        
	pair< double, double > phaseP0D0 =
        results.phaseDiff( "EtaPrimePi0::PositiveRe::P0+",
                          "EtaPrimePi0::PositiveRe::D0+");
        outfile << phaseP0D0.first << "\t" << phaseP0D0.second << "\t";
        
	pair< double, double > phaseP0D1 =
        results.phaseDiff( "EtaPrimePi0::PositiveRe::P0+",
                          "EtaPrimePi0::PositiveRe::D1+");
        outfile << phaseP0D1.first << "\t" << phaseP0D1.second << "\t";

	pair< double, double > phaseP0D2 =
        results.phaseDiff( "EtaPrimePi0::PositiveRe::P0+",
                          "EtaPrimePi0::PositiveRe::D2+");
        outfile << phaseP0D2.first << "\t" << phaseP0D2.second << "\t";
        
	pair< double, double > phaseP0G0 =
        results.phaseDiff( "EtaPrimePi0::PositiveRe::P0+",
                          "EtaPrimePi0::PositiveRe::G0+");
        outfile << phaseP0G0.first << "\t" << phaseP0G0.second << "\t";

	pair< double, double > phaseP0G1 =
        results.phaseDiff( "EtaPrimePi0::PositiveRe::P0+",
                          "EtaPrimePi0::PositiveRe::G1+");
        outfile << phaseP0G1.first << "\t" << phaseP0G1.second << "\t";


	/*        
        pair< double, double > phaseDS =
        results.phaseDiff( "Pi+Pi-Pi+::xpol::J2_rhopi_D",
                          "Pi+Pi-Pi+::xpol::J2_f2pi_S" );
        
        outfile << phaseDS.first << "\t" << phaseDS.second << "\t";
        */
        outfile << endl;
        
        chdir( ".." );
    }
    }
    return 0;
}
