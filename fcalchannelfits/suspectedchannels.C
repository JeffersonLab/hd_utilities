#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <TMath.h>
#include <TCanvas.h>
using namespace std;


////// This is to add suspected channels #5 to badchannels files

/////////////////////////////////////////////////////////////

   int RunNumber[19]={61321,61322,61323,61325,61327,61329,61330,61331,61332,61333,
                   61334,61335,61336,61337,61340,61341,61342,61343,61344};



//////////////////////////////////////////////////////////////////

   void Besuspectedchannels()
        {/////////Besuspectedchannels

     ifstream badinfile;///// bad channel input file
     string badinname;///// string for bad channel input file

     ifstream suspectinfile;///// suspected channel input file
     string suspectinname;///// string for suspected channel input file


     ifstream badchinfile;///// bad channel input file
     string badchinfilename ;///// string for bad channel input file


     ofstream testingoutfile;///// bad channel input file
     string testingoutfilename ;///// string for bad channel input file





   int coun=1; 
                     

        

               for(int regi=1;regi<=5;regi++)

                        {/////regi loop

                           float rn,reg,chan,peak;

            			stringstream s1;    s1<<regi;          ////// region #number

            			suspectinname  ="R"+s1.str();
            			suspectinname +="NpeaksVSchannels";
           			suspectinname +=".txt";
                  suspectinfile.open(suspectinname.c_str(),std::fstream::in) ;


                       // cout<<suspectinname<<endl;
               



                              while(suspectinfile>>rn>>reg>>chan>>peak)
                                     {////// while1
                            //  cout<<rn<<"\t"<<reg<<endl;

                                  for(int RN=0;RN<19;RN++)
                                       {//// RN LOOP


                                stringstream s2;    s2<<RunNumber[RN];          ////// run #number

            			badchinfilename  ="/home/user/Desktop/fcalbadchannelchandra/badchannels/"+s2.str();
            			badchinfilename +="/BADchannelcalib"+s2.str();
           			badchinfilename +=".txt";
testingoutfilename ="testingsuspiciouschannel";
testingoutfilename +=s2.str();
testingoutfilename +=".txt"; 

                  badchinfile.open(badchinfilename.c_str(),std::fstream::in) ;

                 testingoutfile.open(testingoutfilename.c_str(),ios::out| ios::app) ;


	                        float a;

                                        if(rn==RunNumber[RN])
                                           {///// if loop1

                                             while(badchinfile>>a)
                                              {//////while 2




//cout<<a<<endl;   

  cout<<rn<<"\t"<<reg<<endl;

testingoutfile<<a<<endl;

	testingoutfile.close();



                                                  coun++;
                                               }//////while 2
//badchinfile.close();

   //testingoutfile.close();


                                           }///// if loop1

   //testingoutfile.close();



                                        }//// RN LOOP



//badchinfile.close();

                                      }////// while1


                           suspectinfile.close(); ///// very imp to close the in put file, if not loop will execute only once not desired #of times



                    }////regi



       }////////Besuspectedchannels



/////////////////////////////////////////////////////////////////////////////////////////////////

    void suspectedchannels()
     {//////start


       Besuspectedchannels();

     }/////end
