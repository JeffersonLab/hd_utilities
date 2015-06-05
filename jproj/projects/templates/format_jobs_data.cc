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

void getUtime(std::string input, Long64_t &utime, Long64_t default_value);
void getMemory(std::string input, Int_t &mem_int, Int_t default_value);
void getNumber(std::string input, Int_t &num, Int_t default_value);

Bool_t debug = false;

int main(int argc, char **argv){

  char filename[200];

  sprintf(filename,"new_jobs_data.txt");
  ifstream IN(filename);

  // Count how many jobs were processed
  system("wc -l new_jobs_data.txt > ___tmp_wc.txt");
  ifstream IN_wc("___tmp_wc.txt");
  Int_t NTOTAL;
  IN_wc >> NTOTAL;
  // There are 24 lines per entry
  NTOTAL /= 24;
  system("rm -f ___tmp_wc.txt");
  cout << "total of " << NTOTAL << " files to process..." << endl;

  sprintf(filename,"formatted_jobs_data.txt");
  ofstream OUT(filename);
  Int_t nlines = 0;

  // Read variables that can be assigned as "NULL"
  // as strings, convert to ints if not NULL.
  Int_t id, run, file, jobId;
  std::string timeChange;
  std::string hostname, status, exitCode, result;
  std::string timeSubmitted;  Long64_t utimeSubmitted;
  std::string timeDependency; Long64_t utimeDependency;
  std::string timePending;    Long64_t utimePending;
  std::string timeStagingIn;  Long64_t utimeStagingIn;
  std::string timeActive;     Long64_t utimeActive;
  std::string timeStagingOut; Long64_t utimeStagingOut;
  std::string timeComplete;   Long64_t utimeComplete;
  std::string walltime, cput, mem, vmem, error;
  std::string neventsString, timeCopyString, timePluginString;

  // convert read values into ints
  Int_t mem_int, vmem_int;
  Int_t nevents, timeCopy, timePlugin;
  Int_t errorCode;

  Int_t nTotal = 0;
  Int_t nSuccess = 0;
  Int_t nTimeOut = 0;
  Int_t nOverRlimit = 0;
  Int_t nInputFail = 0;
  Int_t nNoJobStatus = 0;
  Int_t nFailUnknown = 0;

  sprintf(filename,"results.root");
  TFile *outfile = new TFile(filename,"recreate");
  TH1F *hmem = new TH1F("hmem",";mem used (MB);",1000,0, 3. * 1000.);
  TH1F *hvmem = new TH1F("hvmem",";vmem used (MB);",1000,0, 5. * 1000.);

  ofstream OUT_Success("success.txt");
  ofstream OUT_TimeOut("timeout.txt");
  ofstream OUT_OverRlimit("overrlimit.txt");
  ofstream OUT_InputFail("inputfail.txt");
  ofstream OUT_NoJobStatus("nojobstatus.txt");
  ofstream OUT_FailUnknown("failunknown.txt");

  while(IN >> id >> run >> file >> jobId
	>> timeChange
	>> hostname >> status >> exitCode >> result
	>> timeSubmitted >> timeDependency >> timePending
	>> timeStagingIn >> timeActive     >> timeStagingOut
	>> timeComplete
	>> walltime >> cput
	>> mem >> vmem >> error
	>> neventsString >>  timeCopyString >> timePluginString){
    
    if(debug && result != "SUCCESS"){
      cout << "-----------------------------------------------------------------------------------" << endl;
      cout << "id = " << id << " run = " << run << " file = " << file << " jobId = " << jobId << endl
	   << " timeChange = " << timeChange << endl
	   << " hostname = " << hostname << " status = " << status << " exitCode = " << exitCode << " result = " << result << endl
	   << " timeSubmitted = " << timeSubmitted << endl
	   << " timeDependency = " << timeDependency << endl
	   << " timePending = " << timePending << endl
	   << " timeStagingIn = " << timeStagingIn << endl
	   << " timeActive = " << timeActive << endl
	   << " timeStagingOut = " << timeStagingOut << endl
	   << " timeComplete = " << timeComplete << endl
	   << " walltime = " << walltime << " cput = " << cput << " mem = " << mem << " vmem = " << vmem << endl
	   << " error = " << error << endl
	   << " nevents = " << nevents << " timeCopy = " << timeCopy << " timePlugin = " << timePlugin << endl;
    }

    if(result == "TIMEOUT"){
      nTimeOut++;
      OUT_TimeOut << setw(6) << setfill('0') << run << "   " << setw(3) << setfill('0') << file << endl;
    }
    else if(result == "OVER_RLIMIT"){
      nOverRlimit++;
      OUT_OverRlimit << setw(6) << setfill('0') << run << "   " << setw(3) << setfill('0') << file << endl;
    }
    else if(result == "FAILED" && error == "fail_to_get_input_file"){
      nInputFail++;
      OUT_InputFail << setw(6) << setfill('0') << run << "   " << setw(3) << setfill('0') << file << endl;
    }
    else if(result == "FAILED" && error == "No_job_status_in_batch_system_and_we_never_recorded_a_finish."){
      nNoJobStatus++;
      OUT_NoJobStatus << setw(6) << setfill('0') << run << "   " << setw(3) << setfill('0') << file << endl;
    }
    else if(result == "FAILED" && error == "Job_failed_with_unknown_reason"){
      nFailUnknown++;
      OUT_FailUnknown << setw(6) << setfill('0') << run << "   " << setw(3) << setfill('0') << file << endl;
    }
    else if(result == "SUCCESS"){
      nSuccess++;
      OUT_Success << setw(6) << setfill('0') << run << "   " << setw(3) << setfill('0') << file << endl;
    }
    else{
      cout << "job did not fit into any of known categories" << endl;
      cout << "-----------------------------------------------------------------------------------" << endl;
      cout << "id = " << id << " run = " << run << " file = " << file << " jobId = " << jobId << endl
	   << " timeChange = " << timeChange << endl
	   << " hostname = " << hostname << " status = " << status << " exitCode = " << exitCode << " result = " << result << endl
	   << " timeSubmitted = " << timeSubmitted << endl
	   << " timeDependency = " << timeDependency << endl
	   << " timePending = " << timePending << endl
	   << " timeStagingIn = " << timeStagingIn << endl
	   << " timeActive = " << timeActive << endl
	   << " timeStagingOut = " << timeStagingOut << endl
	   << " timeComplete = " << timeComplete << endl
	   << " walltime = " << walltime << " cput = " << cput << " mem = " << mem << " vmem = " << vmem << endl
	   << " error = " << error << endl
	   << " nevents = " << nevents << " timeCopy = " << timeCopy << " timePlugin = " << timePlugin << endl;
      abort();
    }

    // Convert times into utime
    getUtime(timeSubmitted,utimeSubmitted,-10000);
    if(debug) cout << utimeSubmitted << endl;

    getUtime(timeDependency,utimeDependency,-20000);
    if(debug) cout << utimeDependency << endl;

    getUtime(timePending,utimePending,-40000);
    if(debug) cout << utimePending << endl;

    getUtime(timeStagingIn,utimeStagingIn,-80000);
    if(debug) cout << utimeStagingIn << endl;

    getUtime(timeActive,utimeActive,-160000);
    if(debug) cout << utimeActive << endl;

    getUtime(timeStagingOut,utimeStagingOut,-320000);
    if(debug) cout << utimeStagingOut << endl;

    getUtime(timeComplete,utimeComplete,-640000);
    if(debug) cout << utimeComplete << endl;

    getMemory(mem,mem_int,-999 * 1024);
    getMemory(vmem,vmem_int,-999 * 1024);
    if(debug) cout << mem_int << "   " << vmem_int << endl;

    getNumber(neventsString,nevents,-999);
    getNumber(timeCopyString,timeCopy,-999);
    getNumber(timePluginString,timePlugin,-999);
    if(debug) cout << "nevents = " << nevents << " timeCopy = " << timeCopy << " timePlugin =  " << timePlugin << endl;

    OUT << id << "\t" << run << "\t" << file << "\t" << jobId
	<< "\t" << timeChange
	<< "\t" << hostname          << "\t" << status << "\t" << exitCode << "\t" << result
	<< "\t" << timeSubmitted
	<< "\t" << timeDependency
	<< "\t" << timePending
	<< "\t" << timeStagingIn
	<< "\t" << timeActive
	<< "\t" << timeStagingOut
	<< "\t" << timeComplete
	<< "\t" << walltime          << "\t" << cput << "\t" << mem << "\t" << vmem << "\t" << errorCode
	<< "\t" << utimeSubmitted    << "\t" << utimeDependency
	<< "\t" << utimePending      << "\t" << utimeStagingIn
	<< "\t" << utimeActive       << "\t" << utimeStagingOut << "\t" << utimeComplete
	<< "\t" << mem_int           << "\t" << vmem_int
	<< "\t" << nevents           << "\t" << timeCopy << "\t" << timePlugin << endl;

    nTotal++;
    if(nTotal % 200 == 0) cout << "processed " << setw(5) << nTotal << " / " << NTOTAL << endl;
  }

  /*
    if(hostname == "NULL" && status == "NULL"){
    if(debug){
    cout << "hostname is NULL AND status is NULL" << endl;
    // If status was NULL, entries for
    // timeCompleteDay,   timeCompleteTime
    // cput, mem, vmem, error will be empty.
    cout << "timeComplete     = " << timeComplete     << endl;
    cout << "timeCompleteTime = " << timeCompleteTime << endl;
    cout << "walltime         = " << walltime;
    cout << "cput             = " << cput;
    cout << "mem              = " << mem;
    cout << "vmem             = " << vmem;
    cout << "error            = " << error;
    }
    walltime = "-999";
    cput     = "-999";
    mem      = "-999";
    vmem     = "-999";
    error    = "-999";
    }else if(hostname == "NULL" && status == "PENDING"){
    if(debug){ cout << "in PENDING " << endl;
    // If job was not submitted, entries for
    // cput, mem, vmem, error will be empty.
    cout << "walltime         = " << walltime;
    cout << "cput             = " << cput;
    cout << "mem              = " << mem;
    cout << "vmem             = " << vmem;
    cout << "error            = " << error;
    }
    walltime = "-999";
    cput     = "-999";
    mem      = "-999";
    vmem     = "-999";
error    = "-999";
    }else if(hostname == "NULL" && status == "DEPENDENCY"){
      if(debug){ cout << "in DEPENDENCY " << endl;
	// If job was in DEPENDENCY, entries for
	// walltime, cput, mem, vmem, error will be empty.
	cout << "walltime         = " << walltime;
	cout << "cput             = " << cput;
	cout << "mem              = " << mem;
	cout << "vmem             = " << vmem;
	cout << "error            = " << error;
      }
      walltime = "-999";
      cput     = "-999";
      mem      = "-999";
      vmem     = "-999";
      error    = "-999";
    }else if(status == "ACTIVE"){
      cout << "status was ACTIVE" << endl;
      if(debug){
	cout << "timeStagingOutTime = " << timeStagingOutTime << endl;
	cout << "timeCompleteTime   = " << timeCompleteTime   << endl;
      }
      timeStagingOutTime = "NULL";
      timeCompleteTime   = "NULL";
      errorCode = 5;
    }else if(result == "FAILED"){
      cout << "result was FAILED" << endl;
      if(debug){
	// If job failed to get input file, then
	// pending, stagingIn, active, stagingOut
	// will all be NULL
	cout << "timePendingDay = " << timePendingDay;
	// If this is a failure with unknown reason,
	// we will have all times
      }
      errorCode = 3;
      cout << "(Job failed with unknown reason)" << endl;
      }else{
	cout << "timeStagingInTime  = " << timeStagingInTime << endl;
	cout << "timeActiveTime     = " << timeActiveTime << endl;
	cout << "timeStagingOutTime = " << timeStagingOutTime << endl;
	cout << "walltime         = " << walltime;
	cout << "cput             = " << cput;
	cout << "mem              = " << mem;
	cout << "vmem             = " << vmem;
	cout << "error            = " << error;
	errorCode = 4;
	cout << "(fail to get input file)" << endl;
      }
    }else if(result == "TIMEOUT"){
      if(debug) cout << "result was TIMEOUT" << endl;
      cout << "timeStagingInTime  = " << timeStagingInTime << endl;
      cout << "timeActiveTime     = " << timeActiveTime << endl;
      cout << "timeStagingOutTime = " << timeStagingOutTime << endl;
      cout << "walltime         = " << walltime;
      cout << "cput             = " << cput;
      cout << "mem              = " << mem;
      cout << "vmem             = " << vmem;
      cout << "error            = " << error;
      errorCode = 1;

      OUT_timedout << setw(6) << setfill('0') << run << "   " << setw(3) << setfill('0') << file << endl;
      nTimedOut++;
    }else{
      if(debug) cout << "in normal processing " << endl;
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

  */
  cout << "--------------------------------------------------"  << endl;
  cout << "total                            : " << setw(5) << setfill(' ') << nTotal       << endl;
  cout << "number of success                : " << setw(5) << setfill(' ') << nSuccess     << endl;
  cout << "number of timed out              : " << setw(5) << setfill(' ') << nTimeOut     << endl;
  cout << "number of exceeded resource limit: " << setw(5) << setfill(' ') << nOverRlimit  << endl;
  cout << "number of fail to get input file : " << setw(5) << setfill(' ') << nInputFail   << endl;
  cout << "number of no job status          : " << setw(5) << setfill(' ') << nNoJobStatus << endl;
  cout << "number of fail for unknown reason: " << setw(5) << setfill(' ') << nFailUnknown << endl;
  outfile->Write();

  // cleanup
  system("rm -f ___tmp1.txt");
  return 0;
}

void getUtime(std::string input, Long64_t &utime, Long64_t default_value){
  char command[400];
  ifstream IN_tmp;
  if(input != "NULL"){
    size_t pos = input.find("_");
    if(pos==std::string::npos){
      cout << "could not find '_' in date-time string..." << endl;
      abort();
    }
    std::string day  = input.substr(0,pos);
    std::string time = input.substr(pos+1);
    if(debug) cout << "day = " << day << " time = " << time << endl;
    sprintf(command,"date -d \"%s %s\" +%%s > ___tmp1.txt",day.c_str(), time.c_str());
    system(command);
    IN_tmp.open("___tmp1.txt");
    IN_tmp >> utime;
    IN_tmp.close();
    system("rm -f ___tmp1");
  }else{
    if(debug) cout << input << " is NULL" << endl;
    utime = default_value;
  }

}

void getMemory(std::string input, Int_t &mem_int, Int_t default_value){
  char command[400];
  ifstream IN_tmp;
  if(input != "-999"){
    sprintf(command,"echo \"%s\" | sed 's:kb::' > ___tmp1.txt",input.c_str());
    system(command);
    IN_tmp.open("___tmp1.txt");
    IN_tmp >> mem_int;
    IN_tmp.close();
    system("rm -f ___tmp1.txt");
  }else{
    mem_int = default_value;
  }
}

void getNumber(std::string input, Int_t &num, Int_t default_value){
  if(input != "NULL"){
    num = atoi(input.c_str());
  }else{
    num = default_value;
  }
}
