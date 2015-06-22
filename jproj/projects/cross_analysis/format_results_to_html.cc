
/*********************************************************************************
 *
 * 2015/03/31 Kei Moriya
 *
 * Parse the output text file of results.txt to create html tables.
 *
 *********************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <vector>
#include <sstream>
#include <map>
#include "unistd.h" // to use optarg
#include <assert.h>
#include <stdlib.h>
#include <algorithm>    // std::find

using namespace std;

int main(int argc, char **argv){

  int  VER_LAST = 15;
  int  VER_INIT =  9;
  bool debug   = false;
  std::string RUNPERIOD = "2014_10";

  extern char* optarg;
  // Check command line arguments
  int c;
  while((c = getopt(argc,argv,"hR:F:V:d")) != -1){
    switch(c){
    case 'h':
      cout << "format_results_to_html: " << endl;
      cout << "Options:" << endl;
      cout << "\t-h    This message" << endl;
      cout << "\t-F    Set final  launch version to process" << endl;
      cout << "\t-V    Version number of first launch to analyze" << endl;
      cout << "\t-R    Set run period (e.g., 2015_03)" << endl;
      cout << "\t-d    Print debug messages" << endl;
      exit(-1);
      break;
    case 'F':
      VER_LAST = atoi(optarg);
      break;
    case 'V':
      VER_INIT = atoi(optarg);
      break;
    case 'R':
      RUNPERIOD = optarg;
      break;
    case 'd':
      debug = true;
      break;
    default:
      break;
    }
  }

  std::vector<int> SKIPPED_LAUNCHES;

  if(RUNPERIOD == "2014_10")
    SKIPPED_LAUNCHES.push_back(14);
  else if(RUNPERIOD == "2015_03")
    SKIPPED_LAUNCHES.push_back(1);

  cout << "Skipped launches for " << RUNPERIOD << ": " << endl;
  for(int i=0;i<SKIPPED_LAUNCHES.size();i++){
    cout << "   ver" << setw(2) << setfill('0') << SKIPPED_LAUNCHES[i] << endl;
  }
  cout << "-----------------------------" << endl;

  // Get the launch ver numbers
  std::vector<int> LAUNCHVERS;
  for(int i=VER_INIT;i<=VER_LAST;i++){
    std::vector<int>::iterator it;
    it = find (SKIPPED_LAUNCHES.begin(), SKIPPED_LAUNCHES.end(), i);
    if (it != SKIPPED_LAUNCHES.end()){
      cout << "skipping launch ver" << setw(2) << setfill('0') << i << endl;
    }else{
      LAUNCHVERS.push_back(i);
    }
  }

  cout << "TOTAL OF " << LAUNCHVERS.size() << " LAUNCHES" << endl;
  for(int i=0;i<LAUNCHVERS.size();i++){
    cout << "  " << setw(2) << setfill('0') << i+1 << " : " <<  setw(2) << setfill('0') << LAUNCHVERS[i] << endl;
  }
  int NVERS = LAUNCHVERS.size();

  char filename[200];
  if(RUNPERIOD=="2014_10") sprintf(filename,"results.txt");
  else                     sprintf(filename,"results_%s.txt",RUNPERIOD.c_str());
  ifstream IN(filename);
  if(!IN){
    cout << "input file " << filename << " does not exist" << endl;
    abort();
  }

  // input variables
  int run, file;
  std::string results[NVERS];

  int run_previous = -999;
  int hundreds_previous = -100;

  ofstream OUT;
  double value;

  // This is for every single run, file
  int ntotalAll[NVERS];
  int nsuccessAll[NVERS];
  int nover_limitAll[NVERS];
  int ntimeoutAll[NVERS];
  int nfailedAll[NVERS];
  int nnullAll[NVERS];

  // These are for each hundreds of runs
  int ntotal[NVERS];
  int nsuccess[NVERS];
  int nover_limit[NVERS];
  int ntimeout[NVERS];
  int nfailed[NVERS];
  int nnull[NVERS];

  for(int i=0;i<NVERS;i++){
    ntotal[i]      = 0;
    nsuccess[i]    = 0;
    nover_limit[i] = 0;
    ntimeout[i]    = 0;
    nfailed[i]     = 0;
    nnull[i]       = 0;

    ntotalAll[i] = 0;
    nsuccessAll[i] = 0;
    nover_limitAll[i] = 0;
    ntimeoutAll[i] = 0;
    nfailedAll[i] = 0;
    nnullAll[i] = 0;
  }

  bool isNewRun = false;

  // Results are one of
  // - SUCCESS
  // - OVER_RLIMIT
  // - TIMEOUT
  // - FAILED
  // - NULL
  char textcolor[80];
  char bgcolor[80];
  char tdoutput[400];

  char command[400];

  while(IN >> run >> file){
    for(int i=0;i<NVERS;i++){
      IN >> results[i];
      if(debug) cout << "i = " << i << " results[i] = " << results[i] << endl;
    }

    if(run != run_previous){
      if(debug) cout << "beginning of new run " << run << endl;
      run_previous = run;
      isNewRun = true;

      if(run / 100 != hundreds_previous){
	if(debug) cout << "start of new hundreds: " << run / 100 * 100 << " with run = " << run << endl;
	if(OUT.is_open()){

	  if(debug){
	    for(int i=0;i<NVERS;i++){
	      cout << "ntotal["      << i << "] = " << ntotal[i]      << endl;
	      cout << "nsuccess["    << i << "] = " << nsuccess[i]    << endl;
	      cout << "nover_limit[" << i << "] = " << nover_limit[i] << endl;
	      cout << "ntimeout["    << i << "] = " << ntimeout[i]    << endl;
	      cout << "nfailed["     << i << "] = " << nfailed[i]     << endl;
	      cout << "nnull["       << i << "] = " << nnull[i]       << endl;
	    }
	  }

	  // Finish table for each file
	  OUT << "  </table>" << endl;
	  OUT << endl;
	  OUT << "  <hr style=\"width:80%;align:center;height:3px;color:#ff0000;border-color:#ff0000;background-color:#ff0000;\">" << endl;
	  OUT << endl;

	  // Create new table with stats for these hundreds
	  OUT << "  <table style=\"border: 1px solid black; table-layout: fixed;\">" << endl;
	  OUT << "    <tr>";

	  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Status for "
	      << setw(6) << setfill('0') << hundreds_previous * 100 << " Runs</th>";
	  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Total (%)</th>";
	  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Success (%)</th>";
	  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Over Limit (%)</th>";
	  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Timeout (%)</th>";
	  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Failed (%)</th>";
	  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">NULL (%)</th>";
	  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">requested memory (MB)</th>";
	  OUT << "    </tr>" << endl;

	  double value;
	  for(int i=0;i<NVERS;i++){
	    // total
	    value = ntotal[i]==0 ? 0 : 100. * ntotal[i] / ntotal[i];
	    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">VER"
		<< setw(2) << setfill('0') << LAUNCHVERS[i] << "</td>"
		<< "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
		<< setw(3) << setfill(' ') << ntotal[i] << " ("
		<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

	    // success
	    value = ntotal[i]==0 ? 0 : 100. * nsuccess[i] / ntotal[i];
	    OUT	<< "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
		<< setw(3) << setfill(' ') << nsuccess[i] << " ("
		<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

	    // over_limit
	    value = ntotal[i]==0 ? 0 : 100. * nover_limit[i] / ntotal[i];
	    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
		<< setw(3) << setfill(' ') << nover_limit[i] << " ("
		<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

	    // timeout
	    value = ntotal[i]==0 ? 0 : 100. * ntimeout[i] / ntotal[i];
	    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
		<< setw(3) << setfill(' ') << ntimeout[i] << " ("
		<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

	    // failed
	    value = ntotal[i]==0 ? 0 : 100. * nfailed[i] / ntotal[i];
	    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
		<< setw(3) << setfill(' ') << nfailed[i] << " ("
		<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

	    // null
	    value = ntotal[i]==0 ? 0 : 100. * nnull[i] / ntotal[i];
	    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
		<< setw(3) << setfill(' ') << nnull[i] << " ("
		<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

	    // Get requested memory from jsub file for each launch
	    if((RUNPERIOD == "2014_10" && LAUNCHVERS[i] >= 11) || RUNPERIOD != "2014_10"){
	      sprintf(command,"grep 'Memory space' /group/halld/data_monitoring/run_conditions/offline_monitoring_RunPeriod%s_ver%2.2d_hd_rawdata.jsub | sed 's/.*space=\"//' | sed 's/\".*//' > ___tmp.txt",RUNPERIOD.c_str(),LAUNCHVERS[i]);
	      system(command);
	      ifstream in("___tmp.txt");
	      in >> value;
	    }else{
	      // For launch ver 09, 10, just set to 5000 MB
	      value = 5000;
	    }
	    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
		<< setw(4) << setfill(' ') << std::fixed << std::setprecision(0) << value << "</td>" << endl;

	    OUT << "    </tr>" << endl;
	  }

	  OUT << "  </table>" << endl;

	  // Finish processing of previous hundreds
	  OUT << "<h2><a href = \"../index.html\">back to main</a></h2>" << endl;

	  OUT << "</body>" << endl;
	  OUT.close();

	  // reset counters for each hundreds of runs
	  if(debug) cout << "resetting counters for hundreds" << endl;
	  for(int i=0;i<NVERS;i++){
	    ntotal[i]      = 0;
	    nsuccess[i]    = 0;
	    nover_limit[i] = 0;
	    ntimeout[i]    = 0;
	    nfailed[i]     = 0;
	    nnull[i]       = 0;
	  }
	} // end of OUT.is_open() is true

	// We are still at beginning of new hundreds

	cout << "start of new hundreds " << (run / 100) << endl;
	sprintf(filename,"mkdir -p html_%s",RUNPERIOD.c_str());
	system(filename);
	sprintf(filename,"html_%s/results_%6.6d.html",RUNPERIOD.c_str(),(run / 100) * 100);
	OUT.open(filename);
	OUT << "<head>" << endl;
	OUT << "<link rel=\"stylesheet\" type=\"text/css\" href=\"../mystyle.css\">" << endl;
	OUT << "<meta http-equiv=\"content-style-type\" content=\"text/css\">" << endl;

	OUT << "  <title> runs " << (run / 100) * 100 << "  </title>" << endl;
	OUT << "</head>" << endl;
	OUT << "<body>" << endl;
	OUT << "<h2>results for " << (run / 100) * 100 << " runs" << endl;

	OUT << "  <table style=\"border: 1px solid black; table-layout: fixed;\">" << endl;
	OUT << "    <tr>";
	OUT << "      <th style=\"border: 1px solid black; height:15px; width:100px;\">run</th>";
	OUT << "      <th style=\"border: 1px solid black; height:15px; width:100px;\">file</th>";
	for(int i=0;i<NVERS;i++){
	OUT << " <th style=\"border: 1px solid black; height:15px; width:100px;\">VER"
	    << setw(2) << setfill('0') << LAUNCHVERS[i] << "</th>";
	}
	OUT << "</tr>" << endl;

	hundreds_previous = (run / 100);
      } // end of start of new hundreds
    } // end of beginning of new run

    if(isNewRun == true){
      // If new run, add a single line in between
      OUT << "<tr style=\"bgcolor:red; padding:25px blue;\"><td style=\"padding:0px; height:8px;\"></td></tr>";
      OUT << "    <tr> ";
      // reset flag
      isNewRun = false;
    }else{
      OUT << "    <tr> ";
    }
    OUT	<< " <td style=\" padding:15px; height:50px; width:100px; border:1px solid black;\">" << setw(6) << setfill('0') << run  << "</td>"
	<< " <td style=\" padding:15px; height:50px; width:100px; border:1px solid black;\">" << setw(3) << setfill('0') << file << "</td>";

    if(debug) cout << "processing run = " << run << " file = " << file << endl;
    for(int i=0;i<NVERS;i++){
      if(debug) cout << "i = " << i << " results[" << i << "] = " << results[i] << endl;
      if(results[i] == "SUCCESS"){
	sprintf(textcolor,"black");
	sprintf(bgcolor,"#66FF99");
	nsuccess[i]++;
	nsuccessAll[i]++;
      }else if(results[i] == "OVER_RLIMIT"){
	sprintf(textcolor,"red");
	sprintf(bgcolor,"yellow");
	nover_limit[i]++;
	nover_limitAll[i]++;
      }else if(results[i] == "TIMEOUT"){
	sprintf(textcolor,"white");
	sprintf(bgcolor,"orange");
	ntimeout[i]++;
	ntimeoutAll[i]++;
      }else if(results[i] == "FAILED"){
	sprintf(textcolor,"#ff0000");
	sprintf(bgcolor,"black");
	nfailed[i]++;
	nfailedAll[i]++;
      }else if(results[i] == "NULL"){
	sprintf(textcolor,"white");
	sprintf(bgcolor,"gray");
	nnull[i]++;
	nnullAll[i]++;
      }else{
	cout << "unknown result: " << results[i] << endl;
      }
      ntotal[i]++;
      ntotalAll[i]++;
      sprintf(tdoutput," <td style=\" padding:15px; height:50px; width:100px; border:1px solid black; background-color:%s; color:%s;\">",bgcolor,textcolor);
      OUT << tdoutput << results[i] << "</td>";
    }
    OUT << " </tr>" << endl;

  } // end of loop over input file

  // Finish table for each file fo final hundreds
  OUT << "  </table>" << endl;
  OUT << endl;
  OUT << "  <hr style=\"width:80%;align:center;height:3px;color:#ff0000;border-color:#ff0000;background-color:#ff0000;\">" << endl;
  OUT << endl;

  if(debug){
    for(int i=0;i<NVERS;i++){
      cout << "ntotal["      << i << "] = " << ntotal[i]      << endl;
      cout << "nsuccess["    << i << "] = " << nsuccess[i]    << endl;
      cout << "nover_limit[" << i << "] = " << nover_limit[i] << endl;
      cout << "ntimeout["    << i << "] = " << ntimeout[i]    << endl;
      cout << "nfailed["     << i << "] = " << nfailed[i]     << endl;
      cout << "nnull["       << i << "] = " << nnull[i]       << endl;
    }
  }

  // Create new table with stats for final hundreds
  OUT << "  <table style=\"border: 1px solid black; table-layout: fixed;\">" << endl;
  OUT << "    <tr>";

  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Status for "
      << setw(6) << setfill('0') << hundreds_previous * 100 << " Runs</th>";
  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Total (%)</th>";
  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Success (%)</th>";
  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Over Limit (%)</th>";
  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Timeout (%)</th>";
  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Failed (%)</th>";
  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">NULL (%)</th>";
  OUT << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">requested memory (MB)</th>";
  OUT << "    </tr>" << endl;

  for(int i=0;i<NVERS;i++){
    // total
    value = ntotal[i]==0 ? 0 : 100. * ntotal[i] / ntotal[i];
    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">VER"
	<< setw(2) << setfill('0') << LAUNCHVERS[i] << "</td>"
	<< "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << ntotal[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // success
    value = ntotal[i]==0 ? 0 : 100. * nsuccess[i] / ntotal[i];
    OUT	<< "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << nsuccess[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // over_limit
    value = ntotal[i]==0 ? 0 : 100. * nover_limit[i] / ntotal[i];
    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << nover_limit[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // timeout
    value = ntotal[i]==0 ? 0 : 100. * ntimeout[i] / ntotal[i];
    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << ntimeout[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // failed
    value = ntotal[i]==0 ? 0 : 100. * nfailed[i] / ntotal[i];
    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << nfailed[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // null
    value = ntotal[i]==0 ? 0 : 100. * nnull[i] / ntotal[i];
    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << nnull[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // Get requested memory from jsub file for each launch
    if((RUNPERIOD == "2014_10" && LAUNCHVERS[i] >= 11) || RUNPERIOD != "2014_10"){
      sprintf(command,"grep 'Memory space' /group/halld/data_monitoring/run_conditions/offline_monitoring_RunPeriod%s_ver%2.2d_hd_rawdata.jsub | sed 's/.*space=\"//' | sed 's/\".*//' > ___tmp.txt",RUNPERIOD.c_str(),LAUNCHVERS[i]);
      system(command);
      ifstream in("___tmp.txt");
      in >> value;
    }else if(RUNPERIOD == "2015_03"){
      sprintf(command,"grep 'Memory space' /home/gxproj2/halld/monitoring/jproj/projects/offline_monitoring_RunPeriod2015_03_ver%2.2d_hd_rawdata/offline_monitoring_RunPeriod2015_03_ver%2.2d_hd_rawdata.jsub | sed 's/.*space=\"//' | sed 's/\".*//' > ___tmp.txt",LAUNCHVERS[i],LAUNCHVERS[i]);
      system(command);
      ifstream in("___tmp.txt");
      in >> value;
    }else{
      // For launch ver 09, 10, just set to 5000 MB
      value = 5000;
    }
    OUT << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(4) << setfill(' ') << std::fixed << std::setprecision(0) << value << "</td>" << endl;

    OUT << "    </tr>" << endl;
  }
  OUT << "  </table>" << endl;

  // Finish processing of previous hundreds
  OUT << "<h2><a href = \"../index.html\">back to main</a></h2>" << endl;

  OUT << "</body>" << endl;
  OUT.close();

  for(int i=0;i<NVERS;i++){
    cout << "------------------------------------------------------" << endl;
    cout << "ver "      << setw(2) << setfill('0') << LAUNCHVERS[i] << endl;
    cout << "total of " << ntotalAll[i]                       <<  endl;
    cout << "total of " << nsuccessAll[i]    << " success"    << endl;
    cout << "total of " << nover_limitAll[i] << " over_limit" << endl;
    cout << "total of " << ntimeoutAll[i]    << " timeout"    << endl;
    cout << "total of " << nfailedAll[i]     << " failed"     << endl;
    cout << "total of " << nnullAll[i]       << " null"       << endl;
  }
  // format this info into html table
  ofstream OUT_ALLSTATS;
  
  if(RUNPERIOD == "2014_10") sprintf(filename,"allstats.html");
  else                       sprintf(filename,"allstats_%s.html",RUNPERIOD.c_str());
  OUT_ALLSTATS.open(filename);

  OUT_ALLSTATS << "<!-- Paste this html code to /group/halld/www/halldweb/html/data_monitoring/launch_analysis/index.html -->" << endl;
  OUT_ALLSTATS << endl;
  OUT_ALLSTATS << "  <hr style=\"width:80%;align:center;height:3px;color:#ff0000;border-color:#ff0000;background-color:#ff0000;\">" << endl;
  OUT_ALLSTATS << endl;

  // Create new table with stats for these hundreds
  OUT_ALLSTATS << "  <table style=\"border: 1px solid black; table-layout: fixed;\">" << endl;
  OUT_ALLSTATS << "    <tr style=\"background: #FAFAFA;\">" << endl;

  OUT_ALLSTATS << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Version </th>" << endl;
  OUT_ALLSTATS << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Total (%)</th>" << endl;
  OUT_ALLSTATS << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Success (%)</th>" << endl;
  OUT_ALLSTATS << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Over Limit (%)</th>" << endl;
  OUT_ALLSTATS << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Timeout (%)</th>" << endl;
  OUT_ALLSTATS << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">Failed (%)</th>" << endl;
  OUT_ALLSTATS << "      <th style=\"border: 1px solid black; height:15px; width:150px;\">NULL (%)</th>" << endl;
  OUT_ALLSTATS << "    </tr>" << endl;
  
  for(int i=0;i<NVERS;i++){
    value = ntotalAll[i]==0 ? 0 : 100. * ntotalAll[i] / ntotalAll[i];
    OUT_ALLSTATS << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">VER"
	<< setw(2) << setfill('0') << LAUNCHVERS[i] << "</td>"
	<< "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << ntotalAll[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // success
    value = ntotalAll[i]==0 ? 0 : 100. * nsuccessAll[i] / ntotalAll[i];
    OUT_ALLSTATS	<< "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << nsuccessAll[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // over_limit
    value = ntotalAll[i]==0 ? 0 : 100. * nover_limitAll[i] / ntotalAll[i];
    OUT_ALLSTATS << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << nover_limitAll[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // timeout
    value = ntotalAll[i]==0 ? 0 : 100. * ntimeoutAll[i] / ntotalAll[i];
    OUT_ALLSTATS << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << ntimeoutAll[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // failed
    value = ntotalAll[i]==0 ? 0 : 100. * nfailedAll[i] / ntotalAll[i];
    OUT_ALLSTATS << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << nfailedAll[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;

    // null
    value = ntotalAll[i]==0 ? 0 : 100. * nnullAll[i] / ntotalAll[i];
    OUT_ALLSTATS << "      <td style=\" padding:15px; height:50px; width:150px; border:1px solid black;\">"
	<< setw(3) << setfill(' ') << nnullAll[i] << " ("
	<< std::fixed << std::setprecision(2) << setw(5) << value << ")</td>" << endl;
    OUT_ALLSTATS << "    </tr>" << endl;
  } // end of loop over vers

  OUT_ALLSTATS << "  </table>" << endl;



  return 0;
}
