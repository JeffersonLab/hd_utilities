// $Id$
//
//    File: JEventProcessor_FCAL_Pi0TOF.cc
// Created: Wed Aug 30 16:23:49 EDT 2017
// Creator: mstaib (on Linux egbert 2.6.32-696.10.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_FCAL_Pi0TOF.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_FCAL_Pi0TOF());
  }
} // "C"


//------------------
// JEventProcessor_FCAL_Pi0TOF (Constructor)
//------------------
JEventProcessor_FCAL_Pi0TOF::JEventProcessor_FCAL_Pi0TOF()
{
  DO_METHOD = 0;
  USE_TRACKS = 0;

  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:DO_METHOD", DO_METHOD );
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:USE_TRACKS", USE_TRACKS );

  m_time_FCALRF_cut = 3.0;
  m_time_FCALFCAL_cut = 5.0;
  m_time_RF_offset = 0;
  m_Ethres_default = 0.25;
  m_Ethres = 0.25;
  m_bit = 1;
  m_no_bit = 1;
  frac_thres_1_to_5 = 0.5;
  frac_thres_6_to_23 = 0.1;
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:time_RF_offset", m_time_RF_offset );
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:time_FCALRF_cut", m_time_FCALRF_cut );
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:time_FCALFCAL_cut", m_time_FCALFCAL_cut );
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:frac_thres_1_to_5", frac_thres_1_to_5 );
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:frac_thres_6_to_23", frac_thres_6_to_23 );
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:Ethres_default", m_Ethres_default );
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:Ethres", m_Ethres );
  for (int i = 0; i < 26; i ++) {
    m_Ethres_ring[i] = m_Ethres;
    gPARMS->SetDefaultParameter( Form("FCAL_Pi0TOF:Ethres_ring_%d", i + 1), m_Ethres_ring[i] );
    cout << "Default Ethres " << m_Ethres_default << " Ethres " << m_Ethres << " ring Ethres " << m_Ethres_ring[i] << endl;
  }
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:bit", m_bit );
  gPARMS->SetDefaultParameter( "FCAL_Pi0TOF:no_bit", m_no_bit );
  
  cout <<"DO_METHOD " << DO_METHOD << " USE_TRACKS " << USE_TRACKS << endl;
}

//------------------
// ~JEventProcessor_FCAL_Pi0TOF (Destructor)
//------------------
JEventProcessor_FCAL_Pi0TOF::~JEventProcessor_FCAL_Pi0TOF()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::init(void)
{
  // This is called once at program startup. 

  gDirectory->mkdir("FCAL_Pi0");
  gDirectory->cd("FCAL_Pi0");
  hCurrentGainConstants = new TProfile("CurrentGainConstants", "Current Gain Constants", 2800, -0.5, 2799.5);
  hCurrentBLKQualities = new TProfile("CurrentBLKQualities", "Current Block Qualities", 2800, -0.5, 2799.5);
  gDirectory->cd("..");

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes

  // Put the current gain constants into the output file
  vector< double > raw_gains;
  // This is called whenever the run number changes
  eventLoop->GetCalib("/FCAL/gains", raw_gains);
  for (unsigned int i=0; i<raw_gains.size(); i++){
    hCurrentGainConstants->Fill(i,raw_gains[i]);
  }

  vector< int > raw_qualities;
  // This is called whenever the run number changes
  eventLoop->GetCalib("/FCAL/block_quality", raw_qualities);
  for (unsigned int i=0; i<raw_qualities.size(); i++){
    hCurrentBLKQualities->Fill(i,raw_qualities[i]);
  }

  DGeometry* dgeom = NULL;
  DApplication* dapp = dynamic_cast< DApplication* >(eventLoop->GetJApplication());
  if (dapp) dgeom = dapp->GetDGeometry(runnumber);
  if (dgeom) {
    dgeom->GetTargetZ(m_targetZ);
  } else {
    cerr << "No geometry accessbile to ccal_timing monitoring plugin." << endl;
    return RESOURCE_UNAVAILABLE;
  }	
  jana::JCalibration *jcalib = japp->GetJCalibration(runnumber);
  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  m_beamSpotX = beam_spot.at("x");
  m_beamSpotY = beam_spot.at("y");
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
  // Here's an example:
  //
  // vector<const MyDataClass*> mydataclasses;
  // loop->Get(mydataclasses);
  //
  // japp->RootFillLock(this);
  //  ... fill historgrams or trees ...
  // japp->RootFillUnLock(this);
  vector<const DFCALGeometry*> fcalGeomVect;
  loop->Get( fcalGeomVect );
  if (fcalGeomVect.size() < 1)
    return OBJECT_NOT_AVAILABLE;
  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
  
  vector<const DNeutralParticle *> neutralParticleVector;
  loop->Get(neutralParticleVector);
  
  vector<const DTOFPoint*> tof_points;
  loop->Get(tof_points);
  
  vector<const DVertex*> kinfitVertex;
  loop->Get(kinfitVertex);

  vector<const DL1Trigger *> locL1Triggers;
  vector<const DBeamPhoton *> locBeamPhotons;  
  loop->Get(locL1Triggers);
  loop->Get(locBeamPhotons);
  /*
  const DEventRFBunch * locEventRFBunches = NULL;                                                                                                                                                               try {                                                                                                                                                                                               
    loop->GetSingle(locEventRFBunches, "CalorimeterOnly" );                                                                                                                                      
  } catch (...) { 
    return NOERROR; 
  }                                                                                                                                                                   
  double locRFTime = locEventRFBunches->dTime;
  if(locEventRFBunches->dNumParticleVotes < 3 ) 
    return NOERROR;    
  */
  vector<const DEventRFBunch*> locEventRFBunches;
  loop->Get(locEventRFBunches);
  double locRFTime = locEventRFBunches.empty() ? 0.0 : locEventRFBunches[0]->dTime;
  
  uint32_t locL1Trigger_fp = locL1Triggers.empty() ? 0.0 : locL1Triggers[0]->fp_trig_mask;
  uint32_t locL1Trigger = locL1Triggers.empty() ? 0.0 : locL1Triggers[0]->trig_mask;
  
  int trig_bit[33];
  if (locL1Triggers.size() > 0) {
    for (unsigned int bit = 0; bit < 32; bit ++) {
      trig_bit[bit + 1] = (locL1Triggers[0]->trig_mask & (1 << bit)) ? 1 : 0;
      if(trig_bit[bit + 1] == 1) //htrig_bit->Fill(Float_t(bit+1));
	Fill1DHistogram("FCAL_Pi0HFA","","trig_bit",
			Float_t(bit+1),
			";Trigger bit #;Count [a.u.]",
			100, 0., 100.);
      
    }
  }
  /*
    Bit 0: CCAL & FCAL (prescaled)
    Bit 1: FCAL (threshold about 3.5 GeV)
    Bit 2: FCAL (threshold 0.5 GeV)
    Bit 3: PS
    Bit 10: CCAL
  */
  
  if (trig_bit[2] == m_bit || m_no_bit == 0) {

    for (unsigned int bit = 0; bit < 32; bit ++) {
      if(trig_bit[bit + 1] == 1) //htrig_bit->Fill(Float_t(bit+1));
	Fill1DHistogram("FCAL_Pi0HFA","","trig_bit_select",
			Float_t(bit+1),
			";Trigger bit #;Count [a.u.]",
			100, 0., 100.);
      
    }
    
    
  DVector3 vertex;
  vertex.SetXYZ(m_beamSpotX, m_beamSpotY, m_targetZ);
  
  //Use kinfit when available
  double kinfitVertexX = m_beamSpotX;
  double kinfitVertexY = m_beamSpotY;
  double kinfitVertexZ = m_targetZ;
  
  vector< const JObject* > locObjectsToSave;
  if (USE_TRACKS == 0) {
    for (unsigned int i = 0 ; i < kinfitVertex.size(); i++) {
      kinfitVertexX = kinfitVertex[i]->dSpacetimeVertex.X();
      kinfitVertexY = kinfitVertex[i]->dSpacetimeVertex.Y();
      kinfitVertexZ = kinfitVertex[i]->dSpacetimeVertex.Z();
    }
  }
  
  bool in_target = 50 < kinfitVertexZ && kinfitVertexZ < 80;

  double Esum = 0;
  for (unsigned int i = 0; i < neutralParticleVector.size(); i++){
    const DNeutralParticleHypothesis *photon1 = neutralParticleVector[i]->Get_Hypothesis(Gamma);
    const DNeutralShower *shower1 = photon1->Get_NeutralShower();
    if(shower1->dDetectorSystem != SYS_FCAL) continue;
    Esum += photon1->lorentzMomentum().E();
  }
  for (unsigned int i = 0; i < neutralParticleVector.size(); i++){
    const DNeutralParticleHypothesis *photon1 = neutralParticleVector[i]->Get_Hypothesis(Gamma);
    //bool bo_pho1 = true;
    //if( photon1 == nullptr && (USE_TRACKS == 0 || USE_TRACKS == 2)) bo_pho1 = false;
    if( photon1 == nullptr && (USE_TRACKS == 0 || USE_TRACKS == 2)) continue;
    // Go into the FCAL shower and find the largest energy deposition
    const DNeutralShower *shower1 = photon1->Get_NeutralShower();
    if(shower1->dDetectorSystem != SYS_FCAL) continue;
    float TOF_FCAL_x1_min = shower1->dTOF_FCAL_x_min;
    float TOF_FCAL_y1_min = shower1->dTOF_FCAL_y_min;
    DFCALShower *fcalShower1 = (DFCALShower *) shower1->dBCALFCALShower;
    const DFCALCluster *fcalCluster1;
    fcalShower1->GetSingle(fcalCluster1);
    DVector3  posInCal1 = fcalCluster1->getCentroid();
    float xc1 = posInCal1.Px();
    float yc1 = posInCal1.Py();
    double radiusc1 = sqrt(xc1 * xc1 + yc1 * yc1);;
    int ring1_nb = (int) (radiusc1 / (5 * k_cm));

    int ch1 = fcalCluster1->getChannelEmax();
    double emax1 = fcalCluster1->getEmax();
    double xShower1=fcalShower1->getPosition().X();
    double yShower1=fcalShower1->getPosition().Y();
    double zShower1=fcalShower1->getPosition().Z();
    double xShowerlog1=fcalShower1->getPosition_log().X();
    double yShowerlog1=fcalShower1->getPosition_log().Y();
    int row = fcalGeom.row(static_cast<float>(yShowerlog1));
    int col = fcalGeom.column(static_cast<float>(xShowerlog1));
    double zShowerlog1=fcalShower1->getPosition_log().Z();
    double x1 = xShower1 - kinfitVertexX;
    double y1 = yShower1 - kinfitVertexY;
    double z1 = zShower1 - kinfitVertexZ;
    double xl1 = xShowerlog1 - kinfitVertexX;
    double yl1 = yShowerlog1 - kinfitVertexY;
    double zl1 = zShowerlog1 - kinfitVertexZ;
    double radiusShower1=sqrt(pow(xShower1,2)+pow(yShower1,2));
    double radiusShowerlog1=sqrt(pow(xShowerlog1,2)+pow(yShowerlog1,2));
    double radius1 = fcalGeom.positionOnFace(ch1).Mod();
    int ring1 = ring1_nb;//(int) (radius1 / (5 * k_cm));
    int rings1 = ring1_nb;//(int) (radiusShower1 / (5 * k_cm));
    int ringl1 = ring1_nb;//(int) (radiusShowerlog1 / (5 * k_cm));
    //ring1 = ringl1;
    //rings1 = ring1;
    //ringl1 = ring1;
    double frac_ring_thres = frac_thres_1_to_5;
    if (ring1 >= 6)
      frac_ring_thres = frac_thres_6_to_23;
    double frac_rings_thres = frac_thres_1_to_5;
    if (rings1 >= 6)
      frac_rings_thres = frac_thres_6_to_23;
    double frac_ringl_thres = frac_thres_1_to_5;
    if (ringl1 >= 6)
      frac_ringl_thres = frac_thres_6_to_23;
    
    double frac1 = fcalCluster1->getEmax()/fcalCluster1->getEnergy();
    double Eclust1 = fcalCluster1->getEnergy();
    double Ephoton1 = photon1->lorentzMomentum().E();
    DVector3 vertex1(xl1, yl1, zl1);
    double r1 = vertex1.Mag();
    double t1 = fcalShower1->getTime() - (r1 / TMath::C() * 1e7);
    double p1 = Ephoton1 ;
    double p1x = p1 * sin(vertex1.Theta()) * cos(vertex1.Phi());
    double p1y = p1 * sin(vertex1.Theta()) * sin(vertex1.Phi());
    double p1z = p1 * cos(vertex1.Theta());
    TLorentzVector photon1P4(p1x, p1y, p1z, p1);
    double tdiff1 = t1 - locRFTime;
    
    Fill2DHistogram("XY","","xy_all", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
    Fill2DHistogram("CR","","xy_all", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    if (emax1 > 1.0) {
      Fill2DHistogram("XY","","xy_1GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_1GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 2.0) {
      Fill2DHistogram("XY","","xy_2GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_2GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 3.0) {
      Fill2DHistogram("XY","","xy_3GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_3GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 4.0) {
      Fill2DHistogram("XY","","xy_4GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_4GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }    
    if (emax1 > 5.0) {
      Fill2DHistogram("XY","","xy_5GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_5GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 6.0) {
      Fill2DHistogram("XY","","xy_6GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_6GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }    
    if (emax1 > 7.0) {
      Fill2DHistogram("XY","","xy_7GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_7GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 8.0) {
      Fill2DHistogram("XY","","xy_8GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_8GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 9.0) {
      Fill2DHistogram("XY","","xy_9GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_9GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 10.0) {
      Fill2DHistogram("XY","","xy_10GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_10GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    Fill1DHistogram("FCAL_Pi0HFA","","tdiff1", tdiff1, ";t_{#gamma}^{1} - t_{RF} [ns];Count [a.u.]", 1000, -99., 99.);
    Fill1DHistogram("FCAL_Pi0HFA","","E", Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","","Emax", emax1, ";E_{max};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("Emax_ring_%d", ring1), emax1, ";E_{max};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","","P", Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("E_ring_%d", ring1), Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("P_ring_%d", ring1), Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("E_rings_%d", rings1), Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("P_rings_%d", rings1), Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("E_ringl_%d", ringl1), Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("P_ringl_%d", ringl1), Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    if(radiusShower1 < 108.4239 && radiusShower1 > 20.785){
      Fill1DHistogram("FCAL_Pi0HFA","","E_cut", Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
      Fill1DHistogram("FCAL_Pi0HFA","","P_cut", Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    }
    if(radiusShowerlog1 < 108.4239 && radiusShowerlog1 > 20.785){
      Fill1DHistogram("FCAL_Pi0HFA","","E_cutlog", Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
      Fill1DHistogram("FCAL_Pi0HFA","","P_cutlog", Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    }
    int tof_match1 = 0;
    int ltof_match1 = 0;
    if (TOF_FCAL_x1_min < 6 && TOF_FCAL_y1_min < 6) {
      tof_match1 = 1;
      ltof_match1 = 1;
    }
    /*
    int tof_match1 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x1, y1, z1);
    int ltof_match1 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x1, y1, z1);
    for (vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); tof_p != tof_points.end(); tof_p++ ) {
      
      double xtof = (*tof_p)->pos.X() - kinfitVertexX;
      double ytof = (*tof_p)->pos.Y() - kinfitVertexY;
      double ztof = (*tof_p)->pos.Z() - kinfitVertexZ;
      xtof = xtof * (z1 / ztof);
      ytof = ytof * (z1 / ztof);
      double dx = xl1 - xtof;
      double dy = yl1 - ytof;
      
      if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
	//if( dt > -1. && dt < 3. ) tof_match = 1;
	tof_match1 = 1;
	ltof_match1 = 1;
      }
    } // end DTOFPoint loop
    */
    
  
    for (unsigned int j = i + 1; j < neutralParticleVector.size(); j ++){
      const DNeutralParticleHypothesis *photon2 = neutralParticleVector[j]->Get_Hypothesis(Gamma);
      //bool bo_pho2 = true;
      //if (photon2 == nullptr && (USE_TRACKS == 0 || USE_TRACKS == 2)) bo_pho2 = false;
      if (photon2 == nullptr && (USE_TRACKS == 0 || USE_TRACKS == 2)) continue;
      const DNeutralShower *shower2 = photon2->Get_NeutralShower();
      if(shower2->dDetectorSystem != SYS_FCAL) continue;
      float TOF_FCAL_x2_min = shower2->dTOF_FCAL_x_min;
      float TOF_FCAL_y2_min = shower2->dTOF_FCAL_y_min;
      DFCALShower *fcalShower2 = (DFCALShower *) shower2->dBCALFCALShower;
      const DFCALCluster *fcalCluster2;
      fcalShower2->GetSingle(fcalCluster2);
      DVector3  posInCal2 = fcalCluster2->getCentroid();
      float xc2 = posInCal2.Px();
      float yc2 = posInCal2.Py();
      double radiusc2 = sqrt(xc2 * xc2 + yc2 * yc2);;
      int ring2_nb = (int) (radiusc2 / (5 * k_cm));
      
      int ch2 = fcalCluster2->getChannelEmax();
      double emax2 = fcalCluster2->getEmax();
      double xShower2=fcalShower2->getPosition().X();
      double yShower2=fcalShower2->getPosition().Y();
      double zShower2=fcalShower2->getPosition().Z();
      double xShowerlog2=fcalShower2->getPosition_log().X();
      double yShowerlog2=fcalShower2->getPosition_log().Y();
      double zShowerlog2=fcalShower2->getPosition_log().Z();
      double x2 = xShower2 - kinfitVertexX;
      double y2 = yShower2 - kinfitVertexY;
      double z2 = zShower2 - kinfitVertexZ;
      double xl2 = xShowerlog2 - kinfitVertexX;
      double yl2 = yShowerlog2 - kinfitVertexY;
      double zl2 = zShowerlog2 - kinfitVertexZ;
      double radiusShower2=sqrt(pow(xShower2,2)+pow(yShower2,2));
      double radiusShowerlog2=sqrt(pow(xShowerlog2,2)+pow(yShowerlog2,2));
      double radius2 = fcalGeom.positionOnFace(ch2).Mod();
      int ring2 = ring2_nb;//(int) (radius2 / (5 * k_cm));
      int rings2 = ring2_nb;//(int) (radiusShower2 / (5 * k_cm));
      int ringl2 = ring2_nb;//(int) (radiusShowerlog2 / (5 * k_cm));
      //ring2=ringl2;
      //rings2 = ring2;
      //ringl2 = ring2;
      double frac_ring_thres = frac_thres_1_to_5;
      if (ring2 >= 6)
	frac_ring_thres = frac_thres_6_to_23;
      double frac_rings_thres = frac_thres_1_to_5;
      if (rings2 >= 6)
	frac_rings_thres = frac_thres_6_to_23;
      double frac_ringl_thres = frac_thres_1_to_5;
      if (ringl2 >= 6)
	frac_ringl_thres = frac_thres_6_to_23;

      double frac2 = fcalCluster2->getEmax()/fcalCluster2->getEnergy();
      double Eclust2 = fcalCluster2->getEnergy();
      double Ephoton2 = photon2->lorentzMomentum().E();
      DVector3 vertex2(xl2, yl2, zl2);
      double r2 = vertex2.Mag();
      double t2 = fcalShower2->getTime() - (r2 / TMath::C() * 1e7);
      double p2 = Ephoton2;
      double p2x = p2 * sin(vertex2.Theta()) * cos(vertex2.Phi());
      double p2y = p2 * sin(vertex2.Theta()) * sin(vertex2.Phi());
      double p2z = p2 * cos(vertex2.Theta());
      TLorentzVector photon2P4(p2x, p2y, p2z, p2);
      double tdiff2 = t2 - locRFTime;
      
      Fill1DHistogram("FCAL_Pi0HFA","","tdiff2", tdiff2, ";t_{#gamma}^{2} - t_{RF} [ns];Count [a.u.]", 1000, -99., 99.);
      Fill1DHistogram("FCAL_Pi0HFA","","tdifft", t2 - t1, ";t_{#gamma}^{2} - t_{#gamma}^{1} [ns];Count [a.u.]", 1000, -99., 99.);
      Fill1DHistogram("FCAL_Pi0HFA","","tdifft", t1 - t2, ";t_{#gamma}^{2} - t_{#gamma}^{1} [ns];Count [a.u.]", 1000, -99., 99.);

      Fill2DHistogram("FCAL_Pi0HFA","","trfvsch", tdiff1, ch1, ";t_{#gamma}^{2} - t_{RF} [ns];Count [a.u.]", 1000, -99., 99., 2800, -0.5, 2799.5);
      Fill2DHistogram("FCAL_Pi0HFA","","trfvsch", tdiff2, ch2, ";t_{#gamma}^{2} - t_{RF} [ns];Count [a.u.]", 1000, -99., 99., 2800, -0.5, 2799.5);
      Fill2DHistogram("FCAL_Pi0HFA","","ttvsch", t2 - t1, ch2, ";t_{#gamma}^{2} - t_{#gamma}^{1} [ns];Count [a.u.]", 1000, -99., 99., 2800, -0.5, 2799.5);
      Fill2DHistogram("FCAL_Pi0HFA","","ttvsch", t1 - t2, ch1, ";t_{#gamma}^{2} - t_{#gamma}^{1} [ns];Count [a.u.]", 1000, -99., 99., 2800, -0.5, 2799.5);

      Fill1DHistogram("FCAL_Pi0HFA","",Form("Emax_ring_%d", ring2), emax2, ";E_{max};Count [a.u.]", 12000, 0., 12.);
      Fill1DHistogram("FCAL_Pi0HFA","","Emax", emax2, ";E_{max};Count [a.u.]", 12000, 0., 12.);
      double Eratio = Eclust1 / Eclust2;
      bool bo_5p = false;
      bool bo_10p = false;
      bool bo_20p = false;
      bool bo_30p = false;
      if (0.95 <= Eratio && Eratio <= 1.05) bo_5p = true;
      if (0.90 <= Eratio && Eratio <= 1.10) bo_10p = true;
      if (0.80 <= Eratio && Eratio <= 1.20) bo_20p = true;
      if (0.70 <= Eratio && Eratio <= 1.30) bo_30p = true;
      double pi0Mass = (photon1->lorentzMomentum() + photon2->lorentzMomentum()).M();
      double pi0Masslog = (photon1P4 + photon2P4).M();
      pi0Mass=pi0Masslog;
      double theta_pi0 = (photon1->lorentzMomentum() + photon2->lorentzMomentum()).Theta() * TMath::RadToDeg();
      double pi0P = (photon1->lorentzMomentum() + photon2->lorentzMomentum()).P();
      double theta_pi0log = (photon1P4 + photon2P4).Theta() * TMath::RadToDeg();
      theta_pi0=theta_pi0log;
      double pi0E = (photon1P4 + photon2P4).E();
      double pi0Plog = (photon1P4 + photon2P4).P();
      pi0P=pi0Plog;
      double avgE = 0.5*fcalCluster1->getEnergy() + 0.5*fcalCluster2->getEnergy();
      DVector3 distance = vertex1 - vertex2;
      double d = distance.Mag();
      bool api0 = false;
      bool aeta = false;
      if (0.11 <= pi0Masslog && pi0Masslog <= 0.16) api0 = true;
      if (0.51 <= pi0Masslog && pi0Masslog <= 0.58) aeta = true;
      int tof_match2 = 0;
      int ltof_match2 = 0;
      if (TOF_FCAL_x2_min < 6 && TOF_FCAL_y2_min < 6) {
	tof_match2 = 1;
	ltof_match2 = 1;
      }      
      /*
      int tof_match2 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x2, y2, z2);
      int ltof_match2 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x2, y2, z2);
      for (vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); tof_p != tof_points.end(); tof_p++ ) {
	
	double xtof = (*tof_p)->pos.X() - kinfitVertexX;
	double ytof = (*tof_p)->pos.Y() - kinfitVertexY;
	double ztof = (*tof_p)->pos.Z() - kinfitVertexZ;
	xtof = xtof * (z2 / ztof);
	ytof = ytof * (z2 / ztof);
	double dx = xl2 - xtof;
	double dy = yl2 - ytof;
	
	if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
	  tof_match2 = 1;
	  ltof_match2 = 1;
	}
      } // end DTOFPoint loop
      */
      if (((tof_match1 == 0 && tof_match2 == 0) || (ltof_match1 == 0 && ltof_match2 == 0)) && 
	  (fabs(tdiff1) < m_time_FCALRF_cut) && (fabs(tdiff2) < m_time_FCALRF_cut) && 
	  (fabs(t1-t2) < m_time_FCALFCAL_cut) && Ephoton1 > m_Ethres_default && Ephoton2 > m_Ethres_default && frac1 > frac_ring_thres && frac2 > frac_ring_thres) {
	
	//if (bo_pho1 && bo_pho2) {
	if (DO_METHOD == 0) {
	  Fill1DHistogram("FCAL_Pi0HFA","","Pi0Mass",
			  pi0Mass,
			  "#pi^{0} Mass; #pi^{0} Mass;",
			  500, 0.05, 0.7);
	  Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass_ring_%d", ring1),
			  pi0Mass,
			  "#pi^{0} Mass; #pi^{0} Mass;",
			  500, 0.05, 0.7);
	  Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass_ring_%d", ring2),
			  pi0Mass,
			  "#pi^{0} Mass; #pi^{0} Mass;",
			  500, 0.05, 0.7);
	  if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) {
	    Fill1DHistogram("FCAL_Pi0HFA","","Pi0Mass_thres",
			  pi0Mass,
			  "#pi^{0} Mass; #pi^{0} Mass;",
			  500, 0.05, 0.7);
	    Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass_ring_thres_%d", ring1),
			    pi0Mass,
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);
	    Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass_ring_thres_%d", ring2),
			    pi0Mass,
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);
	  }
	  if (Ephoton1 > m_Ethres_ring[ring1] && Ephoton2 > m_Ethres_ring[ring2]) {
	    Fill1DHistogram("FCAL_Pi0HFA","","Pi0Mass_thres",
			  pi0Mass,
			  "#pi^{0} Mass; #pi^{0} Mass;",
			  500, 0.05, 0.7);
	    Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass_ring_thres_rg_%d", ring1),
			    pi0Mass,
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);
	    Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass_ring_thres_rg_%d", ring2),
			    pi0Mass,
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);

	  }
	  if (neutralParticleVector.size() == 2) {
	    Fill1DHistogram("FCAL_Pi0HFA","","Pi0Mass2g",
			    pi0Mass, 
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);
	    Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2g_ring_%d", ring1),
			    pi0Mass, 
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);
	    Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2g_ring_%d", ring2),
			    pi0Mass, 
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);
	  }
	  
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum",
			  ch1, pi0Mass,
			  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			  2800, -0.5, 2799.5, 500, 0.05, 0.7);
	  
	  if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) {
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_thres",
			    ch1, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 700, 0.0, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_thres",
			    ch2, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 700, 0.0, 0.7);
	  }
	  
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum",
			  ch2, pi0Mass,
			  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			  2800, -0.5, 2799.5, 500, 0.05, 0.7);


	  
	  Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_ring_%d", ring1),
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 500, 0.05, 0.7);
	  Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_ring_%d", ring2),
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 500, 0.05, 0.7);
	  if (bo_5p){
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_5_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_5_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	  }
	  if (bo_10p){
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_10_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_10_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	  }
	  if (bo_20p){
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_20_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_20_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	  }
	  if (bo_30p){
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_30_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_30_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	  }
	  if (neutralParticleVector.size() == 2) {
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    if (bo_5p){
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_5_ring_%d", ring1),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_5_ring_%d", ring2),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	    }
	    if (bo_10p){
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_10_ring_%d", ring1),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_10_ring_%d", ring2),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	    }
	    if (bo_20p){
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_20_ring_%d", ring1),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_20_ring_%d", ring2),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	    }
	    if (bo_30p){
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_30_ring_%d", ring1),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_30_ring_%d", ring2),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	    } 
	  }
	}
	if (DO_METHOD == 1 || DO_METHOD == 2) {
	  for (unsigned int k = 0; k < (int) locBeamPhotons.size(); k ++) {
	    
	    const DBeamPhoton *ebeam = locBeamPhotons[k]; 
	    //double eb = ebeam->lorentzMomentum().E();
	    double tb = ebeam->time();
	    double zb = ebeam->position().Z();
	    double eb = ebeam->lorentzMomentum().E();
	    double locDeltaTRF = tb - (locRFTime + (zb - m_targetZ) / 29.9792458) + m_time_RF_offset;
	    Fill1DHistogram("FCAL_Pi0TOF","","TaggerTiming1", locDeltaTRF, ";t_{e^{-}} - t_{#gamma} [ns];Count [a.u.]", 500, -100., 100.);
	    //Fill2DWeightedHistogram("FCAL_Pi0TOF","","PhotonTiming1", t1 - t2, pi0Masslog, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100.);
	    //Fill2DWeightedHistogram("FCAL_Pi0TOF","","PhotonTiming1", t2 - t1, pi0Masslog, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100.);
	    double weight = 0;
	    if (fabs(locDeltaTRF) <= 2.004) {
	      weight = 1;
	    } else if ( ( -(2.004 + 3.0 * 4.008) <= locDeltaTRF && locDeltaTRF <= -(2.004 + 4.008) ) || 
			( (2.004 + 4.008) <= locDeltaTRF && locDeltaTRF <= (2.004 + 3.0 * 4.008) ) ) {
	      weight = -0.25;
	    } else {
	      continue;
	    }
	    double dE = eb - pi0E;

	    if (ring1 == 1) if (eb < 8.0) continue;
	    if (ring2 == 1) if (eb < 8.0) continue;

	    if (DO_METHOD == 1) {
	      if (tof_match1 == 0 && tof_match2 == 0) {
		
		Fill1DHistogram("FCAL_Pi0TOF","","TaggerTiming2", locDeltaTRF, ";t_{e^{-}} - t_{#gamma} [ns];Count [a.u.]", 500, -100., 100.);
		Fill2DWeightedHistogram("FCAL_Pi0TOF","","PhotonTiming1", t1 - t2, pi0Mass, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100., 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0TOF","","PhotonTiming1", t2 - t1, pi0Mass, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100., 500, 0.05, 0.7);
		
		Fill1DWeightedHistogram("FCAL_Pi0TOF","","Pi0Mass",
					pi0Mass, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		
		Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_ring_%d", ring1),
					pi0Mass, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_ring_%d", ring2),
					pi0Mass, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);		
		if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) {
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","","Pi0Mass_thres",
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_ring_thres_%d", ring1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_ring_thres_%d", ring2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		}
		if (Ephoton1 > m_Ethres_ring[ring1] && Ephoton2 > m_Ethres_ring[ring2]) {
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","","Pi0Mass_thres_rg",
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_ring_thres_rg_%d", ring1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_ring_thres_rg_%d", ring2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		}
		if (neutralParticleVector.size() == 2) {
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","","Pi0Mass2g",
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_ring_%d", ring1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_ring_%d", ring2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) {
		    Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_ring_thres_%d", ring1),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  
		    Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_ring_thres_%d", ring2),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  }
		  if (Ephoton1 > m_Ethres_ring[ring1] && Ephoton2 > m_Ethres_ring[ring2]) { 
		    Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_ring_thres_rg_%d", ring1),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  
		    Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_ring_thres_rg_%d", ring2),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  }
		}	  
		
		Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum",
					ch1, pi0Mass, weight,
					"#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					2800, -0.5, 2799.5, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum",
					ch2, pi0Mass, weight,
					"#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					2800, -0.5, 2799.5, 500, 0.05, 0.7);
		if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) {
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_thres",
					  ch1, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_thres",
					  ch2, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		}
		if (Ephoton1 > m_Ethres_ring[ring1] && Ephoton2 > m_Ethres_ring[ring2]) {
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_thres_rg",
					  ch1, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_thres_rg",
					  ch2, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		}
		Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_ring_%d", ring1),
					Eclust1, pi0Mass, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		
		Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_ring_%d", ring2),
					Eclust2, pi0Mass, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) { 
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_ring_thers_%d", ring1),
					  Eclust1, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_ring_thres_%d", ring2),
					  Eclust2, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		if (api0) {
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_theta_%d", ring1),
					  pi0P, theta_pi0, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_theta_%d", ring2),
					  pi0P, theta_pi0, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_thetas_%d", rings1),
					  pi0P, theta_pi0, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_thetas_%d", rings2),
					  pi0P, theta_pi0, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		}

		if (neutralParticleVector.size() == 2) {
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_ring_%d", ring1),
					  Eclust1, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_ring_%d", ring2),
					  Eclust2, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) { 
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_rings_%d", ring1),
					    Eclust1, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_rings_%d", ring2),
					    Eclust2, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		}
		
		if (bo_5p){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_ring_%d", ring1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_ring_%d", ring2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_theta_%d", ring1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_theta_%d", ring2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_5_ring_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_5_ring_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_5",
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		if (bo_10p){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_ring_%d", ring1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_ring_%d", ring2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) { 
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_ring_thres_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_ring_thres_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_theta_%d", ring1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_theta_%d", ring2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_10_ring_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_10_ring_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_10",
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		if (bo_20p){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_ring_%d", ring1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_ring_%d", ring2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) { 
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_ring_thres_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_ring_thres_%d", rings2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_theta_%d", ring1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_theta_%d", ring2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_20_ring_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_20_ring_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_20",
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		if (bo_30p){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_ring_%d", ring1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_ring_%d", ring2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) { 
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_ring_thres_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_ring_thres_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_theta_%d", ring1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_theta_%d", ring2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_30_ring_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_30_ring_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);		    
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_30",
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
	      }
	    }
	    if (DO_METHOD == 2) {
	      if (ltof_match1 == 0 && ltof_match2 == 0) {
		
		Fill1DHistogram("FCAL_Pi0log","","TaggerTiming1", locDeltaTRF, ";t_{e^{-}} - t_{#gamma} [ns];Count [a.u.]", 500, -100., 100.);
		Fill2DWeightedHistogram("FCAL_Pi0log","","PhotonTiming1", t1 - t2, pi0Masslog, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100., 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0log","","PhotonTiming1", t2 - t1, pi0Masslog, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100., 500, 0.05, 0.7);
		
		if (api0 && neutralParticleVector.size() == 2 && eb > 8.0) {
		  Fill1DWeightedHistogram("FCAL_Pi0log","","DeltaE",
					  dE, weight,
					  ";E_{#gamma} - E_{#pi^{0}} [GeV];Count [a.u.]",
					  1200, -2., 10.);
		  if (ring1 > 1 && ring2 > 1) {
		    Fill1DWeightedHistogram("FCAL_Pi0log","","DeltaE_ring",
					    dE, weight,
					    ";E_{#gamma} - E_{#pi^{0}} [GeV];Count [a.u.]",
					    1200, -2., 10.);
		  }
		}
		
		if (api0)
		  Fill2DWeightedHistogram("FCAL_Pi0log","","DistanceBetweenPi0Clusters",
					  ring1, d, weight,
					  ";ring #;Distance between two clusters [cm];Count [a.u.]",
					  30, 0, 30, 500, 0., 100.);
		if (aeta)
		  Fill2DWeightedHistogram("FCAL_Pi0log","","DistanceBetweenPi0Clusters",
					  ring1, d, weight,
					  ";ring #;Distance between two clusters [cm];Count [a.u.]",
					  30, 0, 30, 500, 0., 100.);
		
				
		Fill1DWeightedHistogram("FCAL_Pi0log","","Pi0Mass",
					pi0Masslog, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ring_%d", ring1),
					pi0Masslog, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ring_%d", ring2),
					pi0Masslog, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) {
		  Fill1DWeightedHistogram("FCAL_Pi0log","","Pi0Mass_thres",
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ring_thres_%d", ring1),
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ring_thres_%d", ring2),
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		}
		if (Ephoton1 > m_Ethres_ring[ring1] && Ephoton2 > m_Ethres_ring[ring2]) {
		  Fill1DWeightedHistogram("FCAL_Pi0log","","Pi0Mass_thres_rg",
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ring_thres_rg_%d", ring1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ring_thres_rg_%d", ring2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		}
		if (neutralParticleVector.size() == 2) {
		  if (api0)
		    Fill2DWeightedHistogram("FCAL_Pi0log","","DistanceBetweenPi02gClusters",
					    ring1, d, weight,
					    ";ring #;Distance between two clusters [cm];Count [a.u.]",
					    30, 0, 30, 500, 0., 100.);
		  if (aeta)
		    Fill2DWeightedHistogram("FCAL_Pi0log","","DistanceBetweenPi02gClusters",
					    ring1, d, weight,
					    ";ring #;Distance between two clusters [cm];Count [a.u.]",
					    30, 0, 30, 500, 0., 100.);
		  
		  Fill1DWeightedHistogram("FCAL_Pi0log","","Pi0Mass2g",
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ring_%d", ring1),
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ring_%d", ring2),
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) { 
		    Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ring_thres_%d", ring1),
					    pi0Masslog, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  
		    Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ring_thres_%d", ring2),
					    pi0Masslog, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  }
		  if (Ephoton1 > m_Ethres_ring[ring1] && Ephoton2 > m_Ethres_ring[ring2]) { 
		    Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ring_thres_rg_%d", ring1),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  
		    Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ring_thres_rg_%d", rings1),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  }
		}
		
		Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum",
					ch1, pi0Masslog, weight,
					"#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					2800, -0.5, 2799.5, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum",
					ch2, pi0Masslog, weight,
					"#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					2800, -0.5, 2799.5, 500, 0.05, 0.7);
		if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) {
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_thres",
					  ch1, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_thres",
					  ch2, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		}
		if (Ephoton1 > m_Ethres_ring[ring1] && Ephoton2 > m_Ethres_ring[ring2]) {
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_thres_rg",
					  ch1, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_thres_rg",
					  ch2, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		}
		Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ring_%d", ring1),
					Eclust1, pi0Masslog, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ring_%d", ring2),
					Eclust2, pi0Masslog, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);

		if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) {
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ring_thres_%d", ring1),
					  Eclust1, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ring_thres_%d", ring2),
					  Eclust2, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		if (Ephoton1 > m_Ethres_ring[ring1] && Ephoton2 > m_Ethres_ring[ring2]) {
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ring_thres_rg_%d", ring1),
					  Eclust1, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ring_thres_rg_%d", ring2),
					  Eclust2, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		
		if (api0) {
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_theta_%d", ring1),
					  pi0Plog, theta_pi0log, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_theta_%d", ring2),
					  pi0Plog, theta_pi0log, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		}
		if (neutralParticleVector.size() == 2) {
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_ring_%d", ring1),
					  Eclust1, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_ring_%d", ring2),
					  Eclust2, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (Ephoton1 > m_Ethres && Ephoton2 > m_Ethres) {
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_ring_thres_%d", ring1),
					    Eclust1, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_ring_thres_%d", ring2),
					    Eclust2, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		}
		
		if (bo_5p){
		  if (2 < ring2 && ring2 < 21)
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_ring_%d", ring1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (2 < ring1 && ring1 < 21)
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_ring_%d", ring2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    if (2 < ring2 && ring2 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_theta_%d", ring1),
					      pi0Plog, theta_pi0log, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 180, 0., 18.);
		    if (2 < ring1 && ring1 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_theta_%d", ring2),
					      pi0Plog, theta_pi0log, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    if (2 < ring2 && ring2 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_5_ring_%d", ring1),
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    if (2 < ring1 && ring1 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_5_ring_%d", ring2),
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_5",
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		if (bo_10p){
		  if (2 < ring2 && ring2 < 21)
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_ring_%d", ring1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (2 < ring1 && ring1 < 21)
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_ring_%d", ring2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    if (2 < ring2 && ring2 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_theta_%d", ring1),
					      pi0Plog, theta_pi0log, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 180, 0., 18.);
		    if (2 < ring1 && ring1 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_theta_%d", ring2),
					      pi0Plog, theta_pi0log, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    if (2 < ring2 && ring2 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_10_ring_%d", ring1),
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    if (2 < ring1 && ring1 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_10_ring_%d", ring2),
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_10",
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		if (bo_20p){
		  if (2 < ring2 && ring2 < 21)
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_ring_%d", ring1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (2 < ring1 && ring1 < 21)
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_ring_%d", ring2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    if (2 < ring2 && ring2 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_theta_%d", ring1),
					      pi0Plog, theta_pi0log, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 180, 0., 18.);
		    if (2 < ring1 && ring1 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_theta_%d", ring2),
					      pi0Plog, theta_pi0log, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    if (2 < ring2 && ring2 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_20_ring_%d", ring1),
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    if (2 < ring1 && ring1 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_20_ring_%d", ring2),
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_20",
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		if (bo_30p){
		  if (2 < ring2 && ring2 < 21)
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_ring_%d", ring1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (2 < ring1 && ring1 < 21)
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_ring_%d", ring2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    if (2 < ring2 && ring2 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_theta_%d", ring1),
					      pi0Plog, theta_pi0log, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 180, 0., 18.);
		    if (2 < ring1 && ring1 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_theta_%d", ring2),
					      pi0Plog, theta_pi0log, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    if (2 < ring2 && ring2 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_30_ring_%d", ring1),
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    if (2 < ring1 && ring1 < 21)
		      Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_30_ring_%d", ring2),
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_30",
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
	      }
	    }
	  }
	}
      }
    }
  }
  }
  
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::fini(void)
{
  // Called before program exit after event processing is finished.
  return NOERROR;
}


//--------------------------------------------
// Get TOF position from bar
//--------------------------------------------
double JEventProcessor_FCAL_Pi0TOF::bar2x(int bar) {
  
  int ic = 2*bar - 45; 
  
  double pos;
  if( ic ==  1  || ic == -1 ) pos = 3.0*(double)ic;
  else if( ic ==  3  || ic ==  5 ) pos = 1.5*(double)(ic+2);
  else if( ic == -3  || ic == -5 ) pos = 1.5*(double)(ic-2);
  else if( ic >  5 ) pos = 3.*(ic-2);
  else pos = 3.*(ic+2);
  
  double x = 1.1*pos;
  
  return x;
}
/*
int JEventProcessor_FCAL_Pi0TOF::TOF_Match(double kinVertexX, double kinVertexY, double kinVertexZ, double x, double y, double z) {
  
  //-----   Check for match between TOF and FCAL   -----//
  int tof_match = 0;
  for (vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); tof_p != tof_points.end(); tof_p++ ) {
    
    double xtof = (*tof_p)->pos.X() - kinVertexX;
    double ytof = (*tof_p)->pos.Y() - kinVertexY;
    double ztof = (*tof_p)->pos.Z() - kinVertexZ;
    
    //double rtof = sqrt(xtof*xtof + ytof*ytof + ztof*ztof );
    //double ttof = (*tof_p)->t - (rtof/TMath::C());
    
    xtof = xtof * (z / ztof);
    ytof = ytof * (z / ztof);
    
    int hbar  = (*tof_p)->dHorizontalBar;			
    int hstat = (*tof_p)->dHorizontalBarStatus;
    int vbar  = (*tof_p)->dVerticalBar;
    int vstat = (*tof_p)->dVerticalBarStatus;
    
    double dx, dy;
    if( hstat==3 && vstat==3 ) {
      dx = x - xtof;
      dy = y - ytof;
    } else if( vstat==3 ) {
      dx = x - bar2x(vbar)*(z / ztof);
      dy = y - ytof;
    } else if( hstat==3 ) {
      dx = x - xtof;
      dy = y - bar2x(hbar)*(z / ztof);
    } else {
      dx = x - bar2x(vbar)*(z / ztof);
      dy = y - bar2x(hbar)*(z / ztof);
    }
    
    if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
      //if( dt > -1. && dt < 3. ) tof_match = 1;
      tof_match = 1;
    }
  } // end DTOFPoint loop
  
  return tof_match;
}
*/



