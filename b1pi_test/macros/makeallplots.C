{

  char atr[128] = "standardtest";
  //oldgeom
    
  char nam[128];
  TCanvas * c2 = new TCanvas("c2", "c2", 800, 600);

  sprintf(nam,"b1_pi_plots_gen.root",atr);
  TFile *f1 = new TFile(nam);

  sprintf(nam,"b1_pi_plots_rec.root",atr);
  TFile *f2 = new TFile(nam);

  TH2D *prot_gen = (TH2D*)f1->Get("Ep_vs_tp");
  TH2D *prot_rec = (TH2D*)f2->Get("Ep_vs_tp");

  prot_rec->SetMarkerColor(2);

  prot_gen->Draw();
  prot_rec->Draw("same");
  gPad->SetGrid();
  Int_t Egen = prot_gen->GetEntries();
  Int_t Erec = prot_rec->GetEntries();

  char text1[128];
  sprintf(text1,"Generated Protons %d",Egen);
  TText *t1 = new TText(30., 2.5,text1);
  
  char text2[128];
  sprintf(text2,"Reconstructed Protons %d",Erec);
  Double_t max1 = prot_rec->GetMaximum()*0.8; 
  t1 = new TText(30., max1,text1);
  TText *t2 = new TText(30., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_protons.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_protons.gif",atr);
  c2->SaveAs(nam);

  TH2D *photon_gen = (TH2D*)f1->Get("Eg_vs_tg");
  TH2D *photon_rec = (TH2D*)f2->Get("Eg_vs_tg");
 
  photon_rec->SetMarkerColor(2);
  photon_rec->SetLineColor(2);

  photon_gen->Draw("box");
  photon_rec->Draw("same");

  Egen = photon_gen->GetEntries();
  Erec = photon_rec->GetEntries();

  sprintf(text1,"Generated Photons %d",Egen);
  t1 = new TText(30., 0.7,text1);
  
  sprintf(text2,"Reconstructed Photons %d",Erec);
  max1 = photon_gen->GetMaximum()*0.8; 
  t1 = new TText(30., max1,text1);
  t2 = new TText(30., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_photons.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_photons.gif",atr);
  c2->SaveAs(nam);

  
  TH2D *pip_gen = (TH2D*)f1->Get("Epip_vs_tpip");
  TH2D *pip_rec = (TH2D*)f2->Get("Epip_vs_tpip");
 
  pip_rec->SetMarkerColor(2);
  pip_rec->SetLineColor(2);

  pip_gen->Draw("box");
  pip_rec->Draw("same box");

  Egen = pip_gen->GetEntries();
  Erec = pip_rec->GetEntries();

  sprintf(text1,"Generated Pi+ %d",Egen);
  t1 = new TText(30., 2.7,text1);
  
  sprintf(text2,"Reconstructed Pi+ %d",Erec);
  max1 = pip_gen->GetMaximum()*0.8; 
  t1 = new TText(30., max1,text1);
  t2 = new TText(30., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_piplus.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_piplus.gif",atr);
  c2->SaveAs(nam);
 
  TH2D *pim_gen = (TH2D*)f1->Get("Epim_vs_tpim");
  TH2D *pim_rec = (TH2D*)f2->Get("Epim_vs_tpim");
 
  pim_rec->SetMarkerColor(2);
  pim_rec->SetLineColor(2);

  pim_gen->Draw("box");
  pim_rec->Draw("same box");

  Egen = pim_gen->GetEntries();
  Erec = pim_rec->GetEntries();

  sprintf(text1,"Generated Pi- %d",Egen);
  t1 = new TText(30., 2.7,text1);
  
  sprintf(text2,"Reconstructed Pi- %d",Erec);
  max1 = pim_gen->GetMaximum()*0.8; 
  t1 = new TText(30., max1,text1);
  t2 = new TText(30., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_piminus.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_piminus.gif",atr);
  c2->SaveAs(nam);
 
  TH1D *pip_theta_gen =  (TH1D*)f1->Get("theta_pip");
  TH1D *pip_theta_rec =  (TH1D*)f2->Get("theta_pip");

  pip_theta_rec->SetLineColor(2);
  pip_theta_gen->GetXaxis()->SetRangeUser(0.,80.);
  pip_theta_gen->Draw();
  pip_theta_rec->Draw("same");

  Egen = pip_theta_gen->GetEntries();
  Erec = pip_theta_rec->GetEntries();

  sprintf(text1,"Generated Pi+ %d",Egen);
  t1 = new TText(30., 250.,text1);
  
  sprintf(text2,"Reconstructed Pi+ %d",Erec);
  t2->SetTextColor(2);
  max1 = pip_theta_gen->GetMaximum()*0.8; 
  t1 = new TText(30., max1,text1);
  t2 = new TText(30., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_piplus_theta.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_piplus_theta.gif",atr);
  c2->SaveAs(nam);

  TH1D *pim_theta_gen =  (TH1D*)f1->Get("theta_pim");
  TH1D *pim_theta_rec =  (TH1D*)f2->Get("theta_pim");

  pim_theta_rec->SetLineColor(2);
  pim_theta_gen->GetXaxis()->SetRangeUser(0.,80.);
  pim_theta_gen->Draw();
  pim_theta_rec->Draw("same");

  Egen = pim_theta_gen->GetEntries();
  Erec = pim_theta_rec->GetEntries();

  sprintf(text1,"Generated Pi- %d",Egen);
  t1 = new TText(60., 250.,text1);
  
  sprintf(text2,"Reconstructed Pi- %d",Erec);
  t2 = new TText(60., 200.,text2);
  t2->SetTextColor(2);
  max1 = pim_theta_gen->GetMaximum()*0.8; 
  t1 = new TText(30., max1,text1);
  t2 = new TText(30., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_piminus_theta.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_piminus_theta.gif",atr);
  c2->SaveAs(nam);

  TH1D *p_theta_gen =  (TH1D*)f1->Get("theta_p");
  TH1D *p_theta_rec =  (TH1D*)f2->Get("theta_p");

  p_theta_rec->SetLineColor(2);
  p_theta_gen->GetXaxis()->SetRangeUser(0.,80.);
  p_theta_gen->Draw();
  p_theta_rec->Draw("same");

  Egen = p_theta_gen->GetEntries();
  Erec = p_theta_rec->GetEntries();

  sprintf(text1,"Generated Protons %d",Egen);
  t1 = new TText(80., 250.,text1);
  
  sprintf(text2,"Reconstructed Protons %d",Erec);
  max1 = p_theta_gen->GetMaximum()*0.8; 
  t1 = new TText(5., max1,text1);
  t2 = new TText(5., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_protons_theta.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_protons_theta.gif",atr);
  c2->SaveAs(nam);

  TH1D *g_theta_gen =  (TH1D*)f1->Get("theta_gamma");
  TH1D *g_theta_rec =  (TH1D*)f2->Get("theta_gamma");

  g_theta_rec->SetLineColor(2);
  g_theta_rec->Draw();
  g_theta_gen->Draw("same");

  Egen = g_theta_gen->GetEntries();
  Erec = g_theta_rec->GetEntries();

  sprintf(text1,"Generated Photons %d",Egen);
  t1 = new TText(60., 200.,text1);
  
  sprintf(text2,"Reconstructed Photons %d",Erec);
  max1 = g_theta_rec->GetMaximum()*0.8; 
  t1 = new TText(60., max1,text1);
  t2 = new TText(60., max1*0.9,text2); 
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_photons_theta.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_photons_theta.gif",atr);
  c2->SaveAs(nam);

  g_theta_rec->GetXaxis()->SetRangeUser(0.,50.);
  g_theta_rec->Draw();
  g_theta_gen->Draw("same");
  max1 = g_theta_rec->GetMaximum()*0.8; 
  t1 = new TText(18., max1,text1);
  t2 = new TText(18., max1*0.9,text2);
  max1 = g_theta_rec->GetMaximum()*0.8; 
  t1 = new TText(19., max1,text1);
  t2 = new TText(19., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_photons_theta_zoom.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_photons_theta_zoom.gif",atr);
  c2->SaveAs(nam);

  
  TH1D *pip_phi_gen =  (TH1D*)f1->Get("phi_pip");
  TH1D *pip_phi_rec =  (TH1D*)f2->Get("phi_pip");

  pip_phi_rec->SetLineColor(2);
  pip_phi_gen->GetXaxis()->SetRangeUser(-180.,180.); 
  pip_phi_gen->GetYaxis()->SetRangeUser(0.,pip_phi_gen->GetMaximum()*1.2);
  pip_phi_gen->Draw();
  pip_phi_rec->Draw("same");

  Egen = pip_phi_gen->GetEntries();
  Erec = pip_phi_rec->GetEntries();

  sprintf(text1,"Generated Pi+ %d",Egen);
  t1 = new TText(30., 250.,text1);
  
  sprintf(text2,"Reconstructed Pi+ %d",Erec);
  max1 = pip_phi_gen->GetMaximum()*0.8; 
  t1 = new TText(30., max1,text1);
  t2 = new TText(30., max1*0.9,text2); 
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_piplus_phi.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_piplus_phi.gif",atr);
  c2->SaveAs(nam);

  TH1D *pim_phi_gen =  (TH1D*)f1->Get("phi_pim");
  TH1D *pim_phi_rec =  (TH1D*)f2->Get("phi_pim");

  pim_phi_rec->SetLineColor(2);
  pim_phi_gen->GetXaxis()->SetRangeUser(-180.,180.);
  pim_phi_gen->GetYaxis()->SetRangeUser(0.,pim_phi_gen->GetMaximum()*1.2);
  pim_phi_gen->Draw();
  pim_phi_rec->Draw("same");

  Egen = pim_phi_gen->GetEntries();
  Erec = pim_phi_rec->GetEntries();

  sprintf(text1,"Generated Pi- %d",Egen);
  t1 = new TText(60., 250.,text1);
  
  sprintf(text2,"Reconstructed Pi- %d",Erec);
  max1 = pim_phi_gen->GetMaximum()*0.8; 
  t1 = new TText(30., max1,text1);
  t2 = new TText(30., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_piminus_phi.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_piminus_phi.gif",atr);
  c2->SaveAs(nam);

  TH1D *p_phi_gen =  (TH1D*)f1->Get("phi_p");
  TH1D *p_phi_rec =  (TH1D*)f2->Get("phi_p");

  p_phi_rec->SetLineColor(2);
  p_phi_gen->GetXaxis()->SetRangeUser(-180.,180.);
  p_phi_gen->GetYaxis()->SetRangeUser(0.,p_phi_gen->GetMaximum()*1.2);
  p_phi_gen->Draw();
  p_phi_rec->Draw("same");

  Egen = p_phi_gen->GetEntries();
  Erec = p_phi_rec->GetEntries();

  sprintf(text1,"Generated Protons %d",Egen);
  t1 = new TText(80., 250.,text1);
  
  sprintf(text2,"Reconstructed Protons %d",Erec);
  max1 = p_phi_gen->GetMaximum()*0.8; 
  t1 = new TText(5., max1,text1);
  t2 = new TText(5., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_protons_phi.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_protons_phi.gif",atr);
  c2->SaveAs(nam);

  TH1D *g_phi_gen =  (TH1D*)f1->Get("phi_gamma");
  TH1D *g_phi_rec =  (TH1D*)f2->Get("phi_gamma");

  g_phi_rec->SetLineColor(2);  
  g_phi_rec->GetYaxis()->SetRangeUser(0.,g_phi_rec->GetMaximum()*1.2);
  g_phi_rec->Draw();
  g_phi_gen->Draw("same");

  Egen = g_phi_gen->GetEntries();
  Erec = g_phi_rec->GetEntries();

  sprintf(text1,"Generated Photons %d",Egen);
  t1 = new TText(60., 200.,text1);
  
  sprintf(text2,"Reconstructed Photons %d",Erec);
  max1 = g_phi_rec->GetMaximum()*0.8; 
  t1 = new TText(60., max1,text1);
  t2 = new TText(60., max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_photons_phi.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_photons_phi.gif",atr);
  c2->SaveAs(nam);


  TH1D *m_pi0_gen = (TH1D*)f1->Get("mass_pi0");
  TH1D *m_pi0_rec = (TH1D*)f2->Get("mass_pi0");

  m_pi0_rec->SetLineColor(2);
  max1 = m_pi0_rec->GetMaximum()*1.5; 
  m_pi0_gen->GetYaxis()->SetRangeUser(0.,max1);
  m_pi0_gen->Draw();
  m_pi0_rec->Draw("same");
  Egen = m_pi0_gen->GetEntries();
  Erec = m_pi0_rec->GetEntries();
  max1 *=0.8; 
  sprintf(text1,"Generated Pi0 %d",Egen);
  t1 = new TText(0.2, max1,text1);
  sprintf(text2,"Reconstructed Pi0 %d",Erec);
  t2 = new TText(0.2, max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_pi0Mass.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_pi0Mass.gif",atr);
  c2->SaveAs(nam);

  TH1D *m_rho_gen = (TH1D*)f1->Get("mass_rho");
  TH1D *m_rho_rec = (TH1D*)f2->Get("mass_rho");

  m_rho_rec->SetLineColor(2);
  max1 = m_rho_rec->GetMaximum()*1.5; 
  m_rho_gen->GetYaxis()->SetRangeUser(0.,max1);
  m_rho_gen->Draw();
  m_rho_rec->Draw("same");
  Egen = m_rho_gen->GetEntries();
  Erec = m_rho_rec->GetEntries();
  max1 *=0.8; 
  sprintf(text1,"Generated Rho %d",Egen);
  t1 = new TText(0.8, max1,text1);
  sprintf(text2,"Reconstructed Rho %d",Erec);
  t2 = new TText(0.8, max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_rhoMass.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_rhoMass.gif",atr);
  c2->SaveAs(nam);

  TH1D *m_omega_gen = (TH1D*)f1->Get("mass_omega");
  TH1D *m_omega_rec = (TH1D*)f2->Get("mass_omega");

  m_omega_rec->SetLineColor(2);
  max1 = m_omega_rec->GetMaximum()*1.5; 
  m_omega_gen->GetYaxis()->SetRangeUser(0.,max1);
  m_omega_gen->Draw();
  m_omega_rec->Draw("same");
  Egen = m_omega_gen->GetEntries();
  Erec = m_omega_rec->GetEntries();
  max1 *=0.8; 
  sprintf(text1,"Generated Omega %d",Egen);
  t1 = new TText(0.8, max1,text1);
  sprintf(text2,"Reconstructed Omega %d",Erec);
  t2 = new TText(0.8, max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_omegaMass.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_omegaMass.gif",atr);
  c2->SaveAs(nam);

  TH1D *m_b1_gen = (TH1D*)f1->Get("mass_b1");
  TH1D *m_b1_rec = (TH1D*)f2->Get("mass_b1");

  m_b1_rec->SetLineColor(2);
  max1 = m_b1_rec->GetMaximum()*1.5; 
  m_b1_gen->GetYaxis()->SetRangeUser(0.,max1);
  m_b1_gen->Draw();
  m_b1_rec->Draw("same");
  Egen = m_b1_gen->GetEntries();
  Erec = m_b1_rec->GetEntries();
  max1 *=0.8; 
  sprintf(text1,"Generated b1 %d",Egen);
  t1 = new TText(0.15, max1,text1);
  sprintf(text2,"Reconstructed b1 %d",Erec);
  t2 = new TText(0.15, max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_b1Mass.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_b1Mass.gif",atr);
  c2->SaveAs(nam);

  TH1D *m_X_gen = (TH1D*)f1->Get("mass_X");
  TH1D *m_X_rec = (TH1D*)f2->Get("mass_X");

  m_X_rec->SetLineColor(2);
  max1 = m_X_rec->GetMaximum()*1.5; 
  m_X_gen->GetYaxis()->SetRangeUser(0.,max1);
  m_X_gen->Draw();
  m_X_rec->Draw("same");
  Egen = m_X_gen->GetEntries();
  Erec = m_X_rec->GetEntries();
  max1 *=0.8; 
  sprintf(text1,"Generated X %d",Egen);
  t1 = new TText(0.1, max1,text1);
  sprintf(text2,"Reconstructed X %d",Erec);
  t2 = new TText(0.1, max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_XMass.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_XMass.gif",atr);
  c2->SaveAs(nam);

  TH1D *vertex_gen =  (TH1D*)f1->Get("vertex_z");
  TH1D *vertex_rec =  (TH1D*)f2->Get("vertex_z");
  vertex_rec->SetLineColor(2);
  max1 = vertex_gen->GetMaximum()*1.1; 
  vertex_gen->GetYaxis()->SetRangeUser(0.,max1);
  vertex_gen->Draw();
  vertex_rec->Draw("same");
  Egen = vertex_gen->GetEntries();
  Erec = vertex_rec->GetEntries();
  max1 *=0.8; 
  sprintf(text1,"Generated Events %d",Egen);
  t1 = new TText(0.1, max1,text1);
  sprintf(text2,"Reconstructed Events %d",Erec);
  t2 = new TText(0.1, max1*0.9,text2);
  t2->SetTextColor(2);
  t1->Draw();
  t2->Draw();
  sprintf(nam,"b1pi_%s_zVertex.pdf",atr);
  c2->SaveAs(nam);
  sprintf(nam,"b1pi_%s_zVertex.gif",atr);
  c2->SaveAs(nam);

}
