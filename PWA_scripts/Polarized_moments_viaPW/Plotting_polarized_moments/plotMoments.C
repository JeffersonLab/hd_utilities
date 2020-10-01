#include<TH1F.h>
#include<TH2F.h>
#include<TMath.h>
#include<TCanvas.h>
#include<TVector3.h>
#include<TLorentzVector.h>
#include<TLorentzRotation.h>
#include<TFile.h>
#include<TTree.h>

using namespace std;

void plotMoments()

{

     TFile *file = new TFile("data_newhalldsim_SPD.root");
        TTree *tree = (TTree*)file->Get("kin");

	TH1F* h1_totI = new TH1F("h1_totI", "Total Intensity", 50, 0.7, 2.05);

        TH1F* blueMoment00 = new TH1F("blueMoment00", "LM = 00", 100, 0.7, 2.05);
        TH1F* redMoment00 = new TH1F("redMoment00", "LM = 00", 100, 0.7, 2.05);

        TH1F* blueMoment10 = new TH1F("blueMoment10", "LM = 10", 100, 0.7, 2.05);
        TH1F* redMoment10 = new TH1F("redMoment10", "LM = 10", 100, 0.7, 2.05);

        TH1F* blueMoment11 = new TH1F("blueMoment11", "LM = 11", 100, 0.7, 2.05);
        TH1F* redMoment11 = new TH1F("redMoment11", "LM = 11", 100, 0.7, 2.05);

        TH1F* blueMoment20 = new TH1F("blueMoment20", "LM = 20", 100, 0.7, 2.05);
        TH1F* redMoment20 = new TH1F("redMoment20", "LM = 20", 100, 0.7, 2.05);
        TH1F* blueMoment21 = new TH1F("blueMoment21", "LM = 21", 100, 0.7, 2.05);
        TH1F* redMoment21 = new TH1F("redMoment21", "LM = 21", 100, 0.7, 2.05);

        TH1F* blueMoment22 = new TH1F("blueMoment22", "LM = 22", 100, 0.7, 2.05);
        TH1F* redMoment22 = new TH1F("redMoment22", "LM = 22", 100, 0.7, 2.05);
	TH1F* greenMoment22 = new TH1F("greenMoment22", "LM = 22", 100, 0.7, 2.05);


        TH1F* blueMoment30 = new TH1F("blueMoment30", "LM = 30", 100, 0.7, 2.05);
        TH1F* redMoment30 = new TH1F("redMoment30", "LM = 30", 100, 0.7, 2.05);

        TH1F* blueMoment31 = new TH1F("blueMoment31", "LM = 31", 100, 0.7, 2.05);
        TH1F* redMoment31 = new TH1F("redMoment31", "LM = 31", 100, 0.7, 2.05);

        TH1F* blueMoment32 = new TH1F("blueMoment32", "LM = 32", 100, 0.7, 2.05);
        TH1F* redMoment32 = new TH1F("redMoment32", "LM = 32", 100, 0.7, 2.05);

        TH2F* massVsPhi = new TH2F("massVsPhi", "Resonant Mass vs. #Phi", 100, 0, 4, 100, -3.2, 3.2);
        TH2F* massVsCosT = new TH2F("massVsCosT", "Resonant Mass vs. Cos(#theta)", 100, 0, 4, 100, -1,1);
        TH2F* massVsphi = new TH2F("massVsphi", "Resonant Mass vs. #phi", 100, 0, 4, 100, -3.2, 3.2);


        int NumFinalState;
        float EFinal[3], PxFinal[3], PyFinal[3], PzFinal[3], EBeam, PxBeam, PyBeam, PzBeam;
        int pid[3];

        int NUM_EVENTS = tree->GetEntries();

        tree->SetBranchAddress("NumFinalState", &NumFinalState);
        tree->SetBranchAddress("E_FinalState", &EFinal);
        tree->SetBranchAddress("Px_FinalState", &PxFinal);
        tree->SetBranchAddress("Py_FinalState", &PyFinal);
        tree->SetBranchAddress("Pz_FinalState", &PzFinal);
        tree->SetBranchAddress("E_Beam", &EBeam);
        tree->SetBranchAddress("Px_Beam", &PxBeam);
        tree->SetBranchAddress("Py_Beam", &PyBeam);
        tree->SetBranchAddress("Pz_Beam", &PzBeam);

        float etapiM, rweight, bweight;
        TLorentzVector P0, p1, p2, p3;

        for(int i=0; i<NUM_EVENTS; i++){

                tree->GetEntry(i);
               
                P0.SetPx(PxBeam);
                P0.SetPy(PyBeam);
                P0.SetPz(PzBeam);
                P0.SetE(EBeam);

                p1.SetPx(PxFinal[0]);
                p1.SetPy(PyFinal[0]);
                p1.SetPz(PzFinal[0]);
                p1.SetE(EFinal[0]);

                p2.SetPx(PxFinal[1]);
                p2.SetPy(PyFinal[1]);
                p2.SetPz(PzFinal[1]);
                p2.SetE(EFinal[1]);

                p3.SetPx(PxFinal[2]);
                p3.SetPy(PyFinal[2]);
                p3.SetPz(PzFinal[2]);
                p3.SetE(EFinal[2]);


                TLorentzVector resonance = p2 + p3;
                TLorentzRotation resRestBoost( -resonance.BoostVector() );

                TLorentzVector beam_res   = resRestBoost * P0;
                TLorentzVector recoil_res = resRestBoost * p1;
                TLorentzVector p2_res = resRestBoost * p2;

                TVector3 z = -1.*recoil_res.Vect().Unit();
                TVector3 y =(P0.Vect().Unit().Cross(-p1.Vect().Unit())).Unit(); 
                TVector3 x = y.Cross(z);

                TVector3 angles( (p2_res.Vect()).Dot(x),
                                (p2_res.Vect()).Dot(y),
                                (p2_res.Vect()).Dot(z) );

                Double_t cosTheta = angles.CosTheta();
                Double_t phi = angles.Phi();
                Double_t theta = angles.Theta();

                double polAngle = 1.77; // PARA

                TVector3 eps(cos(polAngle*TMath::DegToRad()), sin(polAngle*TMath::DegToRad()), 0.0); // beam polarization vector
                Double_t Phi = atan2(y.Dot(eps), P0.Vect().Unit().Dot(eps.Cross(y)));
                TVector3 eta = p2.Vect();
                Double_t omega = atan2(y.Dot(eta), P0.Vect().Unit().Dot(eta.Cross(y)));

                //double Pgamma = 1.0; 
		double Pgamma = 0.3;

                etapiM = resonance.M();
                massVsCosT->Fill(etapiM, cosTheta);
                massVsPhi->Fill(etapiM, Phi);
                massVsphi->Fill(etapiM, phi);

		//Total Intensity
		h1_totI->Fill(etapiM);


                //LM = 00
                rweight = cos(2*Phi);
                bweight = (Pgamma/2);
                redMoment00->Fill(etapiM, rweight);
                blueMoment00->Fill(etapiM, bweight);
                
               
                //LM = 10
		// Pgamma = 2.0;
                rweight = cos(theta) * cos(2*Phi);
                bweight = (Pgamma/2) * cos(theta);
                redMoment10->Fill(etapiM, rweight);
                blueMoment10->Fill(etapiM, bweight);
                
                
                //LM = 20
		// Pgamma = 0.8;
                rweight = (3*cos(theta)*cos(theta) -1) *cos(2*Phi);
                bweight = (Pgamma/2)*(3*cos(theta)*cos(theta) -1);
                redMoment20->Fill(etapiM, rweight);
                blueMoment20->Fill(etapiM, bweight);


                //LM = 21
		//  Pgamma = 0.8;
                rweight = -sin(2*theta)*cos(2*Phi)*cos(phi);
                bweight = -(Pgamma/2)*sin(2*theta)*cos(phi);
                redMoment21->Fill(etapiM, rweight);
                blueMoment21->Fill(etapiM, bweight);


                //LM = 11 
                rweight = -sin(theta)*cos(phi)*cos(2*Phi);
                bweight = -(Pgamma/2)*sin(theta)*cos(phi);
                redMoment11->Fill(etapiM, rweight);
                blueMoment11->Fill(etapiM, bweight);

 
                //LM = 22
                rweight =  cos(2*phi)*cos(2*Phi)*sin(theta)*sin(theta);
                bweight = (Pgamma/2)*cos(2*phi)*sin(theta)*sin(theta);
		double gweight = -sin(2*phi)*sin(2*Phi)*sin(theta)*sin(theta);
                redMoment22->Fill(etapiM, rweight);
                blueMoment22->Fill(etapiM, bweight);
		greenMoment22->Fill(etapiM, gweight);
                
                
                //LM = 30
                rweight = (5*TMath::Power(cos(theta), 3) - 3*cos(theta))*cos(2*Phi);
                bweight = (Pgamma/2)*(5*TMath::Power(cos(theta), 3) - 3*cos(theta));
                redMoment30->Fill(etapiM, rweight);
                blueMoment30->Fill(etapiM, bweight);


                //LM = 31
                rweight = sin(theta/2)*cos(theta/2)*((15/4)*TMath::Power(cos(theta) - 1, 2) + (15/2)*(cos(theta) -1) +3)* cos(phi) *cos(2*Phi);
                bweight =  (Pgamma/2)*sin(theta/2)*cos(theta/2)*((15/4)*TMath::Power(cos(theta) - 1, 2) + (15/2)*(cos(theta) -1) +3)*cos(phi);
                redMoment31->Fill(etapiM, rweight);
                blueMoment31->Fill(etapiM, bweight);


                //LM = 32
                rweight = TMath::Power(sin(theta/2), 2)*TMath::Power(cos(theta/2), 2)*cos(theta)*cos(2*phi)*cos(2*Phi);
                bweight = (Pgamma/2)*TMath::Power(sin(theta/2), 2)*TMath::Power(cos(theta/2), 2)*cos(theta)*cos(2*phi);
                redMoment32->Fill(etapiM, rweight);
                blueMoment32->Fill(etapiM, bweight);

        }


      


	TCanvas* c2= new TCanvas("c2", "canvas", 1000, 1200);

        redMoment00->Draw("HIST");
	c2->Print("Plots/H1_00_scaled.pdf");

        blueMoment00->Draw("HIST");
	c2->Print("Plots/H0_00_scaled.pdf");
 
       redMoment10->Draw("HIST");
	c2->Print("Plots/H1_10_scaled.pdf");

        blueMoment10->Draw("HIST");
	c2->Print("Plots/H0_10_scaled.pdf"); 

        redMoment11->Draw("HIST");
	c2->Print("Plots/H1_11_scaled.pdf");

        blueMoment11->Draw("HIST");
	c2->Print("Plots/H0_11_scaled.pdf");

        blueMoment20->Draw("HIST");
	c2->Print("Plots/H0_20_scaled.pdf");

        redMoment20->Draw("HIST");
	c2->Print("Plots/H1_20_scaled.pdf");
     
        redMoment21->Draw("HIST");
	c2->Print("Plots/H1_21_scaled.pdf");

        blueMoment21->Draw("HIST");
	c2->Print("Plots/H0_21_scaled.pdf");



        redMoment22->Draw("HIST");
	c2->Print("Plots/H1_22_scaled.pdf");

        blueMoment22->Draw("HIST");
	c2->Print("Plots/H0_22_scaled.pdf");




       redMoment00->SetLineColor(kRed);
        blueMoment00->SetLineColor(kBlue);
        redMoment10->SetLineColor(kRed);
        blueMoment10->SetLineColor(kBlue);
        redMoment11->SetLineColor(kRed);
        blueMoment11->SetLineColor(kBlue);
        redMoment20->SetLineColor(kRed);
        blueMoment20->SetLineColor(kBlue);
        redMoment21->SetLineColor(kRed);
        blueMoment21->SetLineColor(kBlue);

        redMoment30->SetLineColor(kRed);
        blueMoment30->SetLineColor(kBlue);
        redMoment31->SetLineColor(kRed);
        blueMoment31->SetLineColor(kBlue);
        redMoment32->SetLineColor(kRed);
        blueMoment32->SetLineColor(kBlue);
        redMoment22->SetLineColor(kRed);
        blueMoment22->SetLineColor(kBlue);



        TCanvas* c1= new TCanvas("c1", "canvas", 1000, 1200);
        c1->Divide(3,3);
        c1->cd();
        c1->cd(1);
	//	blueMoment00->Scale(1/blueMoment00->Integral());
	//   redMoment00->Scale(1/redMoment00->Integral());
	redMoment00->Draw("HIST, same");
        blueMoment00->Draw("HIST,same");

        c1->cd(2);
	redMoment10->Rebin(2);
	blueMoment10->Rebin(2);
	//	blueMoment10->Scale(1/blueMoment10->Integral());
        //redMoment10->Scale(0.5/redMoment10->Integral());
	blueMoment10->SetMinimum(1.2*redMoment10->GetMinimum());
	blueMoment10->Draw("HIST,same");
	redMoment10->Draw("HIST, same");
    

        c1->cd(3);
	redMoment11->Rebin(2);
	blueMoment11->Rebin(2);
	//	blueMoment11->Scale(1/blueMoment11->Integral());
	// redMoment11->Scale(0.5/redMoment11->Integral());
        redMoment11->Draw("HIST, same");
        blueMoment11->Draw("HIST,same");
        
	c1->cd(4);
	//	blueMoment20->Scale(1/blueMoment20->Integral());
	// redMoment20->Scale(1/redMoment20->Integral());
	blueMoment20->SetMaximum(1.2*redMoment20->GetMaximum());
        blueMoment20->Draw("HIST,same");
        redMoment20->Draw("HIST, same");

        c1->cd(5);
	//	blueMoment21->Scale(1/blueMoment21->Integral());
        //redMoment21->Scale(1/redMoment21->Integral());
	blueMoment21->SetMinimum(1.2*redMoment21->GetMinimum());
        blueMoment21->Draw("HIST,same");
	redMoment21->Draw("HIST, same");

        c1->cd(6);
	//	blueMoment22->Scale(1/blueMoment22->Integral());
	//  redMoment22->Scale(1/redMoment22->Integral());
	blueMoment22->SetMaximum(1.2*redMoment22->GetMaximum());
	blueMoment22->Draw("HIST, same");
        redMoment22->Draw("HIST, same");
       

        c1->cd(7);
        redMoment30->Draw("HIST, same");
        blueMoment30->Draw("HIST, same");
        c1->cd(8);
        redMoment31->Draw("HIST, same");
        blueMoment31->Draw("HIST, same");
        c1->cd(9);
        redMoment32->Draw("HIST, same");
        blueMoment32->Draw("HIST, same");
        c1->Draw();


	TCanvas *c3=new TCanvas();
	greenMoment22->Draw("hist");

	TCanvas *c4=new TCanvas();
	h1_totI->Draw();
	c4->Print("Plots/Total_intensity.pdf");


}
