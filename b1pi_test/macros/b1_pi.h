//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Aug 23 11:31:50 2010 by ROOT version 5.24/00
// from TTree recon/Reconstructed Event parameters
// found on file: hd_root.root
//////////////////////////////////////////////////////////

#ifndef b1_pi_h
#define b1_pi_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
   const Int_t kMaxpip = 4;
   const Int_t kMaxpim = 4;
   const Int_t kMaxKp = 1;
   const Int_t kMaxKm = 1;
   const Int_t kMaxproton = 4;
   const Int_t kMaxphoton = 5;
   const Int_t kMaxpip_match = 4;
   const Int_t kMaxpim_match = 4;
   const Int_t kMaxKp_match = 1;
   const Int_t kMaxKm_match = 1;
   const Int_t kMaxproton_match = 4;
   const Int_t kMaxphoton_match = 5;

class b1_pi : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leaf types
 //Event           *R;
   UInt_t          fUniqueID;
   UInt_t          fBits;
   Int_t           event;
   UInt_t          Npip;
   UInt_t          Npim;
   UInt_t          NKp;
   UInt_t          NKm;
   UInt_t          Nproton;
   UInt_t          Nphoton;
   Int_t           pip_;
   UInt_t          pip_fUniqueID[kMaxpip];   //[pip_]
   UInt_t          pip_fBits[kMaxpip];   //[pip_]
   UInt_t          pip_p_fUniqueID[kMaxpip];   //[pip_]
   UInt_t          pip_p_fBits[kMaxpip];   //[pip_]
   UInt_t          pip_p_fP_fUniqueID[kMaxpip];   //[pip_]
   UInt_t          pip_p_fP_fBits[kMaxpip];   //[pip_]
   Double_t        pip_p_fP_fX[kMaxpip];   //[pip_]
   Double_t        pip_p_fP_fY[kMaxpip];   //[pip_]
   Double_t        pip_p_fP_fZ[kMaxpip];   //[pip_]
   Double_t        pip_p_fE[kMaxpip];   //[pip_]
   UInt_t          pip_x_fUniqueID[kMaxpip];   //[pip_]
   UInt_t          pip_x_fBits[kMaxpip];   //[pip_]
   Double_t        pip_x_fX[kMaxpip];   //[pip_]
   Double_t        pip_x_fY[kMaxpip];   //[pip_]
   Double_t        pip_x_fZ[kMaxpip];   //[pip_]
   Bool_t          pip_is_fiducial[kMaxpip];   //[pip_]
   Int_t           pim_;
   UInt_t          pim_fUniqueID[kMaxpim];   //[pim_]
   UInt_t          pim_fBits[kMaxpim];   //[pim_]
   UInt_t          pim_p_fUniqueID[kMaxpim];   //[pim_]
   UInt_t          pim_p_fBits[kMaxpim];   //[pim_]
   UInt_t          pim_p_fP_fUniqueID[kMaxpim];   //[pim_]
   UInt_t          pim_p_fP_fBits[kMaxpim];   //[pim_]
   Double_t        pim_p_fP_fX[kMaxpim];   //[pim_]
   Double_t        pim_p_fP_fY[kMaxpim];   //[pim_]
   Double_t        pim_p_fP_fZ[kMaxpim];   //[pim_]
   Double_t        pim_p_fE[kMaxpim];   //[pim_]
   UInt_t          pim_x_fUniqueID[kMaxpim];   //[pim_]
   UInt_t          pim_x_fBits[kMaxpim];   //[pim_]
   Double_t        pim_x_fX[kMaxpim];   //[pim_]
   Double_t        pim_x_fY[kMaxpim];   //[pim_]
   Double_t        pim_x_fZ[kMaxpim];   //[pim_]
   Bool_t          pim_is_fiducial[kMaxpim];   //[pim_]
   Int_t           Kp_;
   UInt_t          Kp_fUniqueID[kMaxKp];   //[Kp_]
   UInt_t          Kp_fBits[kMaxKp];   //[Kp_]
   UInt_t          Kp_p_fUniqueID[kMaxKp];   //[Kp_]
   UInt_t          Kp_p_fBits[kMaxKp];   //[Kp_]
   UInt_t          Kp_p_fP_fUniqueID[kMaxKp];   //[Kp_]
   UInt_t          Kp_p_fP_fBits[kMaxKp];   //[Kp_]
   Double_t        Kp_p_fP_fX[kMaxKp];   //[Kp_]
   Double_t        Kp_p_fP_fY[kMaxKp];   //[Kp_]
   Double_t        Kp_p_fP_fZ[kMaxKp];   //[Kp_]
   Double_t        Kp_p_fE[kMaxKp];   //[Kp_]
   UInt_t          Kp_x_fUniqueID[kMaxKp];   //[Kp_]
   UInt_t          Kp_x_fBits[kMaxKp];   //[Kp_]
   Double_t        Kp_x_fX[kMaxKp];   //[Kp_]
   Double_t        Kp_x_fY[kMaxKp];   //[Kp_]
   Double_t        Kp_x_fZ[kMaxKp];   //[Kp_]
   Bool_t          Kp_is_fiducial[kMaxKp];   //[Kp_]
   Int_t           Km_;
   UInt_t          Km_fUniqueID[kMaxKm];   //[Km_]
   UInt_t          Km_fBits[kMaxKm];   //[Km_]
   UInt_t          Km_p_fUniqueID[kMaxKm];   //[Km_]
   UInt_t          Km_p_fBits[kMaxKm];   //[Km_]
   UInt_t          Km_p_fP_fUniqueID[kMaxKm];   //[Km_]
   UInt_t          Km_p_fP_fBits[kMaxKm];   //[Km_]
   Double_t        Km_p_fP_fX[kMaxKm];   //[Km_]
   Double_t        Km_p_fP_fY[kMaxKm];   //[Km_]
   Double_t        Km_p_fP_fZ[kMaxKm];   //[Km_]
   Double_t        Km_p_fE[kMaxKm];   //[Km_]
   UInt_t          Km_x_fUniqueID[kMaxKm];   //[Km_]
   UInt_t          Km_x_fBits[kMaxKm];   //[Km_]
   Double_t        Km_x_fX[kMaxKm];   //[Km_]
   Double_t        Km_x_fY[kMaxKm];   //[Km_]
   Double_t        Km_x_fZ[kMaxKm];   //[Km_]
   Bool_t          Km_is_fiducial[kMaxKm];   //[Km_]
   Int_t           proton_;
   UInt_t          proton_fUniqueID[kMaxproton];   //[proton_]
   UInt_t          proton_fBits[kMaxproton];   //[proton_]
   UInt_t          proton_p_fUniqueID[kMaxproton];   //[proton_]
   UInt_t          proton_p_fBits[kMaxproton];   //[proton_]
   UInt_t          proton_p_fP_fUniqueID[kMaxproton];   //[proton_]
   UInt_t          proton_p_fP_fBits[kMaxproton];   //[proton_]
   Double_t        proton_p_fP_fX[kMaxproton];   //[proton_]
   Double_t        proton_p_fP_fY[kMaxproton];   //[proton_]
   Double_t        proton_p_fP_fZ[kMaxproton];   //[proton_]
   Double_t        proton_p_fE[kMaxproton];   //[proton_]
   UInt_t          proton_x_fUniqueID[kMaxproton];   //[proton_]
   UInt_t          proton_x_fBits[kMaxproton];   //[proton_]
   Double_t        proton_x_fX[kMaxproton];   //[proton_]
   Double_t        proton_x_fY[kMaxproton];   //[proton_]
   Double_t        proton_x_fZ[kMaxproton];   //[proton_]
   Bool_t          proton_is_fiducial[kMaxproton];   //[proton_]
   Int_t           photon_;
   UInt_t          photon_fUniqueID[kMaxphoton];   //[photon_]
   UInt_t          photon_fBits[kMaxphoton];   //[photon_]
   UInt_t          photon_p_fUniqueID[kMaxphoton];   //[photon_]
   UInt_t          photon_p_fBits[kMaxphoton];   //[photon_]
   UInt_t          photon_p_fP_fUniqueID[kMaxphoton];   //[photon_]
   UInt_t          photon_p_fP_fBits[kMaxphoton];   //[photon_]
   Double_t        photon_p_fP_fX[kMaxphoton];   //[photon_]
   Double_t        photon_p_fP_fY[kMaxphoton];   //[photon_]
   Double_t        photon_p_fP_fZ[kMaxphoton];   //[photon_]
   Double_t        photon_p_fE[kMaxphoton];   //[photon_]
   UInt_t          photon_x_fUniqueID[kMaxphoton];   //[photon_]
   UInt_t          photon_x_fBits[kMaxphoton];   //[photon_]
   Double_t        photon_x_fX[kMaxphoton];   //[photon_]
   Double_t        photon_x_fY[kMaxphoton];   //[photon_]
   Double_t        photon_x_fZ[kMaxphoton];   //[photon_]
   Bool_t          photon_is_fiducial[kMaxphoton];   //[photon_]
   Int_t           pip_match_;
   UInt_t          pip_match_fUniqueID[kMaxpip_match];   //[pip_match_]
   UInt_t          pip_match_fBits[kMaxpip_match];   //[pip_match_]
   UInt_t          pip_match_p_fUniqueID[kMaxpip_match];   //[pip_match_]
   UInt_t          pip_match_p_fBits[kMaxpip_match];   //[pip_match_]
   UInt_t          pip_match_p_fP_fUniqueID[kMaxpip_match];   //[pip_match_]
   UInt_t          pip_match_p_fP_fBits[kMaxpip_match];   //[pip_match_]
   Double_t        pip_match_p_fP_fX[kMaxpip_match];   //[pip_match_]
   Double_t        pip_match_p_fP_fY[kMaxpip_match];   //[pip_match_]
   Double_t        pip_match_p_fP_fZ[kMaxpip_match];   //[pip_match_]
   Double_t        pip_match_p_fE[kMaxpip_match];   //[pip_match_]
   UInt_t          pip_match_x_fUniqueID[kMaxpip_match];   //[pip_match_]
   UInt_t          pip_match_x_fBits[kMaxpip_match];   //[pip_match_]
   Double_t        pip_match_x_fX[kMaxpip_match];   //[pip_match_]
   Double_t        pip_match_x_fY[kMaxpip_match];   //[pip_match_]
   Double_t        pip_match_x_fZ[kMaxpip_match];   //[pip_match_]
   Bool_t          pip_match_is_fiducial[kMaxpip_match];   //[pip_match_]
   Int_t           pim_match_;
   UInt_t          pim_match_fUniqueID[kMaxpim_match];   //[pim_match_]
   UInt_t          pim_match_fBits[kMaxpim_match];   //[pim_match_]
   UInt_t          pim_match_p_fUniqueID[kMaxpim_match];   //[pim_match_]
   UInt_t          pim_match_p_fBits[kMaxpim_match];   //[pim_match_]
   UInt_t          pim_match_p_fP_fUniqueID[kMaxpim_match];   //[pim_match_]
   UInt_t          pim_match_p_fP_fBits[kMaxpim_match];   //[pim_match_]
   Double_t        pim_match_p_fP_fX[kMaxpim_match];   //[pim_match_]
   Double_t        pim_match_p_fP_fY[kMaxpim_match];   //[pim_match_]
   Double_t        pim_match_p_fP_fZ[kMaxpim_match];   //[pim_match_]
   Double_t        pim_match_p_fE[kMaxpim_match];   //[pim_match_]
   UInt_t          pim_match_x_fUniqueID[kMaxpim_match];   //[pim_match_]
   UInt_t          pim_match_x_fBits[kMaxpim_match];   //[pim_match_]
   Double_t        pim_match_x_fX[kMaxpim_match];   //[pim_match_]
   Double_t        pim_match_x_fY[kMaxpim_match];   //[pim_match_]
   Double_t        pim_match_x_fZ[kMaxpim_match];   //[pim_match_]
   Bool_t          pim_match_is_fiducial[kMaxpim_match];   //[pim_match_]
   Int_t           Kp_match_;
   UInt_t          Kp_match_fUniqueID[kMaxKp_match];   //[Kp_match_]
   UInt_t          Kp_match_fBits[kMaxKp_match];   //[Kp_match_]
   UInt_t          Kp_match_p_fUniqueID[kMaxKp_match];   //[Kp_match_]
   UInt_t          Kp_match_p_fBits[kMaxKp_match];   //[Kp_match_]
   UInt_t          Kp_match_p_fP_fUniqueID[kMaxKp_match];   //[Kp_match_]
   UInt_t          Kp_match_p_fP_fBits[kMaxKp_match];   //[Kp_match_]
   Double_t        Kp_match_p_fP_fX[kMaxKp_match];   //[Kp_match_]
   Double_t        Kp_match_p_fP_fY[kMaxKp_match];   //[Kp_match_]
   Double_t        Kp_match_p_fP_fZ[kMaxKp_match];   //[Kp_match_]
   Double_t        Kp_match_p_fE[kMaxKp_match];   //[Kp_match_]
   UInt_t          Kp_match_x_fUniqueID[kMaxKp_match];   //[Kp_match_]
   UInt_t          Kp_match_x_fBits[kMaxKp_match];   //[Kp_match_]
   Double_t        Kp_match_x_fX[kMaxKp_match];   //[Kp_match_]
   Double_t        Kp_match_x_fY[kMaxKp_match];   //[Kp_match_]
   Double_t        Kp_match_x_fZ[kMaxKp_match];   //[Kp_match_]
   Bool_t          Kp_match_is_fiducial[kMaxKp_match];   //[Kp_match_]
   Int_t           Km_match_;
   UInt_t          Km_match_fUniqueID[kMaxKm_match];   //[Km_match_]
   UInt_t          Km_match_fBits[kMaxKm_match];   //[Km_match_]
   UInt_t          Km_match_p_fUniqueID[kMaxKm_match];   //[Km_match_]
   UInt_t          Km_match_p_fBits[kMaxKm_match];   //[Km_match_]
   UInt_t          Km_match_p_fP_fUniqueID[kMaxKm_match];   //[Km_match_]
   UInt_t          Km_match_p_fP_fBits[kMaxKm_match];   //[Km_match_]
   Double_t        Km_match_p_fP_fX[kMaxKm_match];   //[Km_match_]
   Double_t        Km_match_p_fP_fY[kMaxKm_match];   //[Km_match_]
   Double_t        Km_match_p_fP_fZ[kMaxKm_match];   //[Km_match_]
   Double_t        Km_match_p_fE[kMaxKm_match];   //[Km_match_]
   UInt_t          Km_match_x_fUniqueID[kMaxKm_match];   //[Km_match_]
   UInt_t          Km_match_x_fBits[kMaxKm_match];   //[Km_match_]
   Double_t        Km_match_x_fX[kMaxKm_match];   //[Km_match_]
   Double_t        Km_match_x_fY[kMaxKm_match];   //[Km_match_]
   Double_t        Km_match_x_fZ[kMaxKm_match];   //[Km_match_]
   Bool_t          Km_match_is_fiducial[kMaxKm_match];   //[Km_match_]
   Int_t           proton_match_;
   UInt_t          proton_match_fUniqueID[kMaxproton_match];   //[proton_match_]
   UInt_t          proton_match_fBits[kMaxproton_match];   //[proton_match_]
   UInt_t          proton_match_p_fUniqueID[kMaxproton_match];   //[proton_match_]
   UInt_t          proton_match_p_fBits[kMaxproton_match];   //[proton_match_]
   UInt_t          proton_match_p_fP_fUniqueID[kMaxproton_match];   //[proton_match_]
   UInt_t          proton_match_p_fP_fBits[kMaxproton_match];   //[proton_match_]
   Double_t        proton_match_p_fP_fX[kMaxproton_match];   //[proton_match_]
   Double_t        proton_match_p_fP_fY[kMaxproton_match];   //[proton_match_]
   Double_t        proton_match_p_fP_fZ[kMaxproton_match];   //[proton_match_]
   Double_t        proton_match_p_fE[kMaxproton_match];   //[proton_match_]
   UInt_t          proton_match_x_fUniqueID[kMaxproton_match];   //[proton_match_]
   UInt_t          proton_match_x_fBits[kMaxproton_match];   //[proton_match_]
   Double_t        proton_match_x_fX[kMaxproton_match];   //[proton_match_]
   Double_t        proton_match_x_fY[kMaxproton_match];   //[proton_match_]
   Double_t        proton_match_x_fZ[kMaxproton_match];   //[proton_match_]
   Bool_t          proton_match_is_fiducial[kMaxproton_match];   //[proton_match_]
   Int_t           photon_match_;
   UInt_t          photon_match_fUniqueID[kMaxphoton_match];   //[photon_match_]
   UInt_t          photon_match_fBits[kMaxphoton_match];   //[photon_match_]
   UInt_t          photon_match_p_fUniqueID[kMaxphoton_match];   //[photon_match_]
   UInt_t          photon_match_p_fBits[kMaxphoton_match];   //[photon_match_]
   UInt_t          photon_match_p_fP_fUniqueID[kMaxphoton_match];   //[photon_match_]
   UInt_t          photon_match_p_fP_fBits[kMaxphoton_match];   //[photon_match_]
   Double_t        photon_match_p_fP_fX[kMaxphoton_match];   //[photon_match_]
   Double_t        photon_match_p_fP_fY[kMaxphoton_match];   //[photon_match_]
   Double_t        photon_match_p_fP_fZ[kMaxphoton_match];   //[photon_match_]
   Double_t        photon_match_p_fE[kMaxphoton_match];   //[photon_match_]
   UInt_t          photon_match_x_fUniqueID[kMaxphoton_match];   //[photon_match_]
   UInt_t          photon_match_x_fBits[kMaxphoton_match];   //[photon_match_]
   Double_t        photon_match_x_fX[kMaxphoton_match];   //[photon_match_]
   Double_t        photon_match_x_fY[kMaxphoton_match];   //[photon_match_]
   Double_t        photon_match_x_fZ[kMaxphoton_match];   //[photon_match_]
   Bool_t          photon_match_is_fiducial[kMaxphoton_match];   //[photon_match_]
   UInt_t          target_fUniqueID;
   UInt_t          target_fBits;
   UInt_t          target_fP_fUniqueID;
   UInt_t          target_fP_fBits;
   Double_t        target_fP_fX;
   Double_t        target_fP_fY;
   Double_t        target_fP_fZ;
   Double_t        target_fE;
   UInt_t          beam_fUniqueID;
   UInt_t          beam_fBits;
   UInt_t          beam_fP_fUniqueID;
   UInt_t          beam_fP_fBits;
   Double_t        beam_fP_fX;
   Double_t        beam_fP_fY;
   Double_t        beam_fP_fZ;
   Double_t        beam_fE;
   UInt_t          vertex_fUniqueID;
   UInt_t          vertex_fBits;
   Double_t        vertex_fX;
   Double_t        vertex_fY;
   Double_t        vertex_fZ;
   UInt_t          W_fUniqueID;
   UInt_t          W_fBits;
   UInt_t          W_fP_fUniqueID;
   UInt_t          W_fP_fBits;
   Double_t        W_fP_fX;
   Double_t        W_fP_fY;
   Double_t        W_fP_fZ;
   Double_t        W_fE;
   Bool_t          all_fiducial;
   Bool_t          all_mesons_fiducial;
   Bool_t          all_photons_fiducial;
   Bool_t          all_protons_fiducial;

   // List of branches
   TBranch        *b_R_fUniqueID;   //!
   TBranch        *b_R_fBits;   //!
   TBranch        *b_R_event;   //!
   TBranch        *b_R_Npip;   //!
   TBranch        *b_R_Npim;   //!
   TBranch        *b_R_NKp;   //!
   TBranch        *b_R_NKm;   //!
   TBranch        *b_R_Nproton;   //!
   TBranch        *b_R_Nphoton;   //!
   TBranch        *b_R_pip_;   //!
   TBranch        *b_pip_fUniqueID;   //!
   TBranch        *b_pip_fBits;   //!
   TBranch        *b_pip_p_fUniqueID;   //!
   TBranch        *b_pip_p_fBits;   //!
   TBranch        *b_pip_p_fP_fUniqueID;   //!
   TBranch        *b_pip_p_fP_fBits;   //!
   TBranch        *b_pip_p_fP_fX;   //!
   TBranch        *b_pip_p_fP_fY;   //!
   TBranch        *b_pip_p_fP_fZ;   //!
   TBranch        *b_pip_p_fE;   //!
   TBranch        *b_pip_x_fUniqueID;   //!
   TBranch        *b_pip_x_fBits;   //!
   TBranch        *b_pip_x_fX;   //!
   TBranch        *b_pip_x_fY;   //!
   TBranch        *b_pip_x_fZ;   //!
   TBranch        *b_pip_is_fiducial;   //!
   TBranch        *b_R_pim_;   //!
   TBranch        *b_pim_fUniqueID;   //!
   TBranch        *b_pim_fBits;   //!
   TBranch        *b_pim_p_fUniqueID;   //!
   TBranch        *b_pim_p_fBits;   //!
   TBranch        *b_pim_p_fP_fUniqueID;   //!
   TBranch        *b_pim_p_fP_fBits;   //!
   TBranch        *b_pim_p_fP_fX;   //!
   TBranch        *b_pim_p_fP_fY;   //!
   TBranch        *b_pim_p_fP_fZ;   //!
   TBranch        *b_pim_p_fE;   //!
   TBranch        *b_pim_x_fUniqueID;   //!
   TBranch        *b_pim_x_fBits;   //!
   TBranch        *b_pim_x_fX;   //!
   TBranch        *b_pim_x_fY;   //!
   TBranch        *b_pim_x_fZ;   //!
   TBranch        *b_pim_is_fiducial;   //!
   TBranch        *b_R_Kp_;   //!
   TBranch        *b_Kp_fUniqueID;   //!
   TBranch        *b_Kp_fBits;   //!
   TBranch        *b_Kp_p_fUniqueID;   //!
   TBranch        *b_Kp_p_fBits;   //!
   TBranch        *b_Kp_p_fP_fUniqueID;   //!
   TBranch        *b_Kp_p_fP_fBits;   //!
   TBranch        *b_Kp_p_fP_fX;   //!
   TBranch        *b_Kp_p_fP_fY;   //!
   TBranch        *b_Kp_p_fP_fZ;   //!
   TBranch        *b_Kp_p_fE;   //!
   TBranch        *b_Kp_x_fUniqueID;   //!
   TBranch        *b_Kp_x_fBits;   //!
   TBranch        *b_Kp_x_fX;   //!
   TBranch        *b_Kp_x_fY;   //!
   TBranch        *b_Kp_x_fZ;   //!
   TBranch        *b_Kp_is_fiducial;   //!
   TBranch        *b_R_Km_;   //!
   TBranch        *b_Km_fUniqueID;   //!
   TBranch        *b_Km_fBits;   //!
   TBranch        *b_Km_p_fUniqueID;   //!
   TBranch        *b_Km_p_fBits;   //!
   TBranch        *b_Km_p_fP_fUniqueID;   //!
   TBranch        *b_Km_p_fP_fBits;   //!
   TBranch        *b_Km_p_fP_fX;   //!
   TBranch        *b_Km_p_fP_fY;   //!
   TBranch        *b_Km_p_fP_fZ;   //!
   TBranch        *b_Km_p_fE;   //!
   TBranch        *b_Km_x_fUniqueID;   //!
   TBranch        *b_Km_x_fBits;   //!
   TBranch        *b_Km_x_fX;   //!
   TBranch        *b_Km_x_fY;   //!
   TBranch        *b_Km_x_fZ;   //!
   TBranch        *b_Km_is_fiducial;   //!
   TBranch        *b_R_proton_;   //!
   TBranch        *b_proton_fUniqueID;   //!
   TBranch        *b_proton_fBits;   //!
   TBranch        *b_proton_p_fUniqueID;   //!
   TBranch        *b_proton_p_fBits;   //!
   TBranch        *b_proton_p_fP_fUniqueID;   //!
   TBranch        *b_proton_p_fP_fBits;   //!
   TBranch        *b_proton_p_fP_fX;   //!
   TBranch        *b_proton_p_fP_fY;   //!
   TBranch        *b_proton_p_fP_fZ;   //!
   TBranch        *b_proton_p_fE;   //!
   TBranch        *b_proton_x_fUniqueID;   //!
   TBranch        *b_proton_x_fBits;   //!
   TBranch        *b_proton_x_fX;   //!
   TBranch        *b_proton_x_fY;   //!
   TBranch        *b_proton_x_fZ;   //!
   TBranch        *b_proton_is_fiducial;   //!
   TBranch        *b_R_photon_;   //!
   TBranch        *b_photon_fUniqueID;   //!
   TBranch        *b_photon_fBits;   //!
   TBranch        *b_photon_p_fUniqueID;   //!
   TBranch        *b_photon_p_fBits;   //!
   TBranch        *b_photon_p_fP_fUniqueID;   //!
   TBranch        *b_photon_p_fP_fBits;   //!
   TBranch        *b_photon_p_fP_fX;   //!
   TBranch        *b_photon_p_fP_fY;   //!
   TBranch        *b_photon_p_fP_fZ;   //!
   TBranch        *b_photon_p_fE;   //!
   TBranch        *b_photon_x_fUniqueID;   //!
   TBranch        *b_photon_x_fBits;   //!
   TBranch        *b_photon_x_fX;   //!
   TBranch        *b_photon_x_fY;   //!
   TBranch        *b_photon_x_fZ;   //!
   TBranch        *b_photon_is_fiducial;   //!
   TBranch        *b_R_pip_match_;   //!
   TBranch        *b_pip_match_fUniqueID;   //!
   TBranch        *b_pip_match_fBits;   //!
   TBranch        *b_pip_match_p_fUniqueID;   //!
   TBranch        *b_pip_match_p_fBits;   //!
   TBranch        *b_pip_match_p_fP_fUniqueID;   //!
   TBranch        *b_pip_match_p_fP_fBits;   //!
   TBranch        *b_pip_match_p_fP_fX;   //!
   TBranch        *b_pip_match_p_fP_fY;   //!
   TBranch        *b_pip_match_p_fP_fZ;   //!
   TBranch        *b_pip_match_p_fE;   //!
   TBranch        *b_pip_match_x_fUniqueID;   //!
   TBranch        *b_pip_match_x_fBits;   //!
   TBranch        *b_pip_match_x_fX;   //!
   TBranch        *b_pip_match_x_fY;   //!
   TBranch        *b_pip_match_x_fZ;   //!
   TBranch        *b_pip_match_is_fiducial;   //!
   TBranch        *b_R_pim_match_;   //!
   TBranch        *b_pim_match_fUniqueID;   //!
   TBranch        *b_pim_match_fBits;   //!
   TBranch        *b_pim_match_p_fUniqueID;   //!
   TBranch        *b_pim_match_p_fBits;   //!
   TBranch        *b_pim_match_p_fP_fUniqueID;   //!
   TBranch        *b_pim_match_p_fP_fBits;   //!
   TBranch        *b_pim_match_p_fP_fX;   //!
   TBranch        *b_pim_match_p_fP_fY;   //!
   TBranch        *b_pim_match_p_fP_fZ;   //!
   TBranch        *b_pim_match_p_fE;   //!
   TBranch        *b_pim_match_x_fUniqueID;   //!
   TBranch        *b_pim_match_x_fBits;   //!
   TBranch        *b_pim_match_x_fX;   //!
   TBranch        *b_pim_match_x_fY;   //!
   TBranch        *b_pim_match_x_fZ;   //!
   TBranch        *b_pim_match_is_fiducial;   //!
   TBranch        *b_R_Kp_match_;   //!
   TBranch        *b_Kp_match_fUniqueID;   //!
   TBranch        *b_Kp_match_fBits;   //!
   TBranch        *b_Kp_match_p_fUniqueID;   //!
   TBranch        *b_Kp_match_p_fBits;   //!
   TBranch        *b_Kp_match_p_fP_fUniqueID;   //!
   TBranch        *b_Kp_match_p_fP_fBits;   //!
   TBranch        *b_Kp_match_p_fP_fX;   //!
   TBranch        *b_Kp_match_p_fP_fY;   //!
   TBranch        *b_Kp_match_p_fP_fZ;   //!
   TBranch        *b_Kp_match_p_fE;   //!
   TBranch        *b_Kp_match_x_fUniqueID;   //!
   TBranch        *b_Kp_match_x_fBits;   //!
   TBranch        *b_Kp_match_x_fX;   //!
   TBranch        *b_Kp_match_x_fY;   //!
   TBranch        *b_Kp_match_x_fZ;   //!
   TBranch        *b_Kp_match_is_fiducial;   //!
   TBranch        *b_R_Km_match_;   //!
   TBranch        *b_Km_match_fUniqueID;   //!
   TBranch        *b_Km_match_fBits;   //!
   TBranch        *b_Km_match_p_fUniqueID;   //!
   TBranch        *b_Km_match_p_fBits;   //!
   TBranch        *b_Km_match_p_fP_fUniqueID;   //!
   TBranch        *b_Km_match_p_fP_fBits;   //!
   TBranch        *b_Km_match_p_fP_fX;   //!
   TBranch        *b_Km_match_p_fP_fY;   //!
   TBranch        *b_Km_match_p_fP_fZ;   //!
   TBranch        *b_Km_match_p_fE;   //!
   TBranch        *b_Km_match_x_fUniqueID;   //!
   TBranch        *b_Km_match_x_fBits;   //!
   TBranch        *b_Km_match_x_fX;   //!
   TBranch        *b_Km_match_x_fY;   //!
   TBranch        *b_Km_match_x_fZ;   //!
   TBranch        *b_Km_match_is_fiducial;   //!
   TBranch        *b_R_proton_match_;   //!
   TBranch        *b_proton_match_fUniqueID;   //!
   TBranch        *b_proton_match_fBits;   //!
   TBranch        *b_proton_match_p_fUniqueID;   //!
   TBranch        *b_proton_match_p_fBits;   //!
   TBranch        *b_proton_match_p_fP_fUniqueID;   //!
   TBranch        *b_proton_match_p_fP_fBits;   //!
   TBranch        *b_proton_match_p_fP_fX;   //!
   TBranch        *b_proton_match_p_fP_fY;   //!
   TBranch        *b_proton_match_p_fP_fZ;   //!
   TBranch        *b_proton_match_p_fE;   //!
   TBranch        *b_proton_match_x_fUniqueID;   //!
   TBranch        *b_proton_match_x_fBits;   //!
   TBranch        *b_proton_match_x_fX;   //!
   TBranch        *b_proton_match_x_fY;   //!
   TBranch        *b_proton_match_x_fZ;   //!
   TBranch        *b_proton_match_is_fiducial;   //!
   TBranch        *b_R_photon_match_;   //!
   TBranch        *b_photon_match_fUniqueID;   //!
   TBranch        *b_photon_match_fBits;   //!
   TBranch        *b_photon_match_p_fUniqueID;   //!
   TBranch        *b_photon_match_p_fBits;   //!
   TBranch        *b_photon_match_p_fP_fUniqueID;   //!
   TBranch        *b_photon_match_p_fP_fBits;   //!
   TBranch        *b_photon_match_p_fP_fX;   //!
   TBranch        *b_photon_match_p_fP_fY;   //!
   TBranch        *b_photon_match_p_fP_fZ;   //!
   TBranch        *b_photon_match_p_fE;   //!
   TBranch        *b_photon_match_x_fUniqueID;   //!
   TBranch        *b_photon_match_x_fBits;   //!
   TBranch        *b_photon_match_x_fX;   //!
   TBranch        *b_photon_match_x_fY;   //!
   TBranch        *b_photon_match_x_fZ;   //!
   TBranch        *b_photon_match_is_fiducial;   //!
   TBranch        *b_R_target_fUniqueID;   //!
   TBranch        *b_R_target_fBits;   //!
   TBranch        *b_R_target_fP_fUniqueID;   //!
   TBranch        *b_R_target_fP_fBits;   //!
   TBranch        *b_R_target_fP_fX;   //!
   TBranch        *b_R_target_fP_fY;   //!
   TBranch        *b_R_target_fP_fZ;   //!
   TBranch        *b_R_target_fE;   //!
   TBranch        *b_R_beam_fUniqueID;   //!
   TBranch        *b_R_beam_fBits;   //!
   TBranch        *b_R_beam_fP_fUniqueID;   //!
   TBranch        *b_R_beam_fP_fBits;   //!
   TBranch        *b_R_beam_fP_fX;   //!
   TBranch        *b_R_beam_fP_fY;   //!
   TBranch        *b_R_beam_fP_fZ;   //!
   TBranch        *b_R_beam_fE;   //!
   TBranch        *b_R_vertex_fUniqueID;   //!
   TBranch        *b_R_vertex_fBits;   //!
   TBranch        *b_R_vertex_fX;   //!
   TBranch        *b_R_vertex_fY;   //!
   TBranch        *b_R_vertex_fZ;   //!
   TBranch        *b_R_W_fUniqueID;   //!
   TBranch        *b_R_W_fBits;   //!
   TBranch        *b_R_W_fP_fUniqueID;   //!
   TBranch        *b_R_W_fP_fBits;   //!
   TBranch        *b_R_W_fP_fX;   //!
   TBranch        *b_R_W_fP_fY;   //!
   TBranch        *b_R_W_fP_fZ;   //!
   TBranch        *b_R_W_fE;   //!
   TBranch        *b_R_all_fiducial;   //!
   TBranch        *b_R_all_mesons_fiducial;   //!
   TBranch        *b_R_all_photons_fiducial;   //!
   TBranch        *b_R_all_protons_fiducial;   //!

   b1_pi(TTree * /*tree*/ =0) { }
   virtual ~b1_pi() { }
   virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) { fInput = input; }
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

	//-------------------------------------
	// Add pointers to my histograms as members of this class
	TH1D *mass_X;
	TH1D *mass_b1;
	TH1D *mass_omega;
	TH1D *mass_rho;
	TH1D *mass_pi0;

	Int_t Ngood;
	TH1D *mass_X_good;
	TH1D *mass_b1_good;
	TH1D *mass_omega_good;
	TH1D *mass_rho_good;
	TH1D *mass_pi0_good;
	
	TH1D *stats;
	//-------------------------------------

   ClassDef(b1_pi,0);
};

#endif

#ifdef b1_pi_cxx
void b1_pi::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("fUniqueID", &fUniqueID, &b_R_fUniqueID);
   fChain->SetBranchAddress("fBits", &fBits, &b_R_fBits);
   fChain->SetBranchAddress("event", &event, &b_R_event);
   fChain->SetBranchAddress("Npip", &Npip, &b_R_Npip);
   fChain->SetBranchAddress("Npim", &Npim, &b_R_Npim);
   fChain->SetBranchAddress("NKp", &NKp, &b_R_NKp);
   fChain->SetBranchAddress("NKm", &NKm, &b_R_NKm);
   fChain->SetBranchAddress("Nproton", &Nproton, &b_R_Nproton);
   fChain->SetBranchAddress("Nphoton", &Nphoton, &b_R_Nphoton);
   fChain->SetBranchAddress("pip", &pip_, &b_R_pip_);
   fChain->SetBranchAddress("pip.fUniqueID", pip_fUniqueID, &b_pip_fUniqueID);
   fChain->SetBranchAddress("pip.fBits", pip_fBits, &b_pip_fBits);
   fChain->SetBranchAddress("pip.p.fUniqueID", pip_p_fUniqueID, &b_pip_p_fUniqueID);
   fChain->SetBranchAddress("pip.p.fBits", pip_p_fBits, &b_pip_p_fBits);
   fChain->SetBranchAddress("pip.p.fP.fUniqueID", pip_p_fP_fUniqueID, &b_pip_p_fP_fUniqueID);
   fChain->SetBranchAddress("pip.p.fP.fBits", pip_p_fP_fBits, &b_pip_p_fP_fBits);
   fChain->SetBranchAddress("pip.p.fP.fX", pip_p_fP_fX, &b_pip_p_fP_fX);
   fChain->SetBranchAddress("pip.p.fP.fY", pip_p_fP_fY, &b_pip_p_fP_fY);
   fChain->SetBranchAddress("pip.p.fP.fZ", pip_p_fP_fZ, &b_pip_p_fP_fZ);
   fChain->SetBranchAddress("pip.p.fE", pip_p_fE, &b_pip_p_fE);
   fChain->SetBranchAddress("pip.x.fUniqueID", pip_x_fUniqueID, &b_pip_x_fUniqueID);
   fChain->SetBranchAddress("pip.x.fBits", pip_x_fBits, &b_pip_x_fBits);
   fChain->SetBranchAddress("pip.x.fX", pip_x_fX, &b_pip_x_fX);
   fChain->SetBranchAddress("pip.x.fY", pip_x_fY, &b_pip_x_fY);
   fChain->SetBranchAddress("pip.x.fZ", pip_x_fZ, &b_pip_x_fZ);
   fChain->SetBranchAddress("pip.is_fiducial", pip_is_fiducial, &b_pip_is_fiducial);
   fChain->SetBranchAddress("pim", &pim_, &b_R_pim_);
   fChain->SetBranchAddress("pim.fUniqueID", pim_fUniqueID, &b_pim_fUniqueID);
   fChain->SetBranchAddress("pim.fBits", pim_fBits, &b_pim_fBits);
   fChain->SetBranchAddress("pim.p.fUniqueID", pim_p_fUniqueID, &b_pim_p_fUniqueID);
   fChain->SetBranchAddress("pim.p.fBits", pim_p_fBits, &b_pim_p_fBits);
   fChain->SetBranchAddress("pim.p.fP.fUniqueID", pim_p_fP_fUniqueID, &b_pim_p_fP_fUniqueID);
   fChain->SetBranchAddress("pim.p.fP.fBits", pim_p_fP_fBits, &b_pim_p_fP_fBits);
   fChain->SetBranchAddress("pim.p.fP.fX", pim_p_fP_fX, &b_pim_p_fP_fX);
   fChain->SetBranchAddress("pim.p.fP.fY", pim_p_fP_fY, &b_pim_p_fP_fY);
   fChain->SetBranchAddress("pim.p.fP.fZ", pim_p_fP_fZ, &b_pim_p_fP_fZ);
   fChain->SetBranchAddress("pim.p.fE", pim_p_fE, &b_pim_p_fE);
   fChain->SetBranchAddress("pim.x.fUniqueID", pim_x_fUniqueID, &b_pim_x_fUniqueID);
   fChain->SetBranchAddress("pim.x.fBits", pim_x_fBits, &b_pim_x_fBits);
   fChain->SetBranchAddress("pim.x.fX", pim_x_fX, &b_pim_x_fX);
   fChain->SetBranchAddress("pim.x.fY", pim_x_fY, &b_pim_x_fY);
   fChain->SetBranchAddress("pim.x.fZ", pim_x_fZ, &b_pim_x_fZ);
   fChain->SetBranchAddress("pim.is_fiducial", pim_is_fiducial, &b_pim_is_fiducial);
   fChain->SetBranchAddress("Kp", &Kp_, &b_R_Kp_);
   fChain->SetBranchAddress("Kp.fUniqueID", &Kp_fUniqueID, &b_Kp_fUniqueID);
   fChain->SetBranchAddress("Kp.fBits", &Kp_fBits, &b_Kp_fBits);
   fChain->SetBranchAddress("Kp.p.fUniqueID", &Kp_p_fUniqueID, &b_Kp_p_fUniqueID);
   fChain->SetBranchAddress("Kp.p.fBits", &Kp_p_fBits, &b_Kp_p_fBits);
   fChain->SetBranchAddress("Kp.p.fP.fUniqueID", &Kp_p_fP_fUniqueID, &b_Kp_p_fP_fUniqueID);
   fChain->SetBranchAddress("Kp.p.fP.fBits", &Kp_p_fP_fBits, &b_Kp_p_fP_fBits);
   fChain->SetBranchAddress("Kp.p.fP.fX", &Kp_p_fP_fX, &b_Kp_p_fP_fX);
   fChain->SetBranchAddress("Kp.p.fP.fY", &Kp_p_fP_fY, &b_Kp_p_fP_fY);
   fChain->SetBranchAddress("Kp.p.fP.fZ", &Kp_p_fP_fZ, &b_Kp_p_fP_fZ);
   fChain->SetBranchAddress("Kp.p.fE", &Kp_p_fE, &b_Kp_p_fE);
   fChain->SetBranchAddress("Kp.x.fUniqueID", &Kp_x_fUniqueID, &b_Kp_x_fUniqueID);
   fChain->SetBranchAddress("Kp.x.fBits", &Kp_x_fBits, &b_Kp_x_fBits);
   fChain->SetBranchAddress("Kp.x.fX", &Kp_x_fX, &b_Kp_x_fX);
   fChain->SetBranchAddress("Kp.x.fY", &Kp_x_fY, &b_Kp_x_fY);
   fChain->SetBranchAddress("Kp.x.fZ", &Kp_x_fZ, &b_Kp_x_fZ);
   fChain->SetBranchAddress("Kp.is_fiducial", &Kp_is_fiducial, &b_Kp_is_fiducial);
   fChain->SetBranchAddress("Km", &Km_, &b_R_Km_);
   fChain->SetBranchAddress("Km.fUniqueID", &Km_fUniqueID, &b_Km_fUniqueID);
   fChain->SetBranchAddress("Km.fBits", &Km_fBits, &b_Km_fBits);
   fChain->SetBranchAddress("Km.p.fUniqueID", &Km_p_fUniqueID, &b_Km_p_fUniqueID);
   fChain->SetBranchAddress("Km.p.fBits", &Km_p_fBits, &b_Km_p_fBits);
   fChain->SetBranchAddress("Km.p.fP.fUniqueID", &Km_p_fP_fUniqueID, &b_Km_p_fP_fUniqueID);
   fChain->SetBranchAddress("Km.p.fP.fBits", &Km_p_fP_fBits, &b_Km_p_fP_fBits);
   fChain->SetBranchAddress("Km.p.fP.fX", &Km_p_fP_fX, &b_Km_p_fP_fX);
   fChain->SetBranchAddress("Km.p.fP.fY", &Km_p_fP_fY, &b_Km_p_fP_fY);
   fChain->SetBranchAddress("Km.p.fP.fZ", &Km_p_fP_fZ, &b_Km_p_fP_fZ);
   fChain->SetBranchAddress("Km.p.fE", &Km_p_fE, &b_Km_p_fE);
   fChain->SetBranchAddress("Km.x.fUniqueID", &Km_x_fUniqueID, &b_Km_x_fUniqueID);
   fChain->SetBranchAddress("Km.x.fBits", &Km_x_fBits, &b_Km_x_fBits);
   fChain->SetBranchAddress("Km.x.fX", &Km_x_fX, &b_Km_x_fX);
   fChain->SetBranchAddress("Km.x.fY", &Km_x_fY, &b_Km_x_fY);
   fChain->SetBranchAddress("Km.x.fZ", &Km_x_fZ, &b_Km_x_fZ);
   fChain->SetBranchAddress("Km.is_fiducial", &Km_is_fiducial, &b_Km_is_fiducial);
   fChain->SetBranchAddress("proton", &proton_, &b_R_proton_);
   fChain->SetBranchAddress("proton.fUniqueID", proton_fUniqueID, &b_proton_fUniqueID);
   fChain->SetBranchAddress("proton.fBits", proton_fBits, &b_proton_fBits);
   fChain->SetBranchAddress("proton.p.fUniqueID", proton_p_fUniqueID, &b_proton_p_fUniqueID);
   fChain->SetBranchAddress("proton.p.fBits", proton_p_fBits, &b_proton_p_fBits);
   fChain->SetBranchAddress("proton.p.fP.fUniqueID", proton_p_fP_fUniqueID, &b_proton_p_fP_fUniqueID);
   fChain->SetBranchAddress("proton.p.fP.fBits", proton_p_fP_fBits, &b_proton_p_fP_fBits);
   fChain->SetBranchAddress("proton.p.fP.fX", proton_p_fP_fX, &b_proton_p_fP_fX);
   fChain->SetBranchAddress("proton.p.fP.fY", proton_p_fP_fY, &b_proton_p_fP_fY);
   fChain->SetBranchAddress("proton.p.fP.fZ", proton_p_fP_fZ, &b_proton_p_fP_fZ);
   fChain->SetBranchAddress("proton.p.fE", proton_p_fE, &b_proton_p_fE);
   fChain->SetBranchAddress("proton.x.fUniqueID", proton_x_fUniqueID, &b_proton_x_fUniqueID);
   fChain->SetBranchAddress("proton.x.fBits", proton_x_fBits, &b_proton_x_fBits);
   fChain->SetBranchAddress("proton.x.fX", proton_x_fX, &b_proton_x_fX);
   fChain->SetBranchAddress("proton.x.fY", proton_x_fY, &b_proton_x_fY);
   fChain->SetBranchAddress("proton.x.fZ", proton_x_fZ, &b_proton_x_fZ);
   fChain->SetBranchAddress("proton.is_fiducial", proton_is_fiducial, &b_proton_is_fiducial);
   fChain->SetBranchAddress("photon", &photon_, &b_R_photon_);
   fChain->SetBranchAddress("photon.fUniqueID", photon_fUniqueID, &b_photon_fUniqueID);
   fChain->SetBranchAddress("photon.fBits", photon_fBits, &b_photon_fBits);
   fChain->SetBranchAddress("photon.p.fUniqueID", photon_p_fUniqueID, &b_photon_p_fUniqueID);
   fChain->SetBranchAddress("photon.p.fBits", photon_p_fBits, &b_photon_p_fBits);
   fChain->SetBranchAddress("photon.p.fP.fUniqueID", photon_p_fP_fUniqueID, &b_photon_p_fP_fUniqueID);
   fChain->SetBranchAddress("photon.p.fP.fBits", photon_p_fP_fBits, &b_photon_p_fP_fBits);
   fChain->SetBranchAddress("photon.p.fP.fX", photon_p_fP_fX, &b_photon_p_fP_fX);
   fChain->SetBranchAddress("photon.p.fP.fY", photon_p_fP_fY, &b_photon_p_fP_fY);
   fChain->SetBranchAddress("photon.p.fP.fZ", photon_p_fP_fZ, &b_photon_p_fP_fZ);
   fChain->SetBranchAddress("photon.p.fE", photon_p_fE, &b_photon_p_fE);
   fChain->SetBranchAddress("photon.x.fUniqueID", photon_x_fUniqueID, &b_photon_x_fUniqueID);
   fChain->SetBranchAddress("photon.x.fBits", photon_x_fBits, &b_photon_x_fBits);
   fChain->SetBranchAddress("photon.x.fX", photon_x_fX, &b_photon_x_fX);
   fChain->SetBranchAddress("photon.x.fY", photon_x_fY, &b_photon_x_fY);
   fChain->SetBranchAddress("photon.x.fZ", photon_x_fZ, &b_photon_x_fZ);
   fChain->SetBranchAddress("photon.is_fiducial", photon_is_fiducial, &b_photon_is_fiducial);
   fChain->SetBranchAddress("pip_match", &pip_match_, &b_R_pip_match_);
   fChain->SetBranchAddress("pip_match.fUniqueID", pip_match_fUniqueID, &b_pip_match_fUniqueID);
   fChain->SetBranchAddress("pip_match.fBits", pip_match_fBits, &b_pip_match_fBits);
   fChain->SetBranchAddress("pip_match.p.fUniqueID", pip_match_p_fUniqueID, &b_pip_match_p_fUniqueID);
   fChain->SetBranchAddress("pip_match.p.fBits", pip_match_p_fBits, &b_pip_match_p_fBits);
   fChain->SetBranchAddress("pip_match.p.fP.fUniqueID", pip_match_p_fP_fUniqueID, &b_pip_match_p_fP_fUniqueID);
   fChain->SetBranchAddress("pip_match.p.fP.fBits", pip_match_p_fP_fBits, &b_pip_match_p_fP_fBits);
   fChain->SetBranchAddress("pip_match.p.fP.fX", pip_match_p_fP_fX, &b_pip_match_p_fP_fX);
   fChain->SetBranchAddress("pip_match.p.fP.fY", pip_match_p_fP_fY, &b_pip_match_p_fP_fY);
   fChain->SetBranchAddress("pip_match.p.fP.fZ", pip_match_p_fP_fZ, &b_pip_match_p_fP_fZ);
   fChain->SetBranchAddress("pip_match.p.fE", pip_match_p_fE, &b_pip_match_p_fE);
   fChain->SetBranchAddress("pip_match.x.fUniqueID", pip_match_x_fUniqueID, &b_pip_match_x_fUniqueID);
   fChain->SetBranchAddress("pip_match.x.fBits", pip_match_x_fBits, &b_pip_match_x_fBits);
   fChain->SetBranchAddress("pip_match.x.fX", pip_match_x_fX, &b_pip_match_x_fX);
   fChain->SetBranchAddress("pip_match.x.fY", pip_match_x_fY, &b_pip_match_x_fY);
   fChain->SetBranchAddress("pip_match.x.fZ", pip_match_x_fZ, &b_pip_match_x_fZ);
   fChain->SetBranchAddress("pip_match.is_fiducial", pip_match_is_fiducial, &b_pip_match_is_fiducial);
   fChain->SetBranchAddress("pim_match", &pim_match_, &b_R_pim_match_);
   fChain->SetBranchAddress("pim_match.fUniqueID", pim_match_fUniqueID, &b_pim_match_fUniqueID);
   fChain->SetBranchAddress("pim_match.fBits", pim_match_fBits, &b_pim_match_fBits);
   fChain->SetBranchAddress("pim_match.p.fUniqueID", pim_match_p_fUniqueID, &b_pim_match_p_fUniqueID);
   fChain->SetBranchAddress("pim_match.p.fBits", pim_match_p_fBits, &b_pim_match_p_fBits);
   fChain->SetBranchAddress("pim_match.p.fP.fUniqueID", pim_match_p_fP_fUniqueID, &b_pim_match_p_fP_fUniqueID);
   fChain->SetBranchAddress("pim_match.p.fP.fBits", pim_match_p_fP_fBits, &b_pim_match_p_fP_fBits);
   fChain->SetBranchAddress("pim_match.p.fP.fX", pim_match_p_fP_fX, &b_pim_match_p_fP_fX);
   fChain->SetBranchAddress("pim_match.p.fP.fY", pim_match_p_fP_fY, &b_pim_match_p_fP_fY);
   fChain->SetBranchAddress("pim_match.p.fP.fZ", pim_match_p_fP_fZ, &b_pim_match_p_fP_fZ);
   fChain->SetBranchAddress("pim_match.p.fE", pim_match_p_fE, &b_pim_match_p_fE);
   fChain->SetBranchAddress("pim_match.x.fUniqueID", pim_match_x_fUniqueID, &b_pim_match_x_fUniqueID);
   fChain->SetBranchAddress("pim_match.x.fBits", pim_match_x_fBits, &b_pim_match_x_fBits);
   fChain->SetBranchAddress("pim_match.x.fX", pim_match_x_fX, &b_pim_match_x_fX);
   fChain->SetBranchAddress("pim_match.x.fY", pim_match_x_fY, &b_pim_match_x_fY);
   fChain->SetBranchAddress("pim_match.x.fZ", pim_match_x_fZ, &b_pim_match_x_fZ);
   fChain->SetBranchAddress("pim_match.is_fiducial", pim_match_is_fiducial, &b_pim_match_is_fiducial);
   fChain->SetBranchAddress("Kp_match", &Kp_match_, &b_R_Kp_match_);
   fChain->SetBranchAddress("Kp_match.fUniqueID", &Kp_match_fUniqueID, &b_Kp_match_fUniqueID);
   fChain->SetBranchAddress("Kp_match.fBits", &Kp_match_fBits, &b_Kp_match_fBits);
   fChain->SetBranchAddress("Kp_match.p.fUniqueID", &Kp_match_p_fUniqueID, &b_Kp_match_p_fUniqueID);
   fChain->SetBranchAddress("Kp_match.p.fBits", &Kp_match_p_fBits, &b_Kp_match_p_fBits);
   fChain->SetBranchAddress("Kp_match.p.fP.fUniqueID", &Kp_match_p_fP_fUniqueID, &b_Kp_match_p_fP_fUniqueID);
   fChain->SetBranchAddress("Kp_match.p.fP.fBits", &Kp_match_p_fP_fBits, &b_Kp_match_p_fP_fBits);
   fChain->SetBranchAddress("Kp_match.p.fP.fX", &Kp_match_p_fP_fX, &b_Kp_match_p_fP_fX);
   fChain->SetBranchAddress("Kp_match.p.fP.fY", &Kp_match_p_fP_fY, &b_Kp_match_p_fP_fY);
   fChain->SetBranchAddress("Kp_match.p.fP.fZ", &Kp_match_p_fP_fZ, &b_Kp_match_p_fP_fZ);
   fChain->SetBranchAddress("Kp_match.p.fE", &Kp_match_p_fE, &b_Kp_match_p_fE);
   fChain->SetBranchAddress("Kp_match.x.fUniqueID", &Kp_match_x_fUniqueID, &b_Kp_match_x_fUniqueID);
   fChain->SetBranchAddress("Kp_match.x.fBits", &Kp_match_x_fBits, &b_Kp_match_x_fBits);
   fChain->SetBranchAddress("Kp_match.x.fX", &Kp_match_x_fX, &b_Kp_match_x_fX);
   fChain->SetBranchAddress("Kp_match.x.fY", &Kp_match_x_fY, &b_Kp_match_x_fY);
   fChain->SetBranchAddress("Kp_match.x.fZ", &Kp_match_x_fZ, &b_Kp_match_x_fZ);
   fChain->SetBranchAddress("Kp_match.is_fiducial", &Kp_match_is_fiducial, &b_Kp_match_is_fiducial);
   fChain->SetBranchAddress("Km_match", &Km_match_, &b_R_Km_match_);
   fChain->SetBranchAddress("Km_match.fUniqueID", &Km_match_fUniqueID, &b_Km_match_fUniqueID);
   fChain->SetBranchAddress("Km_match.fBits", &Km_match_fBits, &b_Km_match_fBits);
   fChain->SetBranchAddress("Km_match.p.fUniqueID", &Km_match_p_fUniqueID, &b_Km_match_p_fUniqueID);
   fChain->SetBranchAddress("Km_match.p.fBits", &Km_match_p_fBits, &b_Km_match_p_fBits);
   fChain->SetBranchAddress("Km_match.p.fP.fUniqueID", &Km_match_p_fP_fUniqueID, &b_Km_match_p_fP_fUniqueID);
   fChain->SetBranchAddress("Km_match.p.fP.fBits", &Km_match_p_fP_fBits, &b_Km_match_p_fP_fBits);
   fChain->SetBranchAddress("Km_match.p.fP.fX", &Km_match_p_fP_fX, &b_Km_match_p_fP_fX);
   fChain->SetBranchAddress("Km_match.p.fP.fY", &Km_match_p_fP_fY, &b_Km_match_p_fP_fY);
   fChain->SetBranchAddress("Km_match.p.fP.fZ", &Km_match_p_fP_fZ, &b_Km_match_p_fP_fZ);
   fChain->SetBranchAddress("Km_match.p.fE", &Km_match_p_fE, &b_Km_match_p_fE);
   fChain->SetBranchAddress("Km_match.x.fUniqueID", &Km_match_x_fUniqueID, &b_Km_match_x_fUniqueID);
   fChain->SetBranchAddress("Km_match.x.fBits", &Km_match_x_fBits, &b_Km_match_x_fBits);
   fChain->SetBranchAddress("Km_match.x.fX", &Km_match_x_fX, &b_Km_match_x_fX);
   fChain->SetBranchAddress("Km_match.x.fY", &Km_match_x_fY, &b_Km_match_x_fY);
   fChain->SetBranchAddress("Km_match.x.fZ", &Km_match_x_fZ, &b_Km_match_x_fZ);
   fChain->SetBranchAddress("Km_match.is_fiducial", &Km_match_is_fiducial, &b_Km_match_is_fiducial);
   fChain->SetBranchAddress("proton_match", &proton_match_, &b_R_proton_match_);
   fChain->SetBranchAddress("proton_match.fUniqueID", proton_match_fUniqueID, &b_proton_match_fUniqueID);
   fChain->SetBranchAddress("proton_match.fBits", proton_match_fBits, &b_proton_match_fBits);
   fChain->SetBranchAddress("proton_match.p.fUniqueID", proton_match_p_fUniqueID, &b_proton_match_p_fUniqueID);
   fChain->SetBranchAddress("proton_match.p.fBits", proton_match_p_fBits, &b_proton_match_p_fBits);
   fChain->SetBranchAddress("proton_match.p.fP.fUniqueID", proton_match_p_fP_fUniqueID, &b_proton_match_p_fP_fUniqueID);
   fChain->SetBranchAddress("proton_match.p.fP.fBits", proton_match_p_fP_fBits, &b_proton_match_p_fP_fBits);
   fChain->SetBranchAddress("proton_match.p.fP.fX", proton_match_p_fP_fX, &b_proton_match_p_fP_fX);
   fChain->SetBranchAddress("proton_match.p.fP.fY", proton_match_p_fP_fY, &b_proton_match_p_fP_fY);
   fChain->SetBranchAddress("proton_match.p.fP.fZ", proton_match_p_fP_fZ, &b_proton_match_p_fP_fZ);
   fChain->SetBranchAddress("proton_match.p.fE", proton_match_p_fE, &b_proton_match_p_fE);
   fChain->SetBranchAddress("proton_match.x.fUniqueID", proton_match_x_fUniqueID, &b_proton_match_x_fUniqueID);
   fChain->SetBranchAddress("proton_match.x.fBits", proton_match_x_fBits, &b_proton_match_x_fBits);
   fChain->SetBranchAddress("proton_match.x.fX", proton_match_x_fX, &b_proton_match_x_fX);
   fChain->SetBranchAddress("proton_match.x.fY", proton_match_x_fY, &b_proton_match_x_fY);
   fChain->SetBranchAddress("proton_match.x.fZ", proton_match_x_fZ, &b_proton_match_x_fZ);
   fChain->SetBranchAddress("proton_match.is_fiducial", proton_match_is_fiducial, &b_proton_match_is_fiducial);
   fChain->SetBranchAddress("photon_match", &photon_match_, &b_R_photon_match_);
   fChain->SetBranchAddress("photon_match.fUniqueID", photon_match_fUniqueID, &b_photon_match_fUniqueID);
   fChain->SetBranchAddress("photon_match.fBits", photon_match_fBits, &b_photon_match_fBits);
   fChain->SetBranchAddress("photon_match.p.fUniqueID", photon_match_p_fUniqueID, &b_photon_match_p_fUniqueID);
   fChain->SetBranchAddress("photon_match.p.fBits", photon_match_p_fBits, &b_photon_match_p_fBits);
   fChain->SetBranchAddress("photon_match.p.fP.fUniqueID", photon_match_p_fP_fUniqueID, &b_photon_match_p_fP_fUniqueID);
   fChain->SetBranchAddress("photon_match.p.fP.fBits", photon_match_p_fP_fBits, &b_photon_match_p_fP_fBits);
   fChain->SetBranchAddress("photon_match.p.fP.fX", photon_match_p_fP_fX, &b_photon_match_p_fP_fX);
   fChain->SetBranchAddress("photon_match.p.fP.fY", photon_match_p_fP_fY, &b_photon_match_p_fP_fY);
   fChain->SetBranchAddress("photon_match.p.fP.fZ", photon_match_p_fP_fZ, &b_photon_match_p_fP_fZ);
   fChain->SetBranchAddress("photon_match.p.fE", photon_match_p_fE, &b_photon_match_p_fE);
   fChain->SetBranchAddress("photon_match.x.fUniqueID", photon_match_x_fUniqueID, &b_photon_match_x_fUniqueID);
   fChain->SetBranchAddress("photon_match.x.fBits", photon_match_x_fBits, &b_photon_match_x_fBits);
   fChain->SetBranchAddress("photon_match.x.fX", photon_match_x_fX, &b_photon_match_x_fX);
   fChain->SetBranchAddress("photon_match.x.fY", photon_match_x_fY, &b_photon_match_x_fY);
   fChain->SetBranchAddress("photon_match.x.fZ", photon_match_x_fZ, &b_photon_match_x_fZ);
   fChain->SetBranchAddress("photon_match.is_fiducial", photon_match_is_fiducial, &b_photon_match_is_fiducial);
   fChain->SetBranchAddress("target.fUniqueID", &target_fUniqueID, &b_R_target_fUniqueID);
   fChain->SetBranchAddress("target.fBits", &target_fBits, &b_R_target_fBits);
   fChain->SetBranchAddress("target.fP.fUniqueID", &target_fP_fUniqueID, &b_R_target_fP_fUniqueID);
   fChain->SetBranchAddress("target.fP.fBits", &target_fP_fBits, &b_R_target_fP_fBits);
   fChain->SetBranchAddress("target.fP.fX", &target_fP_fX, &b_R_target_fP_fX);
   fChain->SetBranchAddress("target.fP.fY", &target_fP_fY, &b_R_target_fP_fY);
   fChain->SetBranchAddress("target.fP.fZ", &target_fP_fZ, &b_R_target_fP_fZ);
   fChain->SetBranchAddress("target.fE", &target_fE, &b_R_target_fE);
   fChain->SetBranchAddress("beam.fUniqueID", &beam_fUniqueID, &b_R_beam_fUniqueID);
   fChain->SetBranchAddress("beam.fBits", &beam_fBits, &b_R_beam_fBits);
   fChain->SetBranchAddress("beam.fP.fUniqueID", &beam_fP_fUniqueID, &b_R_beam_fP_fUniqueID);
   fChain->SetBranchAddress("beam.fP.fBits", &beam_fP_fBits, &b_R_beam_fP_fBits);
   fChain->SetBranchAddress("beam.fP.fX", &beam_fP_fX, &b_R_beam_fP_fX);
   fChain->SetBranchAddress("beam.fP.fY", &beam_fP_fY, &b_R_beam_fP_fY);
   fChain->SetBranchAddress("beam.fP.fZ", &beam_fP_fZ, &b_R_beam_fP_fZ);
   fChain->SetBranchAddress("beam.fE", &beam_fE, &b_R_beam_fE);
   fChain->SetBranchAddress("vertex.fUniqueID", &vertex_fUniqueID, &b_R_vertex_fUniqueID);
   fChain->SetBranchAddress("vertex.fBits", &vertex_fBits, &b_R_vertex_fBits);
   fChain->SetBranchAddress("vertex.fX", &vertex_fX, &b_R_vertex_fX);
   fChain->SetBranchAddress("vertex.fY", &vertex_fY, &b_R_vertex_fY);
   fChain->SetBranchAddress("vertex.fZ", &vertex_fZ, &b_R_vertex_fZ);
   fChain->SetBranchAddress("W.fUniqueID", &W_fUniqueID, &b_R_W_fUniqueID);
   fChain->SetBranchAddress("W.fBits", &W_fBits, &b_R_W_fBits);
   fChain->SetBranchAddress("W.fP.fUniqueID", &W_fP_fUniqueID, &b_R_W_fP_fUniqueID);
   fChain->SetBranchAddress("W.fP.fBits", &W_fP_fBits, &b_R_W_fP_fBits);
   fChain->SetBranchAddress("W.fP.fX", &W_fP_fX, &b_R_W_fP_fX);
   fChain->SetBranchAddress("W.fP.fY", &W_fP_fY, &b_R_W_fP_fY);
   fChain->SetBranchAddress("W.fP.fZ", &W_fP_fZ, &b_R_W_fP_fZ);
   fChain->SetBranchAddress("W.fE", &W_fE, &b_R_W_fE);
   fChain->SetBranchAddress("all_fiducial", &all_fiducial, &b_R_all_fiducial);
   fChain->SetBranchAddress("all_mesons_fiducial", &all_mesons_fiducial, &b_R_all_mesons_fiducial);
   fChain->SetBranchAddress("all_photons_fiducial", &all_photons_fiducial, &b_R_all_photons_fiducial);
   fChain->SetBranchAddress("all_protons_fiducial", &all_protons_fiducial, &b_R_all_protons_fiducial);
}

Bool_t b1_pi::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef b1_pi_cxx
