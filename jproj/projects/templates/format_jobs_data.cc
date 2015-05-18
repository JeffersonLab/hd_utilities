#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <vector>
#include <sstream>
#include <map>
#include "unistd.h" // to use optarg
#include <assert.h>

// ROOT header files
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"

using namespace std;

int main(int argc, char **argv){

  Bool_t debug = false;

  char filename[200];

  sprintf(filename,"jobs_data.txt");
  ifstream IN(filename);

  // Count how many jobs were processed
  system("wc -l jobs_data.txt > ___tmp_wc.txt");
  ifstream IN_wc("___tmp_wc.txt");
  Int_t NTOTAL;
  IN_wc >> NTOTAL;
  system("rm -f ___tmp_wc.txt");
  cout << "total of " << NTOTAL << " files to process..." << endl;

  sprintf(filename,"formatted_jobs_data.txt");
  ofstream OUT(filename);
  Int_t nlines = 0;

  Int_t id, run, file, jobId;
  std::string timeChangeDay, timeChangeTime;
  std::string hostname, status, exitCode, result;
  std::string timeSubmittedDay,  timeSubmittedTime;  Long64_t utimeSubmitted;
  std::string timeDependencyDay, timeDependencyTime; Long64_t utimeDependency;
  std::string timePendingDay,    timePendingTime;    Long64_t utimePending;
  std::string timeStagingInDay,  timeStagingInTime;  Long64_t utimeStagingIn;
  std::string timeActiveDay,     timeActiveTime;     Long64_t utimeActive;
  std::string timeStagingOutDay, timeStagingOutTime; Long64_t utimeStagingOut;
  std::string timeCompleteDay,   timeCompleteTime;   Long64_t utimeComplete;
  std::string walltime, cput, mem, vmem, error;      Int_t errorCode;
  Int_t nevents, timeCopy, timePlugin;

  Int_t mem_int, vmem_int;
  char command[400];

  Int_t nTotal = 0;
  Int_t nTimedOut = 0;
  Int_t nExceededResourceLimit = 0;

  sprintf(filename,"results.root");
  TFile *outfile = new TFile(filename,"recreate");
  TH1F *hmem = new TH1F("hmem",";mem used (MB);",1000,0, 3. * 1000.);
  TH1F *hvmem = new TH1F("hvmem",";vmem used (MB);",1000,0, 5. * 1000.);

  ofstream OUT_timedout("timedout.txt");
  ofstream OUT_exceededResourceLimit("exceededResourceLimit.txt");

  while(IN >> id >> run >> file >> jobId
	>> timeChangeDay >> timeChangeTime
	>> hostname >> status >> exitCode >> result){
    
    if(debug) cout << "hostname = " << hostname << " status = " << status << " exitCode = " << exitCode << " result = " << result << endl;

    if(hostname == "NULL" && status == "NULL"){
      if(debug) cout << "hostname is NULL AND status is NULL" << endl;
      // If status was NULL, entries for
      // timeCompleteDay,   timeCompleteTime
      // cput, mem, vmem, error will be empty.
      IN
	>> timeSubmittedDay  >> timeSubmittedTime
	>> timeDependencyDay >> timeDependencyTime
	>> timePendingDay    >> timePendingTime
	>> timeStagingInDay  >> timeStagingInTime
	>> timeActiveDay     >> timeActiveTime
	>> timeStagingOutDay >> timeStagingOutTime;

      timeCompleteDay = "NULL";
      timeCompleteTime = "NULL";
      walltime = "-999";
      cput     = "-999";
      mem      = "-999";
      vmem     = "-999";
      error    = "-999";
    }else if(hostname == "NULL" && status == "PENDING"){
      IN
	>> timeSubmittedDay  >> timeSubmittedTime
	>> timeDependencyDay >> timeDependencyTime
	>> timePendingDay    >> timePendingTime
	>> timeStagingInDay  >> timeStagingInTime
	>> timeActiveDay     >> timeActiveTime
	>> timeStagingOutDay >> timeStagingOutTime
	>> timeCompleteDay   >> timeCompleteTime;

      if(debug) cout << "in PENDING " << endl;
      // If job was not submitted, entries for
      // cput, mem, vmem, error will be empty.
      IN >> walltime;
      walltime = "-999";
      cput     = "-999";
      mem      = "-999";
      vmem     = "-999";
      error    = "-999";
    }else if(hostname == "NULL" && status == "DEPENDENCY"){
      IN
	>> timeSubmittedDay  >> timeSubmittedTime
	>> timeDependencyDay >> timeDependencyTime
	>> timePendingDay    >> timePendingTime
	>> timeStagingInDay  >> timeStagingInTime
	>> timeActiveDay     >> timeActiveTime
	>> timeStagingOutDay >> timeStagingOutTime
	>> timeCompleteDay   >> timeCompleteTime;

      if(debug) cout << "in DEPENDENCY " << endl;
      // If job was in DEPENDENCY, entries for
      // walltime, cput, mem, vmem, error will be empty.
      walltime = "-999";
      cput     = "-999";
      mem      = "-999";
      vmem     = "-999";
      error    = "-999";
    }else if(status == "ACTIVE"){
      IN
	>> timeSubmittedDay  >> timeSubmittedTime
	>> timeDependencyDay >> timeDependencyTime
	>> timePendingDay    >> timePendingTime
	>> timeStagingInDay  >> timeStagingInTime
	>> timeActiveDay     >> timeActiveTime
	// the final two are NULL, so no time
	>> timeStagingOutDay
	>> timeCompleteDay;
      timeStagingOutTime = "NULL";
      timeCompleteTime   = "NULL";
      if(debug) cout << "status was ACTIVE" << endl;
      // cout << "exitCode = " << exitCode << " result = " << result << endl;
      // cout << "timeSubmittedDay = " << timeSubmittedDay  << " " << "timeSubmittedTime = " << timeSubmittedTime << endl
      // << "timeDependencyDay = " << timeDependencyDay << " " << "timeDependencyTime = " << timeDependencyTime << endl
      // << "timePendingDay = " << timePendingDay    << " " << "timePendingTime = " << timePendingTime << endl
      // << "timeStagingInDay = " << timeStagingInDay  << " " << "timeStagingInTime = " << timeStagingInTime << endl
      // << "timeActiveDay = " << timeActiveDay     << " " << "timeActiveTime = " << timeActiveTime << endl
      // << "timeStagingOutDay = " << timeStagingOutDay << " " << "timeStagingOutTime = " << timeStagingOutTime << endl
      // << "timeCompleteDay = " << timeCompleteDay   << " " << "timeCompleteTime = " << timeCompleteTime << endl;
      
      IN >> walltime >> cput >> mem >> vmem >> error;
      errorCode = 5;
    }else if(result == "FAILED"){
      cout << "result was FAILED" << endl;
      IN
	>> timeSubmittedDay  >> timeSubmittedTime
	>> timeDependencyDay >> timeDependencyTime;

      // If job failed to get input file, then
      // pending, stagingIn, active, stagingOut
      // will all be NULL
      IN >> timePendingDay;
      // If this is a failure with unknown reason,
      // we will have all times
      if(timePendingDay != "NULL"){
	IN >> timePendingTime
	   >> timeStagingInDay  >> timeStagingInTime
	   >> timeActiveDay     >> timeActiveTime
	   >> timeStagingOutDay >> timeStagingOutTime
	   >> timeCompleteDay   >> timeCompleteTime;
	IN >> walltime >> cput >> mem >> vmem;
	if(debug) cout << "walltime = " << walltime << " cput = " << cput << " mem = " << mem << " vmem = " << vmem << endl;
	
	// Need to read in either
	// "Job failed with unknown reason."
	IN >> error; // "Job"
	if(debug) cout << error;
	IN >> error; // failed
	if(debug) cout << " " << error;
	IN >> error; // with
	if(debug) cout << " " << error;
	IN >> error; // unknown
	if(debug) cout << " " << error;
	IN >> error; // reason
	if(debug) cout << " " << error << endl;
	errorCode = 3;
	cout << "(Job failed with unknown reason)" << endl;
      }else{
	IN >> timeStagingInDay >> timeActiveDay >> timeStagingOutDay
	   >> timeCompleteDay   >> timeCompleteTime;
	timeStagingInTime  = "NULL";
	timeActiveTime     = "NULL";
	timeStagingOutTime = "NULL";
	IN >> walltime >> cput >> mem >> vmem;

	// Need to read in
	// "fail to get input file"
	IN >> error; // "fail"
	if(debug) cout << error;
	IN >> error; // to
	if(debug) cout << " " << error;
	IN >> error; // get
	if(debug) cout << " " << error;
	IN >> error; // input
	if(debug) cout << " " << error;
	IN >> error; // file
	if(debug) cout << " " << error << endl;
	errorCode = 4;
	cout << "(fail to get input file)" << endl;
      }
    }else if(result == "TIMEOUT"){
      if(debug) cout << "result was TIMEOUT" << endl;
      IN
	>> timeSubmittedDay  >> timeSubmittedTime
	>> timeDependencyDay >> timeDependencyTime
	>> timePendingDay    >> timePendingTime
	>> timeStagingInDay  >> timeStagingInTime
	>> timeActiveDay     >> timeActiveTime
	>> timeStagingOutDay >> timeStagingOutTime
	>> timeCompleteDay   >> timeCompleteTime;
      IN >> walltime >> cput >> mem >> vmem;
      if(debug) cout << "walltime = " << walltime << " cput = " << cput << " mem = " << mem << " vmem = " << vmem << endl;

      // Need to read in "Job timed out."
      IN >> error; // Job
      if(debug) cout << error;
      IN >> error; // timed
      if(debug) cout << " " << error;
      IN >> error; // out.
      if(debug) cout << " " << error << endl;
      errorCode = 1;

      OUT_timedout << setw(6) << setfill('0') << run << "   " << setw(3) << setfill('0') << file << endl;
      nTimedOut++;
    }else{
      if(debug) cout << "in normal processing " << endl;
      IN
	>> timeSubmittedDay  >> timeSubmittedTime
	>> timeDependencyDay >> timeDependencyTime
	>> timePendingDay    >> timePendingTime
	>> timeStagingInDay  >> timeStagingInTime
	>> timeActiveDay     >> timeActiveTime
	>> timeStagingOutDay >> timeStagingOutTime
	>> timeCompleteDay   >> timeCompleteTime;

      // cout << timeSubmittedDay  << " " << timeSubmittedTime << endl
      // << timeDependencyDay << " " << timeDependencyTime << endl
      // << timePendingDay    << " " << timePendingTime << endl
      // << timeStagingInDay  << " " << timeStagingInTime << endl
      // << timeActiveDay     << " " << timeActiveTime << endl
      // << timeStagingOutDay << " " << timeStagingOutTime << endl
      // << timeCompleteDay   << " " << timeCompleteTime << endl;

      IN >> walltime >> cput >> mem >> vmem;
      if(debug) cout << "walltime = " << walltime << " cput = " << cput << " mem = " << mem << " vmem = " << vmem << endl;
      // Need to get "error". This can be either NULL or "Job timed out."
      IN >> error;
      if(error == "NULL"){
	errorCode = 0;
      }
      else if(error == "Job"){
	// At this stage options are "timed out." or "exceeded resource limit."
	IN >> error;
	if(error == "timed"){
	  // read in "out."
	  IN >> error;
	  errorCode = 1;
	}else if(error == "exceeded"){
	  // read in "resource" and "limit."
	  IN >> error >> error;
	  errorCode = 2;
	  
	  // OUT_exceededResourceLimit << setw(6) << setfill('0') << run << "   " << setw(3) << setfill('0') << file << endl;
	  nExceededResourceLimit++;
	}
      }
    } // end of normal processing

    if(debug) cout << "Getting final 3 entries" << endl;

    // Get final 3 entries after error
    // If we had TIMEOUT, these will be NULL
    if(!(errorCode == 1 || errorCode == 2 || errorCode == 3 || errorCode == 4)){
      IN >> nevents >> timeCopy >> timePlugin;
    } else{
      nevents    = -999;
      timeCopy   = -999;
      timePlugin = -999;
      IN >> error >> error >> error;
    }
      
    if(debug) cout << "nevents = " << nevents << " timeCopy = " << timeCopy << " timePlugin = " << timePlugin << endl;

    nTotal++;
    if(nTotal % 200 == 0) cout << "processed " << setw(5) << nTotal << " / " << NTOTAL << endl;

    char command[400];
    ifstream IN_tmp;

    // On a Mac the command would be
    // date -j -f "%Y-%M-%d %T" "2015-01-28 21:55:43" "+%s"
    // F*** Mac for changing this.
    if(timeSubmittedDay != "NULL" && timeSubmittedTime != "NULL"){
      sprintf(command,"date -d \"%s %s\" +%%s > ___tmp1.txt",timeSubmittedDay.c_str(), timeSubmittedTime.c_str());
      system(command);
      IN_tmp.open("___tmp1.txt");
      IN_tmp >> utimeSubmitted;
      IN_tmp.close();
      // system("rm -f ___tmp1");
    }else{
      cout << "timeSubmitted is NULL for run " << run << " file " << file << endl;
      utimeSubmitted = -10000;
    }

    if(timeDependencyDay != "NULL" && timeDependencyTime != "NULL"){
      sprintf(command,"date -d \"%s %s\" +%%s > ___tmp1.txt",timeDependencyDay.c_str(), timeDependencyTime.c_str());
      system(command);
      IN_tmp.open("___tmp1.txt");
      IN_tmp >> utimeDependency;
      IN_tmp.close();
      // system("rm -f ___tmp1");
    }else{
      cout << "timeDependency is NULL for run " << run << " file " << file << endl;
      utimeDependency = -20000;
    }

    if(timePendingDay != "NULL" && timePendingTime != "NULL"){
      sprintf(command,"date -d \"%s %s\" +%%s > ___tmp1.txt",timePendingDay.c_str(), timePendingTime.c_str());
      system(command);
      IN_tmp.open("___tmp1.txt");
      IN_tmp >> utimePending;
      IN_tmp.close();
      // system("rm -f ___tmp1");
    }else{
      cout << "timePending is NULL for run " << run << " file " << file << endl;
      utimePending = -40000;
    }

    if(timeStagingInDay != "NULL" && timeStagingInTime != "NULL"){
      sprintf(command,"date -d \"%s %s\" +%%s > ___tmp1.txt",timeStagingInDay.c_str(), timeStagingInTime.c_str());
      system(command);
      IN_tmp.open("___tmp1.txt");
      IN_tmp >> utimeStagingIn;
      IN_tmp.close();
      // system("rm -f ___tmp1");
    }else{
      cout << "timeStagingIn is NULL for run " << run << " file " << file << endl;
      utimeStagingIn = -80000;
    }

    if(timeActiveDay != "NULL" && timeActiveTime != "NULL"){
      sprintf(command,"date -d \"%s %s\" +%%s > ___tmp1.txt",timeActiveDay.c_str(), timeActiveTime.c_str());
      system(command);
      IN_tmp.open("___tmp1.txt");
      IN_tmp >> utimeActive;
      IN_tmp.close();
      // system("rm -f ___tmp1");
    }else{
      cout << "timeActive is NULL for run " << run << " file " << file << endl;
      utimeActive = -160000;
    }

    if(timeStagingOutDay != "NULL" && timeStagingOutTime != "NULL"){
      sprintf(command,"date -d \"%s %s\" +%%s > ___tmp1.txt",timeStagingOutDay.c_str(), timeStagingOutTime.c_str());
      system(command);
      IN_tmp.open("___tmp1.txt");
      IN_tmp >> utimeStagingOut;
      IN_tmp.close();
      // system("rm -f ___tmp1");
    }else{
      cout << "timeStagingOut is NULL for run " << run << " file " << file << endl;
      utimeStagingOut = -320000;
    }

    if(timeCompleteDay != "NULL" && timeCompleteTime != "NULL"){
    sprintf(command,"date -d \"%s %s\" +%%s > ___tmp1.txt",timeCompleteDay.c_str(), timeCompleteTime.c_str());
    system(command);
    IN_tmp.open("___tmp1.txt");
    IN_tmp >> utimeComplete;
    IN_tmp.close();
    // system("rm -f ___tmp1.txt");
    }else{
      cout << "timeComplete is NULL for run " << run << " file " << file << endl;
      utimeComplete = -640000;
    }

    // Get size of mem in kb
    if(mem != "-999"){
      sprintf(command,"echo \"%s\" | sed 's:kb::' > ___tmp1.txt",mem.c_str());
      system(command);
      IN_tmp.open("___tmp1.txt");
      IN_tmp >> mem_int;
      IN_tmp.close();
      // system("rm -f ___tmp1.txt");
    }else{
      mem_int = -999 * 1024;
    }

    // Get size of vmem in kb
    if(vmem != "-999"){
      sprintf(command,"echo \"%s\" | sed 's:kb::' > ___tmp1.txt",vmem.c_str());
      system(command);
      IN_tmp.open("___tmp1.txt");
      IN_tmp >> vmem_int;
      IN_tmp.close();
      // system("rm -f ___tmp1.txt");
    }else{
      vmem_int = -999 * 1024;
    }

    hmem->Fill(mem_int / 1024.);
    hvmem->Fill(vmem_int / 1024.);

    if(debug)
      cout << "id = " << id << " run = " << run << " file = " << file << " jobId = " << jobId << endl
	   << " timeChangeDay = " << timeChangeDay << " timeChangeTime = " << timeChangeTime << endl
	   << " hostname = " << hostname << " status = " << status << " exitCode = " << exitCode << " result = " << result << endl
	   << " timeSubmittedDay = " << timeSubmittedDay  << " timeSubmittedTime = " << timeSubmittedTime << endl
	   << " timeDependencyDay = " << timeDependencyDay << " timeDependencyTime = " << timeDependencyTime << endl
	   << " timePendingDay = " << timePendingDay    << " timePendingTime = " << timePendingTime << endl
	   << " timeStagingInDay = " << timeStagingInDay  << " timeStagingInTime = " << timeStagingInTime << endl
	   << " timeActiveDay = " << timeActiveDay     << " timeActiveTime = " << timeActiveTime << endl
	   << " timeStagingOutDay = " << timeStagingOutDay << " timeStagingOutTime = " << timeStagingOutTime << endl
	   << " timeCompleteDay = " << timeCompleteDay   << " timeCompleteTime = " << timeCompleteTime << endl
	   << " walltime = " << walltime << " cput = " << cput << " mem = " << mem << " vmem = " << vmem << " errorCode = " << errorCode << endl;
    
    OUT << id << "\t" << run << "\t" << file << "\t" << jobId
	<< "\t" << timeChangeDay     << "\t" << timeChangeTime
	<< "\t" << hostname          << "\t" << status << "\t" << exitCode << "\t" << result
	<< "\t" << timeSubmittedDay  << "\t" << timeSubmittedTime
	<< "\t" << timeDependencyDay << "\t" << timeDependencyTime
	<< "\t" << timePendingDay    << "\t" << timePendingTime
	<< "\t" << timeStagingInDay  << "\t" << timeStagingInTime
	<< "\t" << timeActiveDay     << "\t" << timeActiveTime
	<< "\t" << timeStagingOutDay << "\t" << timeStagingOutTime
	<< "\t" << timeCompleteDay   << "\t" << timeCompleteTime
	<< "\t" << walltime          << "\t" << cput << "\t" << mem << "\t" << vmem << "\t" << errorCode
	<< "\t" << utimeSubmitted    << "\t" << utimeDependency
	<< "\t" << utimePending      << "\t" << utimeStagingIn
	<< "\t" << utimeActive       << "\t" << utimeStagingOut << "\t" << utimeComplete
	<< "\t" << mem_int           << "\t" << vmem_int
	<< "\t" << nevents           << "\t" << timeCopy << "\t" << timePlugin << endl;
    

    if(debug) cout << run << "   " << file << "   " << mem_int << "   " << vmem_int << endl;
  } // end of reading in each entry

  cout << "--------------------------------------------------" << endl;
  cout << "total                            : " << nTotal << endl;
  cout << "number of timed out              : " << nTimedOut << endl;
  cout << "number of exceeded resource limit: " << nExceededResourceLimit << endl;

  outfile->Write();

  // cleanup
  system("rm -f ___tmp1.txt");

  return 0;
}
