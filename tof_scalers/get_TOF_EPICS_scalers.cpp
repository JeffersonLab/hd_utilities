// Include files for MyA
#include <myarchiver.h>
#include <archiveportal.h>
#include <channelkey.h>
#include <era.h>
#include <iostream>
#include <cpptime.h>

// Include files for input file
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <cstring>

// Include files for root
#include <TFile.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TDatime.h>
#include <TAxis.h>
#include <TH1F.h>
#include <TLegend.h>

using namespace std;

int main() {
	// Global Variables to be used
	vector< map< unsigned int,map<string,string> > > event_time;
	//map<unsigned int,vector<TCanvas*> > scaler_TGraph;

	// Initialize Input File
	char input_file[128];
	sprintf(input_file,"RunInfo.txt");
	ifstream INPUT_FILE(input_file);

	// Read from Input file
	unsigned int run_number;
	char begin_date[16], begin_time[16], end_date[16], end_time[16];  
	char begin[32], end[32];
	unsigned int run_number_count = 0;
	while (INPUT_FILE >> run_number >> begin_date >> begin_time >> end_date >> end_time) {
		// Get Run Number and Start/Stop time
		sprintf(begin,"%s %s",begin_date,begin_time);
		sprintf(end,"%s %s",end_date,end_time);
		map<string,string> time;
		time[begin] = end;
		map< unsigned int, map<string,string> > run_time;
		run_time[run_number] = time;
		event_time.push_back(run_time);
		++run_number_count;
	}

	// Check how many run numbers there are to inform user of software resources
	if (run_number_count == 0) {
		cout << "No run period, run range, or run number given.\n";
		cout << "No TOF scalers to analyze.\n";
	}
	else if (run_number_count == 1) {
		cout << "You selected 1 run:\n";
		cout << "    Estimated Run Time:       30 sec\n";
		cout << "    Estimated ROOT File size: 1.5 MB\n";
	}
	else {
		cout << "You selected " <<  run_number_count << " runs:\n";
		cout << "    Estimated Run Time:       " << run_number_count*30/60.0 << " min\n";
		cout << "    Estimated ROOT File size: " << run_number_count*1.5 << " MB\n";
	}

	// Close Input file
	INPUT_FILE.close();

	// Create EPICS archive
  MYA::MyArchiver arch("ops");
  MYA::ArchivePortal ap(arch.Master().c_str());

  // Get the beam current EPICS channel
  MYA::ChannelKey current(ap,"IBCAD00CRCUR6"); 
  MYA::Era<float> current_era(current);

	// Get EPICS channel keys
	// Since Channel Keys are private, it is impossible to loop over names
	// or make an array of channel keys. Unfortunately this means you need
	// to make a variable name for each channel key
			
	// NORTH:
	MYA::ChannelKey nor_tof_1(ap,"TOF:N:T:1:scaler_t1");
	MYA::ChannelKey nor_tof_2(ap,"TOF:N:T:2:scaler_t1");
	MYA::ChannelKey nor_tof_3(ap,"TOF:N:T:3:scaler_t1");
	MYA::ChannelKey nor_tof_4(ap,"TOF:N:T:4:scaler_t1");
	MYA::ChannelKey nor_tof_5(ap,"TOF:N:T:5:scaler_t1");
	MYA::ChannelKey nor_tof_6(ap,"TOF:N:T:6:scaler_t1");
	MYA::ChannelKey nor_tof_7(ap,"TOF:N:T:7:scaler_t1");
	MYA::ChannelKey nor_tof_8(ap,"TOF:N:T:8:scaler_t1");
	MYA::ChannelKey nor_tof_9(ap,"TOF:N:T:9:scaler_t1");
	MYA::ChannelKey nor_tof_10(ap,"TOF:N:T:10:scaler_t1");
	MYA::ChannelKey nor_tof_11(ap,"TOF:N:T:11:scaler_t1");
	MYA::ChannelKey nor_tof_12(ap,"TOF:N:T:12:scaler_t1");
	MYA::ChannelKey nor_tof_13(ap,"TOF:N:T:13:scaler_t1");
	MYA::ChannelKey nor_tof_14(ap,"TOF:N:T:14:scaler_t1");
	MYA::ChannelKey nor_tof_15(ap,"TOF:N:T:15:scaler_t1");
	MYA::ChannelKey nor_tof_16(ap,"TOF:N:T:16:scaler_t1");
	MYA::ChannelKey nor_tof_17(ap,"TOF:N:T:17:scaler_t1");
	MYA::ChannelKey nor_tof_18(ap,"TOF:N:T:18:scaler_t1");
	MYA::ChannelKey nor_tof_19(ap,"TOF:N:T:19:scaler_t1");
	MYA::ChannelKey nor_tof_20(ap,"TOF:N:T:20:scaler_t1");
	MYA::ChannelKey nor_tof_21(ap,"TOF:N:T:21:scaler_t1");
	MYA::ChannelKey nor_tof_22(ap,"TOF:N:T:22:scaler_t1");
	MYA::ChannelKey nor_tof_23(ap,"TOF:N:T:23:scaler_t1");
	MYA::ChannelKey nor_tof_24(ap,"TOF:N:T:24:scaler_t1");
	MYA::ChannelKey nor_tof_25(ap,"TOF:N:T:25:scaler_t1");
	MYA::ChannelKey nor_tof_26(ap,"TOF:N:T:26:scaler_t1");
	MYA::ChannelKey nor_tof_27(ap,"TOF:N:T:27:scaler_t1");
	MYA::ChannelKey nor_tof_28(ap,"TOF:N:T:28:scaler_t1");
	MYA::ChannelKey nor_tof_29(ap,"TOF:N:T:29:scaler_t1");
	MYA::ChannelKey nor_tof_30(ap,"TOF:N:T:30:scaler_t1");
	MYA::ChannelKey nor_tof_31(ap,"TOF:N:T:31:scaler_t1");
	MYA::ChannelKey nor_tof_32(ap,"TOF:N:T:32:scaler_t1");
	MYA::ChannelKey nor_tof_33(ap,"TOF:N:T:33:scaler_t1");
	MYA::ChannelKey nor_tof_34(ap,"TOF:N:T:34:scaler_t1");
	MYA::ChannelKey nor_tof_35(ap,"TOF:N:T:35:scaler_t1");
	MYA::ChannelKey nor_tof_36(ap,"TOF:N:T:36:scaler_t1");
	MYA::ChannelKey nor_tof_37(ap,"TOF:N:T:37:scaler_t1");
	MYA::ChannelKey nor_tof_38(ap,"TOF:N:T:38:scaler_t1");
	MYA::ChannelKey nor_tof_39(ap,"TOF:N:T:39:scaler_t1");
	MYA::ChannelKey nor_tof_40(ap,"TOF:N:T:40:scaler_t1");
	MYA::ChannelKey nor_tof_41(ap,"TOF:N:T:41:scaler_t1");
	MYA::ChannelKey nor_tof_42(ap,"TOF:N:T:42:scaler_t1");
	MYA::ChannelKey nor_tof_43(ap,"TOF:N:T:43:scaler_t1");
	MYA::ChannelKey nor_tof_44(ap,"TOF:N:T:44:scaler_t1");
		// SOUTH:
	MYA::ChannelKey sou_tof_1(ap,"TOF:S:T:1:scaler_t1");
	MYA::ChannelKey sou_tof_2(ap,"TOF:S:T:2:scaler_t1");
	MYA::ChannelKey sou_tof_3(ap,"TOF:S:T:3:scaler_t1");
	MYA::ChannelKey sou_tof_4(ap,"TOF:S:T:4:scaler_t1");
	MYA::ChannelKey sou_tof_5(ap,"TOF:S:T:5:scaler_t1");
	MYA::ChannelKey sou_tof_6(ap,"TOF:S:T:6:scaler_t1");
	MYA::ChannelKey sou_tof_7(ap,"TOF:S:T:7:scaler_t1");
	MYA::ChannelKey sou_tof_8(ap,"TOF:S:T:8:scaler_t1");
	MYA::ChannelKey sou_tof_9(ap,"TOF:S:T:9:scaler_t1");
	MYA::ChannelKey sou_tof_10(ap,"TOF:S:T:10:scaler_t1");
	MYA::ChannelKey sou_tof_11(ap,"TOF:S:T:11:scaler_t1");
	MYA::ChannelKey sou_tof_12(ap,"TOF:S:T:12:scaler_t1");
	MYA::ChannelKey sou_tof_13(ap,"TOF:S:T:13:scaler_t1");
	MYA::ChannelKey sou_tof_14(ap,"TOF:S:T:14:scaler_t1");
	MYA::ChannelKey sou_tof_15(ap,"TOF:S:T:15:scaler_t1");
	MYA::ChannelKey sou_tof_16(ap,"TOF:S:T:16:scaler_t1");
	MYA::ChannelKey sou_tof_17(ap,"TOF:S:T:17:scaler_t1");
	MYA::ChannelKey sou_tof_18(ap,"TOF:S:T:18:scaler_t1");
	MYA::ChannelKey sou_tof_19(ap,"TOF:S:T:19:scaler_t1");
	MYA::ChannelKey sou_tof_20(ap,"TOF:S:T:20:scaler_t1");
	MYA::ChannelKey sou_tof_21(ap,"TOF:S:T:21:scaler_t1");
	MYA::ChannelKey sou_tof_22(ap,"TOF:S:T:22:scaler_t1");
	MYA::ChannelKey sou_tof_23(ap,"TOF:S:T:23:scaler_t1");
	MYA::ChannelKey sou_tof_24(ap,"TOF:S:T:24:scaler_t1");
	MYA::ChannelKey sou_tof_25(ap,"TOF:S:T:25:scaler_t1");
	MYA::ChannelKey sou_tof_26(ap,"TOF:S:T:26:scaler_t1");
	MYA::ChannelKey sou_tof_27(ap,"TOF:S:T:27:scaler_t1");
	MYA::ChannelKey sou_tof_28(ap,"TOF:S:T:28:scaler_t1");
	MYA::ChannelKey sou_tof_29(ap,"TOF:S:T:29:scaler_t1");
	MYA::ChannelKey sou_tof_30(ap,"TOF:S:T:30:scaler_t1");
	MYA::ChannelKey sou_tof_31(ap,"TOF:S:T:31:scaler_t1");
	MYA::ChannelKey sou_tof_32(ap,"TOF:S:T:32:scaler_t1");
	MYA::ChannelKey sou_tof_33(ap,"TOF:S:T:33:scaler_t1");
	MYA::ChannelKey sou_tof_34(ap,"TOF:S:T:34:scaler_t1");
	MYA::ChannelKey sou_tof_35(ap,"TOF:S:T:35:scaler_t1");
	MYA::ChannelKey sou_tof_36(ap,"TOF:S:T:36:scaler_t1");
	MYA::ChannelKey sou_tof_37(ap,"TOF:S:T:37:scaler_t1");
	MYA::ChannelKey sou_tof_38(ap,"TOF:S:T:38:scaler_t1");
	MYA::ChannelKey sou_tof_39(ap,"TOF:S:T:39:scaler_t1");
	MYA::ChannelKey sou_tof_40(ap,"TOF:S:T:40:scaler_t1");
	MYA::ChannelKey sou_tof_41(ap,"TOF:S:T:41:scaler_t1");
	MYA::ChannelKey sou_tof_42(ap,"TOF:S:T:42:scaler_t1");
	MYA::ChannelKey sou_tof_43(ap,"TOF:S:T:43:scaler_t1");
	MYA::ChannelKey sou_tof_44(ap,"TOF:S:T:44:scaler_t1");
		// TOP:
	MYA::ChannelKey top_tof_1(ap,"TOF:UP:T:1:scaler_t1");
	MYA::ChannelKey top_tof_2(ap,"TOF:UP:T:2:scaler_t1");
	MYA::ChannelKey top_tof_3(ap,"TOF:UP:T:3:scaler_t1");
	MYA::ChannelKey top_tof_4(ap,"TOF:UP:T:4:scaler_t1");
	MYA::ChannelKey top_tof_5(ap,"TOF:UP:T:5:scaler_t1");
	MYA::ChannelKey top_tof_6(ap,"TOF:UP:T:6:scaler_t1");
	MYA::ChannelKey top_tof_7(ap,"TOF:UP:T:7:scaler_t1");
	MYA::ChannelKey top_tof_8(ap,"TOF:UP:T:8:scaler_t1");
	MYA::ChannelKey top_tof_9(ap,"TOF:UP:T:9:scaler_t1");
	MYA::ChannelKey top_tof_10(ap,"TOF:UP:T:10:scaler_t1");
	MYA::ChannelKey top_tof_11(ap,"TOF:UP:T:11:scaler_t1");
	MYA::ChannelKey top_tof_12(ap,"TOF:UP:T:12:scaler_t1");
	MYA::ChannelKey top_tof_13(ap,"TOF:UP:T:13:scaler_t1");
	MYA::ChannelKey top_tof_14(ap,"TOF:UP:T:14:scaler_t1");
	MYA::ChannelKey top_tof_15(ap,"TOF:UP:T:15:scaler_t1");
	MYA::ChannelKey top_tof_16(ap,"TOF:UP:T:16:scaler_t1");
	MYA::ChannelKey top_tof_17(ap,"TOF:UP:T:17:scaler_t1");
	MYA::ChannelKey top_tof_18(ap,"TOF:UP:T:18:scaler_t1");
	MYA::ChannelKey top_tof_19(ap,"TOF:UP:T:19:scaler_t1");
	MYA::ChannelKey top_tof_20(ap,"TOF:UP:T:20:scaler_t1");
	MYA::ChannelKey top_tof_21(ap,"TOF:UP:T:21:scaler_t1");
	MYA::ChannelKey top_tof_22(ap,"TOF:UP:T:22:scaler_t1");
	MYA::ChannelKey top_tof_23(ap,"TOF:UP:T:23:scaler_t1");
	MYA::ChannelKey top_tof_24(ap,"TOF:UP:T:24:scaler_t1");
	MYA::ChannelKey top_tof_25(ap,"TOF:UP:T:25:scaler_t1");
	MYA::ChannelKey top_tof_26(ap,"TOF:UP:T:26:scaler_t1");
	MYA::ChannelKey top_tof_27(ap,"TOF:UP:T:27:scaler_t1");
	MYA::ChannelKey top_tof_28(ap,"TOF:UP:T:28:scaler_t1");
	MYA::ChannelKey top_tof_29(ap,"TOF:UP:T:29:scaler_t1");
	MYA::ChannelKey top_tof_30(ap,"TOF:UP:T:30:scaler_t1");
	MYA::ChannelKey top_tof_31(ap,"TOF:UP:T:31:scaler_t1");
	MYA::ChannelKey top_tof_32(ap,"TOF:UP:T:32:scaler_t1");
	MYA::ChannelKey top_tof_33(ap,"TOF:UP:T:33:scaler_t1");
	MYA::ChannelKey top_tof_34(ap,"TOF:UP:T:34:scaler_t1");
	MYA::ChannelKey top_tof_35(ap,"TOF:UP:T:35:scaler_t1");
	MYA::ChannelKey top_tof_36(ap,"TOF:UP:T:36:scaler_t1");
	MYA::ChannelKey top_tof_37(ap,"TOF:UP:T:37:scaler_t1");
	MYA::ChannelKey top_tof_38(ap,"TOF:UP:T:38:scaler_t1");
	MYA::ChannelKey top_tof_39(ap,"TOF:UP:T:39:scaler_t1");
	MYA::ChannelKey top_tof_40(ap,"TOF:UP:T:40:scaler_t1");
	MYA::ChannelKey top_tof_41(ap,"TOF:UP:T:41:scaler_t1");
	MYA::ChannelKey top_tof_42(ap,"TOF:UP:T:42:scaler_t1");
	MYA::ChannelKey top_tof_43(ap,"TOF:UP:T:43:scaler_t1");
	MYA::ChannelKey top_tof_44(ap,"TOF:UP:T:44:scaler_t1");
		// BOTTOM:
	MYA::ChannelKey bot_tof_1(ap,"TOF:DW:T:1:scaler_t1");
	MYA::ChannelKey bot_tof_2(ap,"TOF:DW:T:2:scaler_t1");
	MYA::ChannelKey bot_tof_3(ap,"TOF:DW:T:3:scaler_t1");
	MYA::ChannelKey bot_tof_4(ap,"TOF:DW:T:4:scaler_t1");
	MYA::ChannelKey bot_tof_5(ap,"TOF:DW:T:5:scaler_t1");
	MYA::ChannelKey bot_tof_6(ap,"TOF:DW:T:6:scaler_t1");
	MYA::ChannelKey bot_tof_7(ap,"TOF:DW:T:7:scaler_t1");
	MYA::ChannelKey bot_tof_8(ap,"TOF:DW:T:8:scaler_t1");
	MYA::ChannelKey bot_tof_9(ap,"TOF:DW:T:9:scaler_t1");
	MYA::ChannelKey bot_tof_10(ap,"TOF:DW:T:10:scaler_t1");
	MYA::ChannelKey bot_tof_11(ap,"TOF:DW:T:11:scaler_t1");
	MYA::ChannelKey bot_tof_12(ap,"TOF:DW:T:12:scaler_t1");
	MYA::ChannelKey bot_tof_13(ap,"TOF:DW:T:13:scaler_t1");
	MYA::ChannelKey bot_tof_14(ap,"TOF:DW:T:14:scaler_t1");
	MYA::ChannelKey bot_tof_15(ap,"TOF:DW:T:15:scaler_t1");
	MYA::ChannelKey bot_tof_16(ap,"TOF:DW:T:16:scaler_t1");
	MYA::ChannelKey bot_tof_17(ap,"TOF:DW:T:17:scaler_t1");
	MYA::ChannelKey bot_tof_18(ap,"TOF:DW:T:18:scaler_t1");
	MYA::ChannelKey bot_tof_19(ap,"TOF:DW:T:19:scaler_t1");
	MYA::ChannelKey bot_tof_20(ap,"TOF:DW:T:20:scaler_t1");
	MYA::ChannelKey bot_tof_21(ap,"TOF:DW:T:21:scaler_t1");
	MYA::ChannelKey bot_tof_22(ap,"TOF:DW:T:22:scaler_t1");
	MYA::ChannelKey bot_tof_23(ap,"TOF:DW:T:23:scaler_t1");
	MYA::ChannelKey bot_tof_24(ap,"TOF:DW:T:24:scaler_t1");
	MYA::ChannelKey bot_tof_25(ap,"TOF:DW:T:25:scaler_t1");
	MYA::ChannelKey bot_tof_26(ap,"TOF:DW:T:26:scaler_t1");
	MYA::ChannelKey bot_tof_27(ap,"TOF:DW:T:27:scaler_t1");
	MYA::ChannelKey bot_tof_28(ap,"TOF:DW:T:28:scaler_t1");
	MYA::ChannelKey bot_tof_29(ap,"TOF:DW:T:29:scaler_t1");
	MYA::ChannelKey bot_tof_30(ap,"TOF:DW:T:30:scaler_t1");
	MYA::ChannelKey bot_tof_31(ap,"TOF:DW:T:31:scaler_t1");
	MYA::ChannelKey bot_tof_32(ap,"TOF:DW:T:32:scaler_t1");
	MYA::ChannelKey bot_tof_33(ap,"TOF:DW:T:33:scaler_t1");
	MYA::ChannelKey bot_tof_34(ap,"TOF:DW:T:34:scaler_t1");
	MYA::ChannelKey bot_tof_35(ap,"TOF:DW:T:35:scaler_t1");
	MYA::ChannelKey bot_tof_36(ap,"TOF:DW:T:36:scaler_t1");
	MYA::ChannelKey bot_tof_37(ap,"TOF:DW:T:37:scaler_t1");
	MYA::ChannelKey bot_tof_38(ap,"TOF:DW:T:38:scaler_t1");
	MYA::ChannelKey bot_tof_39(ap,"TOF:DW:T:39:scaler_t1");
	MYA::ChannelKey bot_tof_40(ap,"TOF:DW:T:40:scaler_t1");
	MYA::ChannelKey bot_tof_41(ap,"TOF:DW:T:41:scaler_t1");
	MYA::ChannelKey bot_tof_42(ap,"TOF:DW:T:42:scaler_t1");
	MYA::ChannelKey bot_tof_43(ap,"TOF:DW:T:43:scaler_t1");
	MYA::ChannelKey bot_tof_44(ap,"TOF:DW:T:44:scaler_t1");

	// Get EPICS Eras
	// NORTH:
	MYA::Era<float> nor_tof_1_era(nor_tof_1);
	MYA::Era<float> nor_tof_2_era(nor_tof_2);
	MYA::Era<float> nor_tof_3_era(nor_tof_3);
	MYA::Era<float> nor_tof_4_era(nor_tof_4);
	MYA::Era<float> nor_tof_5_era(nor_tof_5);
	MYA::Era<float> nor_tof_6_era(nor_tof_6);
	MYA::Era<float> nor_tof_7_era(nor_tof_7);
	MYA::Era<float> nor_tof_8_era(nor_tof_8);
	MYA::Era<float> nor_tof_9_era(nor_tof_9);
	MYA::Era<float> nor_tof_10_era(nor_tof_10);
	MYA::Era<float> nor_tof_11_era(nor_tof_11);
	MYA::Era<float> nor_tof_12_era(nor_tof_12);
	MYA::Era<float> nor_tof_13_era(nor_tof_13);
	MYA::Era<float> nor_tof_14_era(nor_tof_14);
	MYA::Era<float> nor_tof_15_era(nor_tof_15);
	MYA::Era<float> nor_tof_16_era(nor_tof_16);
	MYA::Era<float> nor_tof_17_era(nor_tof_17);
	MYA::Era<float> nor_tof_18_era(nor_tof_18);
	MYA::Era<float> nor_tof_19_era(nor_tof_19);
	MYA::Era<float> nor_tof_20_era(nor_tof_20);
	MYA::Era<float> nor_tof_21_era(nor_tof_21);
	MYA::Era<float> nor_tof_22_era(nor_tof_22);
	MYA::Era<float> nor_tof_23_era(nor_tof_23);
	MYA::Era<float> nor_tof_24_era(nor_tof_24);
	MYA::Era<float> nor_tof_25_era(nor_tof_25);
	MYA::Era<float> nor_tof_26_era(nor_tof_26);
	MYA::Era<float> nor_tof_27_era(nor_tof_27);
	MYA::Era<float> nor_tof_28_era(nor_tof_28);
	MYA::Era<float> nor_tof_29_era(nor_tof_29);
	MYA::Era<float> nor_tof_30_era(nor_tof_30);
	MYA::Era<float> nor_tof_31_era(nor_tof_31);
	MYA::Era<float> nor_tof_32_era(nor_tof_32);
	MYA::Era<float> nor_tof_33_era(nor_tof_33);
	MYA::Era<float> nor_tof_34_era(nor_tof_34);
	MYA::Era<float> nor_tof_35_era(nor_tof_35);
	MYA::Era<float> nor_tof_36_era(nor_tof_36);
	MYA::Era<float> nor_tof_37_era(nor_tof_37);
	MYA::Era<float> nor_tof_38_era(nor_tof_38);
	MYA::Era<float> nor_tof_39_era(nor_tof_39);
	MYA::Era<float> nor_tof_40_era(nor_tof_40);
	MYA::Era<float> nor_tof_41_era(nor_tof_41);
	MYA::Era<float> nor_tof_42_era(nor_tof_42);
	MYA::Era<float> nor_tof_43_era(nor_tof_43);
	MYA::Era<float> nor_tof_44_era(nor_tof_44);
	// SOUTH:
	MYA::Era<float> sou_tof_1_era(sou_tof_1);
	MYA::Era<float> sou_tof_2_era(sou_tof_2);
	MYA::Era<float> sou_tof_3_era(sou_tof_3);
	MYA::Era<float> sou_tof_4_era(sou_tof_4);
	MYA::Era<float> sou_tof_5_era(sou_tof_5);
	MYA::Era<float> sou_tof_6_era(sou_tof_6);
	MYA::Era<float> sou_tof_7_era(sou_tof_7);
	MYA::Era<float> sou_tof_8_era(sou_tof_8);
	MYA::Era<float> sou_tof_9_era(sou_tof_9);
	MYA::Era<float> sou_tof_10_era(sou_tof_10);
	MYA::Era<float> sou_tof_11_era(sou_tof_11);
	MYA::Era<float> sou_tof_12_era(sou_tof_12);
	MYA::Era<float> sou_tof_13_era(sou_tof_13);
	MYA::Era<float> sou_tof_14_era(sou_tof_14);
	MYA::Era<float> sou_tof_15_era(sou_tof_15);
	MYA::Era<float> sou_tof_16_era(sou_tof_16);
	MYA::Era<float> sou_tof_17_era(sou_tof_17);
	MYA::Era<float> sou_tof_18_era(sou_tof_18);
	MYA::Era<float> sou_tof_19_era(sou_tof_19);
	MYA::Era<float> sou_tof_20_era(sou_tof_20);
	MYA::Era<float> sou_tof_21_era(sou_tof_21);
	MYA::Era<float> sou_tof_22_era(sou_tof_22);
	MYA::Era<float> sou_tof_23_era(sou_tof_23);
	MYA::Era<float> sou_tof_24_era(sou_tof_24);
	MYA::Era<float> sou_tof_25_era(sou_tof_25);
	MYA::Era<float> sou_tof_26_era(sou_tof_26);
	MYA::Era<float> sou_tof_27_era(sou_tof_27);
	MYA::Era<float> sou_tof_28_era(sou_tof_28);
	MYA::Era<float> sou_tof_29_era(sou_tof_29);
	MYA::Era<float> sou_tof_30_era(sou_tof_30);
	MYA::Era<float> sou_tof_31_era(sou_tof_31);
	MYA::Era<float> sou_tof_32_era(sou_tof_32);
	MYA::Era<float> sou_tof_33_era(sou_tof_33);
	MYA::Era<float> sou_tof_34_era(sou_tof_34);
	MYA::Era<float> sou_tof_35_era(sou_tof_35);
	MYA::Era<float> sou_tof_36_era(sou_tof_36);
	MYA::Era<float> sou_tof_37_era(sou_tof_37);
	MYA::Era<float> sou_tof_38_era(sou_tof_38);
	MYA::Era<float> sou_tof_39_era(sou_tof_39);
	MYA::Era<float> sou_tof_40_era(sou_tof_40);
	MYA::Era<float> sou_tof_41_era(sou_tof_41);
	MYA::Era<float> sou_tof_42_era(sou_tof_42);
	MYA::Era<float> sou_tof_43_era(sou_tof_43);
	MYA::Era<float> sou_tof_44_era(sou_tof_44);
	// TOP:
	MYA::Era<float> top_tof_1_era(top_tof_1);
	MYA::Era<float> top_tof_2_era(top_tof_2);
	MYA::Era<float> top_tof_3_era(top_tof_3);
	MYA::Era<float> top_tof_4_era(top_tof_4);
	MYA::Era<float> top_tof_5_era(top_tof_5);
	MYA::Era<float> top_tof_6_era(top_tof_6);
	MYA::Era<float> top_tof_7_era(top_tof_7);
	MYA::Era<float> top_tof_8_era(top_tof_8);
	MYA::Era<float> top_tof_9_era(top_tof_9);
	MYA::Era<float> top_tof_10_era(top_tof_10);
	MYA::Era<float> top_tof_11_era(top_tof_11);
	MYA::Era<float> top_tof_12_era(top_tof_12);
	MYA::Era<float> top_tof_13_era(top_tof_13);
	MYA::Era<float> top_tof_14_era(top_tof_14);
	MYA::Era<float> top_tof_15_era(top_tof_15);
	MYA::Era<float> top_tof_16_era(top_tof_16);
	MYA::Era<float> top_tof_17_era(top_tof_17);
	MYA::Era<float> top_tof_18_era(top_tof_18);
	MYA::Era<float> top_tof_19_era(top_tof_19);
	MYA::Era<float> top_tof_20_era(top_tof_20);
	MYA::Era<float> top_tof_21_era(top_tof_21);
	MYA::Era<float> top_tof_22_era(top_tof_22);
	MYA::Era<float> top_tof_23_era(top_tof_23);
	MYA::Era<float> top_tof_24_era(top_tof_24);
	MYA::Era<float> top_tof_25_era(top_tof_25);
	MYA::Era<float> top_tof_26_era(top_tof_26);
	MYA::Era<float> top_tof_27_era(top_tof_27);
	MYA::Era<float> top_tof_28_era(top_tof_28);
	MYA::Era<float> top_tof_29_era(top_tof_29);
	MYA::Era<float> top_tof_30_era(top_tof_30);
	MYA::Era<float> top_tof_31_era(top_tof_31);
	MYA::Era<float> top_tof_32_era(top_tof_32);
	MYA::Era<float> top_tof_33_era(top_tof_33);
	MYA::Era<float> top_tof_34_era(top_tof_34);
	MYA::Era<float> top_tof_35_era(top_tof_35);
	MYA::Era<float> top_tof_36_era(top_tof_36);
	MYA::Era<float> top_tof_37_era(top_tof_37);
	MYA::Era<float> top_tof_38_era(top_tof_38);
	MYA::Era<float> top_tof_39_era(top_tof_39);
	MYA::Era<float> top_tof_40_era(top_tof_40);
	MYA::Era<float> top_tof_41_era(top_tof_41);
	MYA::Era<float> top_tof_42_era(top_tof_42);
	MYA::Era<float> top_tof_43_era(top_tof_43);
	MYA::Era<float> top_tof_44_era(top_tof_44);
	// BOTTOM:
	MYA::Era<float> bot_tof_1_era(bot_tof_1);
	MYA::Era<float> bot_tof_2_era(bot_tof_2);
	MYA::Era<float> bot_tof_3_era(bot_tof_3);
	MYA::Era<float> bot_tof_4_era(bot_tof_4);
	MYA::Era<float> bot_tof_5_era(bot_tof_5);
	MYA::Era<float> bot_tof_6_era(bot_tof_6);
	MYA::Era<float> bot_tof_7_era(bot_tof_7);
	MYA::Era<float> bot_tof_8_era(bot_tof_8);
	MYA::Era<float> bot_tof_9_era(bot_tof_9);
	MYA::Era<float> bot_tof_10_era(bot_tof_10);
	MYA::Era<float> bot_tof_11_era(bot_tof_11);
	MYA::Era<float> bot_tof_12_era(bot_tof_12);
	MYA::Era<float> bot_tof_13_era(bot_tof_13);
	MYA::Era<float> bot_tof_14_era(bot_tof_14);
	MYA::Era<float> bot_tof_15_era(bot_tof_15);
	MYA::Era<float> bot_tof_16_era(bot_tof_16);
	MYA::Era<float> bot_tof_17_era(bot_tof_17);
	MYA::Era<float> bot_tof_18_era(bot_tof_18);
	MYA::Era<float> bot_tof_19_era(bot_tof_19);
	MYA::Era<float> bot_tof_20_era(bot_tof_20);
	MYA::Era<float> bot_tof_21_era(bot_tof_21);
	MYA::Era<float> bot_tof_22_era(bot_tof_22);
	MYA::Era<float> bot_tof_23_era(bot_tof_23);
	MYA::Era<float> bot_tof_24_era(bot_tof_24);
	MYA::Era<float> bot_tof_25_era(bot_tof_25);
	MYA::Era<float> bot_tof_26_era(bot_tof_26);
	MYA::Era<float> bot_tof_27_era(bot_tof_27);
	MYA::Era<float> bot_tof_28_era(bot_tof_28);
	MYA::Era<float> bot_tof_29_era(bot_tof_29);
	MYA::Era<float> bot_tof_30_era(bot_tof_30);
	MYA::Era<float> bot_tof_31_era(bot_tof_31);
	MYA::Era<float> bot_tof_32_era(bot_tof_32);
	MYA::Era<float> bot_tof_33_era(bot_tof_33);
	MYA::Era<float> bot_tof_34_era(bot_tof_34);
	MYA::Era<float> bot_tof_35_era(bot_tof_35);
	MYA::Era<float> bot_tof_36_era(bot_tof_36);
	MYA::Era<float> bot_tof_37_era(bot_tof_37);
	MYA::Era<float> bot_tof_38_era(bot_tof_38);
	MYA::Era<float> bot_tof_39_era(bot_tof_39);
	MYA::Era<float> bot_tof_40_era(bot_tof_40);
	MYA::Era<float> bot_tof_41_era(bot_tof_41);
	MYA::Era<float> bot_tof_42_era(bot_tof_42);
	MYA::Era<float> bot_tof_43_era(bot_tof_43);
	MYA::Era<float> bot_tof_44_era(bot_tof_44);

	// Name the root file and cd into it
	TFile *ROOTFile;
	ROOTFile = new TFile("TOF_scalers.root","recreate");
	ROOTFile->cd();

	// Make all runs variables
	bool all_runs_flag = false;
	if (event_time.size() > 1)
		all_runs_flag = true;
	map<int,float> all_runs_beam_current, pmt_scaler_avg_all_runs[4][44];

	// Load the EPICS start and stop times
	for (vector< map< unsigned int,map<string,string> > >::iterator event_time_it=event_time.begin();event_time_it!=event_time.end();++event_time_it) {

		map< unsigned int, map<string,string> > run_time = *event_time_it;
		for (map< unsigned int, map<string,string> >::iterator run_time_it=run_time.begin();run_time_it!=run_time.end();++run_time_it) {
			unsigned int runnumber = run_time_it->first;
			map<string,string> time = run_time_it->second;
			string start_time, end_time;
			cout << "ANALYZING RUN NUMBER: " << runnumber << endl;
			for (map<string,string>::iterator time_it=time.begin();time_it!=time.end();++time_it) {
				start_time = time_it->first;
				end_time = time_it->second;
				MYA::MyaTime beginEPICStime(cpplib::StrToSec(start_time));
				MYA::MyaTime endEPICStime(cpplib::StrToSec(end_time));

				// Fetch the eras
				current_era.Get(beginEPICStime, endEPICStime, 0, true);
				// NORTH:
				nor_tof_1_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_2_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_3_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_4_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_5_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_6_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_7_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_8_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_9_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_10_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_11_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_12_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_13_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_14_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_15_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_16_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_17_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_18_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_19_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_20_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_21_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_22_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_23_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_24_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_25_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_26_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_27_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_28_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_29_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_30_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_31_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_32_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_33_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_34_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_35_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_36_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_37_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_38_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_39_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_40_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_41_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_42_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_43_era.Get(beginEPICStime, endEPICStime, 0, true);
				nor_tof_44_era.Get(beginEPICStime, endEPICStime, 0, true);
				// SOUTH:
				sou_tof_1_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_2_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_3_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_4_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_5_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_6_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_7_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_8_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_9_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_10_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_11_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_12_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_13_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_14_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_15_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_16_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_17_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_18_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_19_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_20_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_21_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_22_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_23_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_24_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_25_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_26_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_27_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_28_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_29_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_30_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_31_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_32_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_33_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_34_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_35_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_36_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_37_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_38_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_39_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_40_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_41_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_42_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_43_era.Get(beginEPICStime, endEPICStime, 0, true);
				sou_tof_44_era.Get(beginEPICStime, endEPICStime, 0, true);
				// TOP:
				top_tof_1_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_2_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_3_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_4_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_5_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_6_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_7_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_8_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_9_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_10_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_11_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_12_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_13_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_14_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_15_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_16_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_17_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_18_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_19_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_20_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_21_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_22_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_23_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_24_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_25_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_26_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_27_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_28_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_29_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_30_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_31_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_32_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_33_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_34_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_35_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_36_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_37_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_38_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_39_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_40_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_41_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_42_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_43_era.Get(beginEPICStime, endEPICStime, 0, true);
				top_tof_44_era.Get(beginEPICStime, endEPICStime, 0, true);
				// BOTTOM:
				bot_tof_1_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_2_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_3_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_4_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_5_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_6_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_7_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_8_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_9_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_10_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_11_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_12_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_13_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_14_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_15_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_16_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_17_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_18_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_19_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_20_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_21_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_22_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_23_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_24_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_25_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_26_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_27_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_28_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_29_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_30_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_31_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_32_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_33_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_34_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_35_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_36_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_37_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_38_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_39_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_40_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_41_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_42_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_43_era.Get(beginEPICStime, endEPICStime, 0, true);
				bot_tof_44_era.Get(beginEPICStime, endEPICStime, 0, true);
			} // for map<string,string>::iterator time_it

			// Make Runnumber Directory
			char directory_name[128];
			sprintf(directory_name,"Run %d",runnumber);
			TDirectory * runnumber_dir = ROOTFile->mkdir(directory_name);
			runnumber_dir->cd();
			TDirectory * tof_dir;

			// Get beam current data
			char TCanvas_beam_name[128];
			sprintf(TCanvas_beam_name,"Run %d Beam Current",runnumber);
			TCanvas * beam_canvas = new TCanvas(TCanvas_beam_name,TCanvas_beam_name,200,10,700,500); 
			float beam_x[current_era().size()], beam_y[current_era().size()];
			for (unsigned era_it=0;era_it<current_era().size();++era_it)
			{
				MYA::Event<float> *event = current_era()[era_it];
				MYA::MyaTime eventTime = event->Timestamp();
				TDatime event_time((eventTime.ToString()).c_str());
				beam_x[era_it] = event_time.Convert();
				beam_y[era_it] = event->Value(0);
			} // for unsigned era_it

			// Plot the beam current
			char beam_axis_name[128];
			beam_canvas->cd();
			TGraph * beam_graph = new TGraph(current_era().size(),beam_x,beam_y);
			beam_graph->SetTitle("Beam Current");
			beam_graph->SetMarkerStyle(20);
			beam_graph->Draw("AP");
			beam_graph->GetXaxis()->SetTimeDisplay(1);
   		beam_graph->GetXaxis()->SetNdivisions(-503);
   		beam_graph->GetXaxis()->SetTimeFormat("%m-%d %H:%M");
			sprintf(beam_axis_name,"Run %d",runnumber);
			beam_graph->GetXaxis()->SetTitle(beam_axis_name);
			beam_graph->GetYaxis()->SetTitle("Beam Current [nA]");
			if (all_runs_flag) 
					all_runs_beam_current[runnumber] = beam_graph->GetYaxis()->GetXmax()*0.7653;
			beam_canvas->Update();
			beam_canvas->Draw();
			beam_canvas->Write();

			// Make array for pmt scaler averages
			float y_pmt_scaler_averages[4][44];

			// Put hists inside directory
			for (unsigned int direction_num = 1; direction_num<5; ++direction_num) {
				sprintf(directory_name,"TOF Scaler");
// North
				if (direction_num==1) {
					sprintf(directory_name,"%s N",directory_name); 
					tof_dir = runnumber_dir->mkdir(directory_name);
					tof_dir->cd();
					for (unsigned int pmt_num = 1; pmt_num<45; ++pmt_num) {

						// Make the Canvas
						char TCanvas_name[128];
						sprintf(TCanvas_name,"Run %d %s %d",runnumber,directory_name,pmt_num);
						TCanvas * temp_canvas = new TCanvas(TCanvas_name,TCanvas_name,200,10,700,500); 

						// Loop over the data
						unsigned beam_check = 0;
						vector<float> x_vec, y_vec;
						switch (pmt_num) {
							case 1:
								for (unsigned era_it=0;era_it<nor_tof_1_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_1_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 2:
								for (unsigned era_it=0;era_it<nor_tof_2_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_2_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 3:
								for (unsigned era_it=0;era_it<nor_tof_3_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_3_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 4:
								for (unsigned era_it=0;era_it<nor_tof_4_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_4_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 5:
								for (unsigned era_it=0;era_it<nor_tof_5_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_5_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 6:
								for (unsigned era_it=0;era_it<nor_tof_6_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_6_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 7:
								for (unsigned era_it=0;era_it<nor_tof_7_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_7_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 8:
								for (unsigned era_it=0;era_it<nor_tof_8_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_8_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 9:
								for (unsigned era_it=0;era_it<nor_tof_9_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_9_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 10:
								for (unsigned era_it=0;era_it<nor_tof_10_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_10_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 11:
								for (unsigned era_it=0;era_it<nor_tof_11_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_11_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 12:
								for (unsigned era_it=0;era_it<nor_tof_12_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_12_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 13:
								for (unsigned era_it=0;era_it<nor_tof_13_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_13_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 14:
								for (unsigned era_it=0;era_it<nor_tof_14_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_14_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 15:
								for (unsigned era_it=0;era_it<nor_tof_15_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_15_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 16:
								for (unsigned era_it=0;era_it<nor_tof_16_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_16_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 17:
								for (unsigned era_it=0;era_it<nor_tof_17_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_17_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 18:
								for (unsigned era_it=0;era_it<nor_tof_18_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_18_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 19:
								for (unsigned era_it=0;era_it<nor_tof_19_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_19_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;			
							case 20:
								for (unsigned era_it=0;era_it<nor_tof_20_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_20_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 21:
								for (unsigned era_it=0;era_it<nor_tof_21_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_21_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 22:
								for (unsigned era_it=0;era_it<nor_tof_22_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_22_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 23:
								for (unsigned era_it=0;era_it<nor_tof_23_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_23_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 24:
								for (unsigned era_it=0;era_it<nor_tof_24_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_24_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 25:
								for (unsigned era_it=0;era_it<nor_tof_25_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_25_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 26:
								for (unsigned era_it=0;era_it<nor_tof_26_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_26_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 27:
								for (unsigned era_it=0;era_it<nor_tof_27_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_27_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 28:
								for (unsigned era_it=0;era_it<nor_tof_28_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_28_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 29:
								for (unsigned era_it=0;era_it<nor_tof_29_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_29_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 30:
								for (unsigned era_it=0;era_it<nor_tof_30_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_30_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 31:
								for (unsigned era_it=0;era_it<nor_tof_31_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_31_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 32:
								for (unsigned era_it=0;era_it<nor_tof_32_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_32_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 33:
								for (unsigned era_it=0;era_it<nor_tof_33_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_33_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 34:
								for (unsigned era_it=0;era_it<nor_tof_34_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_34_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 35:
								for (unsigned era_it=0;era_it<nor_tof_35_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_35_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 36:
								for (unsigned era_it=0;era_it<nor_tof_36_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_36_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 37:
								for (unsigned era_it=0;era_it<nor_tof_37_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_37_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 38:
								for (unsigned era_it=0;era_it<nor_tof_38_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_38_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 39:
								for (unsigned era_it=0;era_it<nor_tof_39_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_39_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 40:
								for (unsigned era_it=0;era_it<nor_tof_40_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_40_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 41:
								for (unsigned era_it=0;era_it<nor_tof_41_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_41_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 42:
								for (unsigned era_it=0;era_it<nor_tof_42_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_42_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 43:
								for (unsigned era_it=0;era_it<nor_tof_43_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_43_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 44:
								for (unsigned era_it=0;era_it<nor_tof_44_era().size();++era_it)
								{
									MYA::Event<float> *event = nor_tof_44_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
						} // switch(pmt_num)
						
						// Load the vector values into arrays
						float x[x_vec.size()],y[y_vec.size()];
						for (unsigned vec_it=0;vec_it<x_vec.size();++vec_it) {
							x[vec_it] = x_vec.at(vec_it);
							y[vec_it] = y_vec.at(vec_it);
						}

						// Draw the data
						char axis_name[128];
						temp_canvas->cd();
						TGraph * temp_graph = new TGraph(x_vec.size(),x,y);
						temp_graph->SetTitle(TCanvas_name);
						temp_graph->SetMarkerStyle(20);
						temp_graph->Draw("AP");
						temp_graph->GetXaxis()->SetTimeDisplay(1);
   					temp_graph->GetXaxis()->SetNdivisions(-503);
   					temp_graph->GetXaxis()->SetTimeFormat("%m-%d %H:%M");
						sprintf(axis_name,"Run %d",runnumber);
						temp_graph->GetXaxis()->SetTitle(axis_name);
						sprintf(axis_name,"TOF Scalers [scaler/nA]");
						temp_graph->GetYaxis()->SetTitle(axis_name);
						y_pmt_scaler_averages[direction_num-1][pmt_num-1] = temp_graph->GetYaxis()->GetXmax()*0.8125;
						if (all_runs_flag) 
							(pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1])[runnumber] = temp_graph->GetYaxis()->GetXmax()*0.8125;
						temp_canvas->Update();
						temp_canvas->Draw();
						temp_canvas->Write();
					} // for unsigned int pmt_num
				} // if North
// South
				if (direction_num==2)	{
					sprintf(directory_name,"%s S",directory_name); 
					tof_dir = runnumber_dir->mkdir(directory_name);
					tof_dir->cd();

					for (unsigned int pmt_num = 1; pmt_num<45; ++pmt_num) {

						// Make the Canvas
						char TCanvas_name[128];
						sprintf(TCanvas_name,"Run %d %s %d",runnumber,directory_name,pmt_num);
						TCanvas * temp_canvas = new TCanvas(TCanvas_name,TCanvas_name,200,10,700,500); 

						// Loop over the data
						unsigned beam_check = 0;
						vector<float> x_vec, y_vec;
						switch (pmt_num) {
							case 1:
								for (unsigned era_it=0;era_it<sou_tof_1_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_1_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 2:
								for (unsigned era_it=0;era_it<sou_tof_2_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_2_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 3:
								for (unsigned era_it=0;era_it<sou_tof_3_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_3_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 4:
								for (unsigned era_it=0;era_it<sou_tof_4_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_4_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 5:
								for (unsigned era_it=0;era_it<sou_tof_5_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_5_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 6:
								for (unsigned era_it=0;era_it<sou_tof_6_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_6_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 7:
								for (unsigned era_it=0;era_it<sou_tof_7_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_7_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 8:
								for (unsigned era_it=0;era_it<sou_tof_8_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_8_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 9:
								for (unsigned era_it=0;era_it<sou_tof_9_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_9_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 10:
								for (unsigned era_it=0;era_it<sou_tof_10_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_10_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 11:
								for (unsigned era_it=0;era_it<sou_tof_11_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_11_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 12:
								for (unsigned era_it=0;era_it<sou_tof_12_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_12_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 13:
								for (unsigned era_it=0;era_it<sou_tof_13_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_13_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 14:
								for (unsigned era_it=0;era_it<sou_tof_14_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_14_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 15:
								for (unsigned era_it=0;era_it<sou_tof_15_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_15_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 16:
								for (unsigned era_it=0;era_it<sou_tof_16_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_16_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 17:
								for (unsigned era_it=0;era_it<sou_tof_17_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_17_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 18:
								for (unsigned era_it=0;era_it<sou_tof_18_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_18_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 19:
								for (unsigned era_it=0;era_it<sou_tof_19_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_19_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;			
							case 20:
								for (unsigned era_it=0;era_it<sou_tof_20_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_20_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 21:
								for (unsigned era_it=0;era_it<sou_tof_21_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_21_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 22:
								for (unsigned era_it=0;era_it<sou_tof_22_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_22_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 23:
								for (unsigned era_it=0;era_it<sou_tof_23_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_23_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 24:
								for (unsigned era_it=0;era_it<sou_tof_24_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_24_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 25:
								for (unsigned era_it=0;era_it<sou_tof_25_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_25_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 26:
								for (unsigned era_it=0;era_it<sou_tof_26_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_26_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 27:
								for (unsigned era_it=0;era_it<sou_tof_27_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_27_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 28:
								for (unsigned era_it=0;era_it<sou_tof_28_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_28_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 29:
								for (unsigned era_it=0;era_it<sou_tof_29_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_29_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 30:
								for (unsigned era_it=0;era_it<sou_tof_30_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_30_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 31:
								for (unsigned era_it=0;era_it<sou_tof_31_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_31_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 32:
								for (unsigned era_it=0;era_it<sou_tof_32_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_32_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 33:
								for (unsigned era_it=0;era_it<sou_tof_33_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_33_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 34:
								for (unsigned era_it=0;era_it<sou_tof_34_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_34_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 35:
								for (unsigned era_it=0;era_it<sou_tof_35_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_35_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 36:
								for (unsigned era_it=0;era_it<sou_tof_36_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_36_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 37:
								for (unsigned era_it=0;era_it<sou_tof_37_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_37_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 38:
								for (unsigned era_it=0;era_it<sou_tof_38_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_38_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 39:
								for (unsigned era_it=0;era_it<sou_tof_39_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_39_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 40:
								for (unsigned era_it=0;era_it<sou_tof_40_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_40_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 41:
								for (unsigned era_it=0;era_it<sou_tof_41_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_41_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 42:
								for (unsigned era_it=0;era_it<sou_tof_42_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_42_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 43:
								for (unsigned era_it=0;era_it<sou_tof_43_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_43_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 44:
								for (unsigned era_it=0;era_it<sou_tof_44_era().size();++era_it)
								{
									MYA::Event<float> *event = sou_tof_44_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
						} // switch(pmt_num)

						// Load the vector values into arrays
						float x[x_vec.size()],y[y_vec.size()];
						for (unsigned vec_it=0;vec_it<x_vec.size();++vec_it) {
							x[vec_it] = x_vec.at(vec_it);
							y[vec_it] = y_vec.at(vec_it);
						}

						// Draw the data
						char axis_name[128];
						temp_canvas->cd();
						TGraph * temp_graph = new TGraph(x_vec.size(),x,y);
						temp_graph->SetTitle(TCanvas_name);
						temp_graph->SetMarkerStyle(20);
						temp_graph->Draw("AP");
						temp_graph->GetXaxis()->SetTimeDisplay(1);
   					temp_graph->GetXaxis()->SetNdivisions(-503);
   					temp_graph->GetXaxis()->SetTimeFormat("%m-%d %H:%M");
						sprintf(axis_name,"Run %d",runnumber);
						temp_graph->GetXaxis()->SetTitle(axis_name);
						sprintf(axis_name,"TOF Scalers [scaler/nA]");
						temp_graph->GetYaxis()->SetTitle(axis_name);
						y_pmt_scaler_averages[direction_num-1][pmt_num-1] = temp_graph->GetYaxis()->GetXmax()*0.8125;
						if (all_runs_flag) 
							(pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1])[runnumber] = temp_graph->GetYaxis()->GetXmax()*0.8125;
						temp_canvas->Update();
						temp_canvas->Draw();
						temp_canvas->Write();
					} // for unsigned int pmt_num
				} // if South
// TOP
				if (direction_num==3) {
					sprintf(directory_name,"%s T",directory_name);
					tof_dir = runnumber_dir->mkdir(directory_name);
					tof_dir->cd();
					for (unsigned int pmt_num = 1; pmt_num<45; ++pmt_num) {

						// Make the Canvas
						char TCanvas_name[128];
						sprintf(TCanvas_name,"Run %d %s %d",runnumber,directory_name,pmt_num);
						TCanvas * temp_canvas = new TCanvas(TCanvas_name,TCanvas_name,200,10,700,500); 

						// Loop over the data
						unsigned beam_check = 0;
						vector<float> x_vec, y_vec;
						switch (pmt_num) {
							case 1:
								for (unsigned era_it=0;era_it<top_tof_1_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_1_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 2:
								for (unsigned era_it=0;era_it<top_tof_2_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_2_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 3:
								for (unsigned era_it=0;era_it<top_tof_3_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_3_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 4:
								for (unsigned era_it=0;era_it<top_tof_4_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_4_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 5:
								for (unsigned era_it=0;era_it<top_tof_5_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_5_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 6:
								for (unsigned era_it=0;era_it<top_tof_6_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_6_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 7:
								for (unsigned era_it=0;era_it<top_tof_7_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_7_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 8:
								for (unsigned era_it=0;era_it<top_tof_8_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_8_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 9:
								for (unsigned era_it=0;era_it<top_tof_9_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_9_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 10:
								for (unsigned era_it=0;era_it<top_tof_10_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_10_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 11:
								for (unsigned era_it=0;era_it<top_tof_11_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_11_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 12:
								for (unsigned era_it=0;era_it<top_tof_12_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_12_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 13:
								for (unsigned era_it=0;era_it<top_tof_13_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_13_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 14:
								for (unsigned era_it=0;era_it<top_tof_14_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_14_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 15:
								for (unsigned era_it=0;era_it<top_tof_15_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_15_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 16:
								for (unsigned era_it=0;era_it<top_tof_16_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_16_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 17:
								for (unsigned era_it=0;era_it<top_tof_17_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_17_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 18:
								for (unsigned era_it=0;era_it<top_tof_18_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_18_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 19:
								for (unsigned era_it=0;era_it<top_tof_19_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_19_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;			
							case 20:
								for (unsigned era_it=0;era_it<top_tof_20_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_20_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 21:
								for (unsigned era_it=0;era_it<top_tof_21_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_21_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 22:
								for (unsigned era_it=0;era_it<top_tof_22_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_22_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 23:
								for (unsigned era_it=0;era_it<top_tof_23_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_23_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 24:
								for (unsigned era_it=0;era_it<top_tof_24_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_24_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 25:
								for (unsigned era_it=0;era_it<top_tof_25_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_25_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 26:
								for (unsigned era_it=0;era_it<top_tof_26_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_26_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 27:
								for (unsigned era_it=0;era_it<top_tof_27_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_27_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 28:
								for (unsigned era_it=0;era_it<top_tof_28_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_28_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 29:
								for (unsigned era_it=0;era_it<top_tof_29_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_29_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 30:
								for (unsigned era_it=0;era_it<top_tof_30_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_30_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 31:
								for (unsigned era_it=0;era_it<top_tof_31_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_31_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 32:
								for (unsigned era_it=0;era_it<top_tof_32_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_32_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 33:
								for (unsigned era_it=0;era_it<top_tof_33_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_33_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 34:
								for (unsigned era_it=0;era_it<top_tof_34_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_34_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 35:
								for (unsigned era_it=0;era_it<top_tof_35_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_35_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 36:
								for (unsigned era_it=0;era_it<top_tof_36_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_36_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 37:
								for (unsigned era_it=0;era_it<top_tof_37_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_37_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 38:
								for (unsigned era_it=0;era_it<top_tof_38_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_38_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 39:
								for (unsigned era_it=0;era_it<top_tof_39_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_39_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 40:
								for (unsigned era_it=0;era_it<top_tof_40_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_40_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 41:
								for (unsigned era_it=0;era_it<top_tof_41_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_41_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 42:
								for (unsigned era_it=0;era_it<top_tof_42_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_42_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 43:
								for (unsigned era_it=0;era_it<top_tof_43_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_43_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 44:
								for (unsigned era_it=0;era_it<top_tof_44_era().size();++era_it)
								{
									MYA::Event<float> *event = top_tof_44_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
						} // switch(pmt_num)

						// Load the vector values into arrays
						float x[x_vec.size()],y[y_vec.size()];
						for (unsigned vec_it=0;vec_it<x_vec.size();++vec_it) {
							x[vec_it] = x_vec.at(vec_it);
							y[vec_it] = y_vec.at(vec_it);
						}

						// Draw the data
						char axis_name[128];
						temp_canvas->cd();
						TGraph * temp_graph = new TGraph(x_vec.size(),x,y);
						temp_graph->SetTitle(TCanvas_name);
						temp_graph->SetMarkerStyle(20);
						temp_graph->Draw("AP");
						temp_graph->GetXaxis()->SetTimeDisplay(1);
   					temp_graph->GetXaxis()->SetNdivisions(-503);
   					temp_graph->GetXaxis()->SetTimeFormat("%m-%d %H:%M");
						sprintf(axis_name,"Run %d",runnumber);
						temp_graph->GetXaxis()->SetTitle(axis_name);
						sprintf(axis_name,"TOF Scalers [scaler/nA]");
						temp_graph->GetYaxis()->SetTitle(axis_name);
						y_pmt_scaler_averages[direction_num-1][pmt_num-1] = temp_graph->GetYaxis()->GetXmax()*0.8125;
						if (all_runs_flag) 
							(pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1])[runnumber] = temp_graph->GetYaxis()->GetXmax()*0.8125;
						temp_canvas->Update();
						temp_canvas->Draw();
						temp_canvas->Write();
					} // for unsigned int pmt_num
				} // if Top
// Bottom
				if (direction_num==4) {
					sprintf(directory_name,"%s B",directory_name);
					tof_dir = runnumber_dir->mkdir(directory_name);
					tof_dir->cd();
					for (unsigned int pmt_num = 1; pmt_num<45; ++pmt_num) {

						// Make the Canvas
						char TCanvas_name[128];
						sprintf(TCanvas_name,"Run %d %s %d",runnumber,directory_name,pmt_num);
						TCanvas * temp_canvas = new TCanvas(TCanvas_name,TCanvas_name,200,10,700,500); 

						// Loop over the data
						unsigned beam_check = 0;
						vector<float> x_vec, y_vec;
						switch (pmt_num) {
							case 1:
								for (unsigned era_it=0;era_it<bot_tof_1_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_1_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 2:
								for (unsigned era_it=0;era_it<bot_tof_2_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_2_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 3:
								for (unsigned era_it=0;era_it<bot_tof_3_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_3_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 4:
								for (unsigned era_it=0;era_it<bot_tof_4_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_4_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 5:
								for (unsigned era_it=0;era_it<bot_tof_5_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_5_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 6:
								for (unsigned era_it=0;era_it<bot_tof_6_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_6_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 7:
								for (unsigned era_it=0;era_it<bot_tof_7_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_7_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 8:
								for (unsigned era_it=0;era_it<bot_tof_8_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_8_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 9:
								for (unsigned era_it=0;era_it<bot_tof_9_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_9_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 10:
								for (unsigned era_it=0;era_it<bot_tof_10_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_10_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 11:
								for (unsigned era_it=0;era_it<bot_tof_11_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_11_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 12:
								for (unsigned era_it=0;era_it<bot_tof_12_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_12_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 13:
								for (unsigned era_it=0;era_it<bot_tof_13_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_13_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 14:
								for (unsigned era_it=0;era_it<bot_tof_14_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_14_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 15:
								for (unsigned era_it=0;era_it<bot_tof_15_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_15_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 16:
								for (unsigned era_it=0;era_it<bot_tof_16_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_16_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 17:
								for (unsigned era_it=0;era_it<bot_tof_17_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_17_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 18:
								for (unsigned era_it=0;era_it<bot_tof_18_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_18_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 19:
								for (unsigned era_it=0;era_it<bot_tof_19_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_19_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;			
							case 20:
								for (unsigned era_it=0;era_it<bot_tof_20_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_20_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 21:
								for (unsigned era_it=0;era_it<bot_tof_21_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_21_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 22:
								for (unsigned era_it=0;era_it<bot_tof_22_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_22_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 23:
								for (unsigned era_it=0;era_it<bot_tof_23_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_23_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 24:
								for (unsigned era_it=0;era_it<bot_tof_24_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_24_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 25:
								for (unsigned era_it=0;era_it<bot_tof_25_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_25_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 26:
								for (unsigned era_it=0;era_it<bot_tof_26_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_26_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 27:
								for (unsigned era_it=0;era_it<bot_tof_27_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_27_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 28:
								for (unsigned era_it=0;era_it<bot_tof_28_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_28_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 29:
								for (unsigned era_it=0;era_it<bot_tof_29_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_29_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 30:
								for (unsigned era_it=0;era_it<bot_tof_30_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_30_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 31:
								for (unsigned era_it=0;era_it<bot_tof_31_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_31_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 32:
								for (unsigned era_it=0;era_it<bot_tof_32_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_32_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 33:
								for (unsigned era_it=0;era_it<bot_tof_33_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_33_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 34:
								for (unsigned era_it=0;era_it<bot_tof_34_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_34_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 35:
								for (unsigned era_it=0;era_it<bot_tof_35_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_35_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 36:
								for (unsigned era_it=0;era_it<bot_tof_36_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_36_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 37:
								for (unsigned era_it=0;era_it<bot_tof_37_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_37_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 38:
								for (unsigned era_it=0;era_it<bot_tof_38_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_38_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 39:
								for (unsigned era_it=0;era_it<bot_tof_39_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_39_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 40:
								for (unsigned era_it=0;era_it<bot_tof_40_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_40_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 41:
								for (unsigned era_it=0;era_it<bot_tof_41_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_41_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 42:
								for (unsigned era_it=0;era_it<bot_tof_42_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_42_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 43:
								for (unsigned era_it=0;era_it<bot_tof_43_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_43_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
							case 44:
								for (unsigned era_it=0;era_it<bot_tof_44_era().size();++era_it)
								{
									MYA::Event<float> *event = bot_tof_44_era()[era_it];
									MYA::MyaTime eventTime = event->Timestamp();
									TDatime event_time((eventTime.ToString()).c_str());
									x_vec.push_back(event_time.Convert());

									// Find the beam current associated with this time
									unsigned y_push_count = 0;
									for (unsigned beam_era_it=beam_check;beam_era_it<current_era().size();++beam_era_it) {
										if (strcmp((((current_era()[beam_era_it])->Timestamp()).ToString()).c_str(),(eventTime.ToString()).c_str()) == 0) {
											if (beam_y[beam_era_it] > 10.0) {
												y_vec.push_back(event->Value(0) / beam_y[beam_era_it]);
												beam_check = beam_era_it;
												++y_push_count;
												break;
											}
											else {
												y_vec.push_back(0.0);
												beam_check = beam_era_it;
												++y_push_count;
											}
										} // if strcmp
									} // for unsigned beam_era_it
									if (y_push_count == 0)
										y_vec.push_back(0.0);
								} // for unsigned era_it
								break;
						} // switch(pmt_num)

						// Load the vector values into arrays
						float x[x_vec.size()],y[y_vec.size()];
						for (unsigned vec_it=0;vec_it<x_vec.size();++vec_it) {
							x[vec_it] = x_vec.at(vec_it);
							y[vec_it] = y_vec.at(vec_it);
						}

						// Draw the data
						char axis_name[128];
						temp_canvas->cd();
						TGraph * temp_graph = new TGraph(x_vec.size(),x,y);
						temp_graph->SetTitle(TCanvas_name);
						temp_graph->SetMarkerStyle(20);
						temp_graph->Draw("AP");
						temp_graph->GetXaxis()->SetTimeDisplay(1);
   					temp_graph->GetXaxis()->SetNdivisions(-503);
   					temp_graph->GetXaxis()->SetTimeFormat("%m-%d %H:%M");
						sprintf(axis_name,"Run %d",runnumber);
						temp_graph->GetXaxis()->SetTitle(axis_name);
						sprintf(axis_name,"TOF Scalers [scaler/nA]");
						temp_graph->GetYaxis()->SetTitle(axis_name);
						y_pmt_scaler_averages[direction_num-1][pmt_num-1] = temp_graph->GetYaxis()->GetXmax()*0.8125;
						if (all_runs_flag) 
							(pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1])[runnumber] = temp_graph->GetYaxis()->GetXmax()*0.8125;
						temp_canvas->Update();
						temp_canvas->Draw();
						temp_canvas->Write();						
					} // for unsigned int pmt_num
				} // if Bottom
				runnumber_dir->cd();
			} // for unsigned int direction_num

			// Make histogram for pmt scaler averages
			char TCanvas_pmt_scaler_averages[128], axis_name[128];
			sprintf(TCanvas_pmt_scaler_averages,"Run %d TOF PMT Scaler Averages",runnumber);
			TCanvas * pmt_scaler_averages_canvas = new TCanvas(TCanvas_pmt_scaler_averages,TCanvas_pmt_scaler_averages,200,10,700,500); 
			pmt_scaler_averages_canvas->cd();
			TH1F * pmt_scaler_averages_graph[4] = {new TH1F(),new TH1F(),new TH1F(),new TH1F()};

			// Loop over directions
			for (unsigned direction_num_it=0;direction_num_it<4;++direction_num_it) {
				sprintf(TCanvas_pmt_scaler_averages,"%s %d",TCanvas_pmt_scaler_averages,direction_num_it);
				pmt_scaler_averages_graph[direction_num_it] = new TH1F(TCanvas_pmt_scaler_averages,TCanvas_pmt_scaler_averages,44,1.0,45.0);

				// Loop over pmts
				for (unsigned pmt_scaler_averages_it=0;pmt_scaler_averages_it<44;++pmt_scaler_averages_it)
						pmt_scaler_averages_graph[direction_num_it]->Fill(pmt_scaler_averages_it+1,y_pmt_scaler_averages[direction_num_it][pmt_scaler_averages_it]);
				switch (direction_num_it) {
					case 0: // North
						pmt_scaler_averages_canvas->cd();
						pmt_scaler_averages_graph[direction_num_it]->SetBarWidth(0.25);
						pmt_scaler_averages_graph[direction_num_it]->SetFillColor(kGreen);
						pmt_scaler_averages_graph[direction_num_it]->SetBarOffset(0.25);
						sprintf(axis_name,"Module Number");
						pmt_scaler_averages_graph[direction_num_it]->GetXaxis()->SetTitle(axis_name);
						sprintf(axis_name,"Average TOF Scalers [scaler/nA]");
						pmt_scaler_averages_graph[direction_num_it]->GetYaxis()->SetTitle(axis_name);
						sprintf(TCanvas_pmt_scaler_averages,"Run %d All TOF PMT Scaler Averages",runnumber);
						pmt_scaler_averages_graph[direction_num_it]->SetTitle(TCanvas_pmt_scaler_averages);
						pmt_scaler_averages_graph[direction_num_it]->SetName(TCanvas_pmt_scaler_averages);
						pmt_scaler_averages_graph[direction_num_it]->Draw("bar");
						break;
					case 1: // South
						pmt_scaler_averages_canvas->cd();
						pmt_scaler_averages_graph[direction_num_it]->Draw("bar,same");
						pmt_scaler_averages_graph[direction_num_it]->SetBarWidth(0.25);
						pmt_scaler_averages_graph[direction_num_it]->SetFillColor(kRed);
						pmt_scaler_averages_graph[direction_num_it]->SetBarOffset(0.00);
						sprintf(axis_name,"Module Number");
						pmt_scaler_averages_graph[direction_num_it]->GetXaxis()->SetTitle(axis_name);
						sprintf(axis_name,"Average TOF Scalers [scaler/nA]");
						pmt_scaler_averages_graph[direction_num_it]->GetYaxis()->SetTitle(axis_name);
						sprintf(TCanvas_pmt_scaler_averages,"Run %d TOF South PMT Scaler Averages",runnumber);
						pmt_scaler_averages_graph[direction_num_it]->SetTitle(TCanvas_pmt_scaler_averages);
						pmt_scaler_averages_graph[direction_num_it]->SetName(TCanvas_pmt_scaler_averages);
						break;
					case 2: // Top
						pmt_scaler_averages_canvas->cd();
						pmt_scaler_averages_graph[direction_num_it]->Draw("bar,same");
						pmt_scaler_averages_graph[direction_num_it]->SetBarWidth(0.25);
						pmt_scaler_averages_graph[direction_num_it]->SetFillColor(kBlue);
						pmt_scaler_averages_graph[direction_num_it]->SetBarOffset(0.50);
						sprintf(axis_name,"Module Number");
						pmt_scaler_averages_graph[direction_num_it]->GetXaxis()->SetTitle(axis_name);
						sprintf(axis_name,"Average TOF Scalers [scaler/nA]");
						pmt_scaler_averages_graph[direction_num_it]->GetYaxis()->SetTitle(axis_name);
						sprintf(TCanvas_pmt_scaler_averages,"Run %d TOF Top PMT Scaler Averages",runnumber);
						pmt_scaler_averages_graph[direction_num_it]->SetTitle(TCanvas_pmt_scaler_averages);
						pmt_scaler_averages_graph[direction_num_it]->SetName(TCanvas_pmt_scaler_averages);
						break;
					case 3: // Bottom
						pmt_scaler_averages_canvas->cd();
						pmt_scaler_averages_graph[direction_num_it]->Draw("bar,same");
						pmt_scaler_averages_graph[direction_num_it]->SetBarWidth(0.25);
						pmt_scaler_averages_graph[direction_num_it]->SetFillColor(kMagenta);
						pmt_scaler_averages_graph[direction_num_it]->SetBarOffset(0.75);
						sprintf(axis_name,"Module Number");
						pmt_scaler_averages_graph[direction_num_it]->GetXaxis()->SetTitle(axis_name);
						sprintf(axis_name,"Average TOF Scalers [scaler/nA]");
						pmt_scaler_averages_graph[direction_num_it]->GetYaxis()->SetTitle(axis_name);
						sprintf(TCanvas_pmt_scaler_averages,"Run %d TOF Bottom PMT Scaler Averages",runnumber);
						pmt_scaler_averages_graph[direction_num_it]->SetTitle(TCanvas_pmt_scaler_averages);
						pmt_scaler_averages_graph[direction_num_it]->SetName(TCanvas_pmt_scaler_averages);
						break;
				} // switch direction_num_it
			}
			
			// Now add a legend
			pmt_scaler_averages_canvas->cd();
			TLegend *legend = new TLegend(0.77,0.77,0.98,0.94); 
			legend->AddEntry(pmt_scaler_averages_graph[1],"South","f");
   		legend->AddEntry(pmt_scaler_averages_graph[0],"North","f");
			legend->AddEntry(pmt_scaler_averages_graph[2],"Top","f");
			legend->AddEntry(pmt_scaler_averages_graph[3],"Bottom","f");
   		legend->Draw();
			pmt_scaler_averages_canvas->Update();
			pmt_scaler_averages_canvas->Draw();
			pmt_scaler_averages_canvas->Write();

			// Reset North name and title because of loop structure above
			sprintf(TCanvas_pmt_scaler_averages,"Run %d TOF North PMT Scaler Averages",runnumber);
			pmt_scaler_averages_graph[0]->SetTitle(TCanvas_pmt_scaler_averages);
			pmt_scaler_averages_graph[0]->SetName(TCanvas_pmt_scaler_averages);
			
		} // for map< unsigned int, map<string,string> >::iterator run_time_it
	} // for  vector< map< unsigned int,map<string,string> > >::iterator event_time_it

	// Make Histograms for entire run range
	if (all_runs_flag) {
		cout << "Compiling Statistics for All Runs...\n";
		// Make All Runs Directory
		ROOTFile->cd();
		TDirectory * allruns_dir = ROOTFile->mkdir("All Runs");
		allruns_dir->cd();

		// Get beam current data  //map<int,float>all_runs_beam_current, pmt_scaler_avg_all_runs[4][44];
		TCanvas * beam_canvas = new TCanvas("All Runs Beam Current","All Runs Beam Current",200,10,700,500); 
		float beam_x[all_runs_beam_current.size()], beam_y[all_runs_beam_current.size()];
		unsigned int counter = 0;
		for (map<int,float>::iterator all_runs_beam_current_it=all_runs_beam_current.begin();all_runs_beam_current_it!=all_runs_beam_current.end();++all_runs_beam_current_it)
		{
			beam_x[counter] = all_runs_beam_current_it->first;
			beam_y[counter] = all_runs_beam_current_it->second;
			++counter;
		} // for unsigned all_runs_beam_current_it

		// Plot the beam current
		beam_canvas->cd();
		TGraph * beam_graph = new TGraph(all_runs_beam_current.size(),beam_x,beam_y);
		beam_graph->SetTitle("All Runs Beam Current");
		beam_graph->SetMarkerStyle(20);
		beam_graph->Draw("AP");
		beam_graph->GetXaxis()->SetNdivisions(all_runs_beam_current.size());
		beam_graph->GetXaxis()->SetTitle("All Runs");
		beam_graph->GetYaxis()->SetTitle("Beam Current [nA]");
		beam_canvas->Update();
		beam_canvas->Draw();
		beam_canvas->Write();

		// Plot all TOF scaler averages by run number    (pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1])[runnumber] = temp_graph->GetYaxis()->GetXmax()*0.8125;
		TDirectory * tof_dir;
		char dir_name[128];
		for (unsigned int direction_num=1;direction_num<5;++direction_num) {
			if (direction_num == 1) { // North
					sprintf(dir_name,"All Runs TOF Scaler N");
					tof_dir  = allruns_dir->mkdir(dir_name); }
			if (direction_num == 2) { // South
					sprintf(dir_name,"All Runs TOF Scaler S");
					tof_dir  = allruns_dir->mkdir(dir_name); }
			if (direction_num == 3) { // Top
					sprintf(dir_name,"All Runs TOF Scaler T");
					tof_dir  = allruns_dir->mkdir(dir_name); }
			if (direction_num == 4) { // Bottom
					sprintf(dir_name,"All Runs TOF Scaler B");
					tof_dir  = allruns_dir->mkdir(dir_name); }
			tof_dir->cd();
			for (unsigned int pmt_num = 1;pmt_num<45;++pmt_num) {

				// Make the Canvas
				char TCanvas_name[128];
				sprintf(TCanvas_name,"%s %d",dir_name,pmt_num);
				TCanvas * temp_canvas = new TCanvas(TCanvas_name,TCanvas_name,200,10,700,500); 

				// Load the vector values into arrays
				float x[(pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1]).size()],y[(pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1]).size()];
				unsigned int counter = 0;
				for (map<int,float>::iterator pmt_scaler_avg_all_runs_it=(pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1]).begin();pmt_scaler_avg_all_runs_it!=(pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1]).end();++pmt_scaler_avg_all_runs_it) {
					x[counter] = pmt_scaler_avg_all_runs_it->first;
					y[counter] = pmt_scaler_avg_all_runs_it->second;
					++counter;
				}

				// Draw the data
				temp_canvas->cd();
				TGraph * temp_graph = new TGraph((pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1]).size(),x,y);
				temp_graph->SetTitle(TCanvas_name);
				temp_graph->SetMarkerStyle(20);
				temp_graph->Draw("AP");
   			temp_graph->GetXaxis()->SetNdivisions((pmt_scaler_avg_all_runs[direction_num-1][pmt_num-1]).size());
				temp_graph->GetXaxis()->SetTitle("Run Number");
				temp_graph->GetYaxis()->SetTitle(TCanvas_name);
				temp_canvas->Update();
				temp_canvas->Draw();
				temp_canvas->Write();
						
			} // for unsigned int pmt_num
			allruns_dir->cd();
		} // for unsigned int direction_num*/
	} // if all_runs_flag
	
	// Write and close root file
	ROOTFile->Write();
	ROOTFile->Close();
	cout << "Analysis Complete.\n";

	return 0;
} // int main()




















