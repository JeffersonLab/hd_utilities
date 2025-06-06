
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

#include <TROOT.h>
#include <TLatex.h>
#include <TH2.h>
#include <TCanvas.h>

//
// June 17, 2016  David Lawrence  davidl@jlab.org
//
//
// The macros in this file can be used to add some standard labels to ROOT
// plots in the upper right hand corner. To use this, you should #include
// this file and then call StandardLabels(TH1*) passing in the currently
// displayed histogram (the one defining the axes). Here's an example:
//
// #include "StandardLabels.C"
//
// void MyMacro(void)
// {
//   ...
//   TH2D *hist = new TH2D("axes", ...
//
//   // ... fill histogram ...
//
//   hist->Draw();
//   StandardLabels(hist, "#gammap#rightarrow#b_{1}#pi"); // second argument is optional
// }
//
// The labels include the date, creators initials, and svn revision number.
// The date and creator initials are generated automatically, but one may
// wish to replace that with static strings in the routines below. Similarly
// for the svn revision string which is always statically defined.
//
// WARNING: I've experienced problems with ROOT giving errors after running
// this a couple of times in the same session. Specifically, I've seen the following:
//
// !!!Fatal Error: Interpreter memory overwritten by illegal access.!!!
// !!!Terminate session!!!
//
// Oddly enough, this message came up on the 3rd invocation, but subsequent
// invocations in the same session gave no errors and everything seemed to
// work OK(??)

void ConvertFromNDC1D(TLatex *obj, TH1D *h=NULL);
void ConvertFromNDC2D(TLatex *obj, TH2D *h=NULL);

void StandardLabels1D(TH1D *axes=NULL, string mess1="", string mess2="", string mess3="", string mess4="");
void StandardLabels2D(TH2D *axes=NULL, string mess1="", string mess2="", string mess3="", string mess4="");


//----------------
// GetRevision
//----------------
string GetRevision(void)
{
	// This will try and automatically find the svn revision number of
	// the source pointed to by your HALLD_HOME environment variable.
	// To fix the revision to a specific value, uncomment the following
	// line and set it there.

	return "";

	// Extract revision number using git and HALLD_HOME
	string revision = "";
	const char *HALLD_HOME = getenv("HALLD_HOME");
	if(HALLD_HOME != NULL){
		char cmd[1024];
		sprintf(cmd, "git --git-dir=%s/.git rev-parse --short HEAD > .git_revision", HALLD_HOME);
		system(cmd);
	}
	ifstream ifstr(".git_revision");
	ifstr >> revision;
	ifstr.close();
	
	return string("git revision #") + revision;
}

//----------------
// GetDate
//----------------
string GetDate(void)
{
	// uncomment the following line to use a static string for the date
	//return string("January 4, 1970");

	clock_t t = time(NULL);
	long tlong = t;
	struct tm *tm = localtime(&tlong);
	char str[256];
	strftime(str, 256, "%B %e, %Y  %R %Z", tm);

	return string(str);
}

//----------------
// GetInitials
//----------------
string GetInitials(void)
{
	// uncomment the following line to use a static string for the creator's initials
	//return string("AB");

	string initials;
	const char *cmd = "finger $USER |grep \"Name:\" | awk '{print substr($4,1,1)substr($5,1,1)}' > .initials";
	system(cmd);
	ifstream ifstr(".initials");
	ifstr >> initials;
	ifstr.close();
	
	return initials;
}

// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
// ( you probably don't want to edit below here! )
// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

//----------------
// DateLabel
//----------------
void DateLabel(char *date_lab)
{
	sprintf(date_lab,"%s %s", GetDate().c_str(), GetInitials().c_str());
}

//----------------
// StandardLabels
//----------------
void StandardLabels(TH1 *h, string mess1="", string mess2="", string mess3="", string mess4="")
{
	TH2D *h2d = dynamic_cast<TH2D*>(h);
	if(h2d){
		StandardLabels2D(h2d, mess1, mess2, mess3, mess4);
		return;
	}

	TH1D *h1d = dynamic_cast<TH1D*>(h);
	if(h1d){
		StandardLabels1D(h1d, mess1, mess2, mess3, mess4);
		return;
	}
}

//----------------
// StandardLabels2D
//----------------
void StandardLabels2D(TH2D *axes, string mess1, string mess2, string mess3, string mess4)
{
	// This will draw a label or two on the
	// current plot using the NDC coordinates.
	// It is put here to make sure all plots have
	// a consistent labeling.
	
	char date_lab[256];
	DateLabel(date_lab);
	string rev_lab = GetRevision();

	// Date, Author
	TLatex *lab = new TLatex(0.600, 0.7, date_lab);
	ConvertFromNDC2D(lab, axes);
	lab->SetTextSize(0.03);
	lab->SetTextAlign(33);
	lab->SetTextColor(kBlue);
	lab->Draw();

	// SVN Revision
	lab = new TLatex(0.625, 0.645, rev_lab.c_str());
	ConvertFromNDC2D(lab, axes);
	lab->SetTextSize(0.02);
	lab->SetTextAlign(31);
	lab->SetTextColor(kBlue);
	lab->Draw();

	// mess1  - upper right
	if(mess1!=""){
		lab = new TLatex(0.6, 0.61, mess1.c_str());
		ConvertFromNDC2D(lab, axes);
		lab->SetTextSize(0.03);
		lab->SetTextAlign(31);
		lab->Draw();
	}

	// mess2  - top, right of center
	if(mess2!=""){
		lab = new TLatex(0.0, 0.60, mess2.c_str());
		ConvertFromNDC2D(lab, axes);
		lab->SetTextSize(0.03);
		lab->SetTextAlign(32);
		lab->Draw();
	}

	// mess3  - top left
	if(mess3!=""){
		lab = new TLatex(-0.575, 0.60, mess3.c_str());
		ConvertFromNDC2D(lab, axes);
		lab->SetTextSize(0.025);
		lab->SetTextAlign(12);
		lab->Draw();
	}

	// mess4  - on right hand side at 90 degrees
	if(mess4!=""){
		lab = new TLatex(0.65, 0.0, mess4.c_str());
		ConvertFromNDC2D(lab, axes);
		lab->SetTextSize(0.035);
		lab->SetTextAlign(22);
		lab->SetTextAngle(270.0);
		lab->Draw();
	}
}

//----------------
// StandardLabels1D
//----------------
void StandardLabels1D(TH1D *axes, string mess1, string mess2, string mess3, string mess4)
{
	// This will draw a label or two on the
	// current plot using the NDC coordinates.
	// It is put here to make sure all plots have
	// a consistent labeling.
	
	char date_lab[256];
	DateLabel(date_lab);
	string rev_lab = GetRevision();
	
	// Date, Author
	TLatex *lab = new TLatex(0.7, 0.7, date_lab);
	ConvertFromNDC1D(lab, axes);
	lab->SetTextSize(0.03);
	lab->SetTextAlign(33);
	lab->SetTextColor(kBlue);
	lab->Draw();

	// SVN Revision
	lab = new TLatex(0.7, 0.645, rev_lab.c_str());
	ConvertFromNDC1D(lab, axes);
	lab->SetTextSize(0.02);
	lab->SetTextAlign(31);
	lab->SetTextColor(kBlue);
	lab->Draw();

	// mess1  - upper right
	lab = new TLatex(0.7, 0.61, mess1.c_str());
	ConvertFromNDC1D(lab, axes);
	lab->SetTextSize(0.03);
	lab->SetTextAlign(31);
	lab->Draw();
	
	// mess2  - top, right of center
	lab = new TLatex(0.35, 0.675, mess2.c_str());
	ConvertFromNDC1D(lab, axes);
	lab->SetTextSize(0.03);
	lab->SetTextAlign(32);
	lab->Draw();

	// mess3  - top left
	if(mess3!=""){
		lab = new TLatex(-0.575, 0.60, mess3.c_str());
		ConvertFromNDC1D(lab, axes);
		lab->SetTextSize(0.025);
		lab->SetTextAlign(12);
		lab->Draw();
	}

	// mess4  - on right hand side at 90 degrees
	if(mess4!=""){
		lab = new TLatex(0.65, 0.0, mess4.c_str());
		ConvertFromNDC1D(lab, axes);
		lab->SetTextSize(0.035);
		lab->SetTextAlign(22);
		lab->SetTextAngle(270.0);
		lab->Draw();
	}
}

//----------------
// ConvertFromNDC
//----------------
void ConvertFromNDC2D(TLatex *obj, TH2D *h)
{
	// Bugs in ROOT make it hard to plot labels consistently.
	// For 1D plots, the histogram axes define the coordinate
	// system. For 2D plots, we seem to be forced to use the
	// NDC. There does not seem to be an obvious way to tell
	// which we're using so we pass the information in in the
	// form of the "axes" histogram. If it is not NULL, then
	// we use it to define the limits. Otherwise, we do nothing.

	if(h==NULL)return;

	// Check if there is a TCanvas object named c1. If so,
	// assume it is what was used to plot things and we can
	// check on whether logy is set.
	TCanvas *c1 = (TCanvas*)gROOT->FindObject("c1");
	bool is_logy = false;
	if(c1 != NULL)is_logy = c1->GetLogy();

	TAxis *xaxis = h->GetXaxis();
	TAxis *yaxis = h->GetYaxis();

	// These seem to be sensitive to values set by "SetRangeUser" as well as pad margins
	double xmin = xaxis->GetBinCenter(xaxis->GetFirst());
	double xmax = xaxis->GetBinCenter(xaxis->GetLast());

	double ymin = yaxis->GetBinLowEdge(yaxis->GetFirst());
	double ymax = yaxis->GetBinLowEdge(yaxis->GetLast()+1);

	double x = obj->GetX();
	double y = obj->GetY();
	
	x = xmin + (xmax-xmin)*(0.5+x/1.15);

	// Calculate "y" based on whether y-axis is log or not
	if(is_logy){
		double log_ymin = ymin!=0.0 ? log(ymin):1.0;
		double log_ymax = log(ymax);
		double a = log_ymin + (log_ymax-log_ymin)*(0.5+y/1.15);
		y = exp(log_ymin + (log_ymax-log_ymin)*(0.5+y/1.15));
	}else{
		y = ymin + (ymax-ymin)*(0.5+y/1.15);
	}

	obj->SetX(x);
	obj->SetY(y);

}

//----------------
// ConvertFromNDC1D
//----------------
void ConvertFromNDC1D(TLatex *obj, TH1D *h)
{
	// Bugs in ROOT make it hard to plot labels consistently.
	// For 1D plots, the histogram axes define the coordinate
	// system. For 2D plots, we seem to be forced to use the
	// NDC. There does not seem to be an obvious way to tell
	// which we're using so we pass the information in the
	// form of the "axes" histogram. If it is not NULL, then
	// we use it to define the limits. Otherwise, we do nothing.
	//
	// caveat: When plotting on a log axis, there is no way to
	// pull the lower limit from the histogram. Instead, we
	// detect try and detect whether the y-axis is log scale
	// by looking for a "c1" object and assuming it is a TCanvas.
	// We then use it to determine if logy is set and if so,
	// to get the limits of the histogram from gPad.

	if(h==NULL)return;
	
	// Check if there is a TCanvas object named c1. If so,
	// assume it is what was used to plot things and we can
	// check on whether logy is set.
	TCanvas *c1 = (TCanvas*)gROOT->FindObject("c1");
	bool is_logy = false;
	if(c1 != NULL)is_logy = c1->GetLogy();

	TAxis *xaxis = h->GetXaxis();
	double ymin = h->GetMinimum();
	double ymax = h->GetMaximum()*1.05;

	// If y-axis is log scale then try and get limits from gPad
	if(c1 != NULL){
		if(is_logy){
			c1->Update(); // force gPad to update
			ymin = pow(10.0, gPad->GetUymin());
			ymax = pow(10.0, gPad->GetUymax());
		}else{
			c1->Update();
			ymin = gPad->GetUymin();
			ymax = gPad->GetUymax();
		}
	}

	// These seem to be sensitive to values set by "SetRangeUser" as well as pad margins
	double xmin = xaxis->GetBinCenter(xaxis->GetFirst());
	double xmax = xaxis->GetBinCenter(xaxis->GetLast());

	//double ymin = yaxis->GetBinCenter(yaxis->GetFirst());
	//double ymax = yaxis->GetBinCenter(yaxis->GetLast());

	double x = obj->GetX();
	double y = obj->GetY();
	
	x = xmin + (xmax-xmin)*(0.5+x/1.15);
	
	// Calculate "y" based on whether y-axis is log or not
	if(is_logy){
		double log_ymin = ymin!=0.0 ? log(ymin):1.0;
		double log_ymax = log(ymax);
		double a = log_ymin + (log_ymax-log_ymin)*(0.5+y/1.15);
		y = exp(log_ymin + (log_ymax-log_ymin)*(0.5+y/1.15));
	}else{
		y = ymin + (ymax-ymin)*(0.5+(y-ymin)/1.15);
	}
	
	obj->SetX(x);
	obj->SetY(y);

}
