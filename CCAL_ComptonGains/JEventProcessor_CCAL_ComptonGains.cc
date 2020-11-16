
/*-------------------------------------------------------------------------
	
	This plugin can be used to calibrate CCAL gain constants using 
	Compton Scattering events.
	
	If running on simulation, you will probably want to set the 
	'BYPASS_TRIG' parameter to 1 (default 0). This can be done at run time
	by doing (for example):
		
		hd_root -PPLUGINS=CCAL_ComptonGains -PCCAL_GAINS:BYPASS_TRIG=1 <input_file>.hddm
	

--------------------------------------------------------------------------*/


#include "JEventProcessor_CCAL_ComptonGains.h"

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
    	app->AddProcessor(new JEventProcessor_CCAL_ComptonGains());
}
} // "C"




//==========================================================
//
//   Constructor
//
//==========================================================

JEventProcessor_CCAL_ComptonGains::JEventProcessor_CCAL_ComptonGains()
{
	// Set defaults:
	
	BYPASS_TRIG = false;
	
	MIN_FCAL_ENERGY_CUT  =   0.5;
	MIN_CCAL_ENERGY_CUT  =   3.0;
	MIN_BEAM_ENERGY_CUT  =   5.0;
	
	COPL_CUT             =  20.0;
	DELTA_K_CUT          =   0.5;
	
	
	
	gPARMS->SetDefaultParameter( "CCAL_ComptonGains:BYPASS_TRIG", BYPASS_TRIG, 
			"Set to true to bypass trigger information (for simulation)" );
	
	gPARMS->SetDefaultParameter( "CCAL_ComptonGains:MIN_FCAL_ENERGY_CUT", MIN_FCAL_ENERGY_CUT,
			"Minimum energy of FCAL Shower" );
	gPARMS->SetDefaultParameter( "CCAL_ComptonGains:MIN_CCAL_ENERGY_CUT", MIN_CCAL_ENERGY_CUT,
			"Minimum energy of CCAL Shower" );
	gPARMS->SetDefaultParameter( "CCAL_ComptonGains:MIN_BEAM_ENERGY_CUT", MIN_BEAM_ENERGY_CUT,
			"Minimum beam photon energy" );
	
	gPARMS->SetDefaultParameter( "CCAL_ComptonGains:COPL_CUT", COPL_CUT,
			"Coplanarity Cut (degrees)" );
	gPARMS->SetDefaultParameter( "CCAL_ComptonGains:DELTA_K_CUT", DELTA_K_CUT );
	
}




//==========================================================
//
//   init
//
//==========================================================

jerror_t JEventProcessor_CCAL_ComptonGains::init(void)
{
  	
	
	// TOF/FCAL Matching Plots
	
	TDirectory *dir_FCAL_TOF = new TDirectoryFile( "FCAL_TOF", "FCAL_TOF" );
  	dir_FCAL_TOF->cd();
	
	h_fcal_tof_dt     = new TH1F( "fcal_tof_dt", "t_{FCAL} - t_{TOF}; [ns]", 5000, -50., 50. );
	
	h_fcal_tof_dx     = new TH1F( "fcal_tof_dx",     "x_{FCAL} - x_{TOF}; [cm]", 
		5000, -50., 50. );
	h_fcal_tof_dx_bar = new TH1F( "fcal_tof_dx_bar", "x_{FCAL} - x_{TOF}; [cm]", 
		5000, -50., 50. );
	
	h_fcal_tof_dy     = new TH1F( "fcal_tof_dy",     "y_{FCAL} - y_{TOF}; [cm]", 
		5000, -50., 50. );
	h_fcal_tof_dy_bar = new TH1F( "fcal_tof_dy_bar", "y_{FCAL} - y_{TOF}; [cm]", 
		5000, -50., 50. );
	
	dir_FCAL_TOF->cd( "../" );
	
	
	
	
	// RF Timing Histograms
	
	h_fcal_rf_dt   =  new TH1F( "fcal_rf_dt", "t_{FCAL} - t_{RF}; [ns]",   2000, -50., 50. );
	h_ccal_rf_dt   =  new TH1F( "ccal_rf_dt", "t_{CCAL} - t_{RF}; [ns]",   2000, -50., 50. );
	h_tof_rf_dt    =  new TH1F( "tof_rf_dt",  "t_{TOF} - t_{RF}; [ns]",    2000, -50., 50. );
	
	h_beam_rf_dt   =  new TH1F( "beam_rf_dt", "t_{#gamma} - t_{RF}; [ns]", 2000, -50., 50. );
	h_beam_ccal_dt =  new TH1F( "beam_ccal_dt", "t_{#gamma} - t_{CCAL}",   2000, -20., 20. );
	h_beam_fcal_dt =  new TH1F( "beam_fcal_dt", "t_{#gamma} - t_{FCAL}",   2000, -20., 20. );
	
	
	
	
	
	h_deltaT   =  new TH1F( "deltaT",   "#DeltaT; t_{FCAL} - t_{CCAL} [ns]", 2000, -20., 20. );
	h_deltaPhi =  new TH1F( "deltaPhi", "#Delta#phi; #phi_{FCAL} - #phi_{CCAL} [deg.]", 
		2000, 100., 260. );
	
	
	
	
	
	//----------   Compton Plots   ----------//
	
	
	h_deltaE     = new TH1F( "deltaE",     "#DeltaE", 4000, -4., 4. );
	h_deltaK     = new TH1F( "deltaK",     "#DeltaK", 4000, -4., 4. );
	
	
	
	
	h_CompRatio   =  new TH2F( "CompRatio",   "Compton Energy Ratio", 
		144, -0.5, 143.5, 2000, 0., 2. );
	h_CompRatio_g =  new TH2F( "CompRatio_g", "Compton Energy Ratio (Photons Only)", 
		144, -0.5, 143.5, 2000, 0., 2. );
	h_CompRatio_e =  new TH2F( "CompRatio_e", "Compton Energy Ratio (Electrons Only)", 
		144, -0.5, 143.5, 2000, 0., 2. );
	
	h_CompDiff    =  new TH2F( "CompDiff",   "Compton Energy Difference", 
		144, -0.5, 143.5, 2000, -2., 2. );
	h_CompDiff_g  =  new TH2F( "CompDiff_g", "Compton Energy Difference (Photons only)", 
		144, -0.5, 143.5, 2000, -2., 2. );
	h_CompDiff_e  =  new TH2F( "CompDiff_e", "Compton Energy Difference (Electrons only)", 
		144, -0.5, 143.5, 2000, -2., 2. );
	
	h_CompRatio->GetXaxis()->SetTitle( "CCAL id." );
	h_CompRatio->GetYaxis()->SetTitle( "E_{CCAL} / E_{Comp}" );
	h_CompRatio_g->GetXaxis()->SetTitle( "CCAL id." );
	h_CompRatio_g->GetYaxis()->SetTitle( "E_{CCAL} / E_{Comp}" );
	h_CompRatio_e->GetXaxis()->SetTitle( "CCAL id." );
	h_CompRatio_e->GetYaxis()->SetTitle( "E_{CCAL} / E_{Comp}" );
	
	h_CompDiff->GetXaxis()->SetTitle( "CCAL id." );
	h_CompDiff->GetYaxis()->SetTitle( "E_{CCAL} - E_{Comp}" );
	h_CompDiff_g->GetXaxis()->SetTitle( "CCAL id." );
	h_CompDiff_g->GetYaxis()->SetTitle( "E_{CCAL} - E_{Comp}" );
	h_CompDiff_e->GetXaxis()->SetTitle( "CCAL id." );
	h_CompDiff_e->GetYaxis()->SetTitle( "E_{CCAL} - E_{Comp}" );
	
	
	
	
	h_CompRatio_v_E       =  new TH2F( "CompRatio_v_E",       "Compton Energy Ratio vs. E_{Comp}",
		120, 0., 12., 2000, 0., 2. );
	h_CompRatio_v_E_e     =  new TH2F( "CompRatio_v_E_e",     "Compton Energy Ratio vs. E_{Comp} (electrons)",
		120, 0., 12., 2000, 0., 2. );
	h_CompRatio_v_E_g     =  new TH2F( "CompRatio_v_E_g",     "Compton Energy Ratio vs. E_{Comp} (photons)",
		120, 0., 12., 2000, 0., 2. );
	
	h_CompRatio_v_E->GetXaxis()->SetTitle( "E_{Comp} [GeV]" );
	h_CompRatio_v_E->GetYaxis()->SetTitle( "E_{CCAL} / E_{Comp}" );
	h_CompRatio_v_E_e->GetXaxis()->SetTitle( "E_{Comp} [GeV]" );
	h_CompRatio_v_E_e->GetYaxis()->SetTitle( "E_{CCAL} / E_{Comp}" );
	h_CompRatio_v_E_g->GetXaxis()->SetTitle( "E_{Comp} [GeV]" );
	h_CompRatio_v_E_g->GetYaxis()->SetTitle( "E_{CCAL} / E_{Comp}" );
	
	
	
	
	h_xy_fcal     = new TH2F( "xy_fcal", "Distribution of FCAL Showers; x_{FCAL} [cm]; y_{FCAL} [cm]", 
		1000, -60., 60., 1000, -60., 60. );
	h_xy_ccal     = new TH2F( "xy_ccal", "Distribution of CCAL Showers; x_{FCAL} [cm]; y_{FCAL} [cm]", 
		1000, -13., 13., 1000, -13., 13. );
	
	
	h_tof_match = new TH1F( "tof_match", "TOF Match", 2, -0.5, 1.5 );
	
	
	
	
	
  	return NOERROR;
}




///==========================================================
//
//   brun
//
//==========================================================

jerror_t JEventProcessor_CCAL_ComptonGains::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	
	DGeometry*   dgeom = NULL;
  	DApplication* dapp = dynamic_cast< DApplication* >( eventLoop->GetJApplication() );
  	if( dapp )   dgeom = dapp->GetDGeometry( runnumber );
   	
	if( dgeom ){
    	  	dgeom->GetTargetZ( m_beamZ );
		dgeom->GetFCALPosition( m_fcalX, m_fcalY, m_fcalZ );
		dgeom->GetCCALPosition( m_ccalX, m_ccalY, m_ccalZ );
  	} else{
    	  	cerr << "No geometry accessbile to compton_analysis plugin." << endl;
    	  	return RESOURCE_UNAVAILABLE;
  	}
	
	
	jana::JCalibration *jcalib = japp->GetJCalibration(runnumber);
  	std::map<string, float> beam_spot;
  	jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  	m_beamX  =  beam_spot.at("x");
  	m_beamY  =  beam_spot.at("y");
	
	
	
	
  	return NOERROR;
}




//==========================================================
//
//   evnt
//
//==========================================================

jerror_t JEventProcessor_CCAL_ComptonGains::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{
	
	
	
	if( BYPASS_TRIG ) {}
	else {
		const DL1Trigger *trig = NULL;
  		try {
      	  		eventLoop->GetSingle(trig);
  		} catch (...) {}
		if (trig == NULL) { return NOERROR; }
		
		uint32_t fp_trigmask = trig->fp_trig_mask; 
		if( fp_trigmask ) return NOERROR;
	}
	
	
	
	// Get the RF Time
	
	const DEventRFBunch *locRFBunch = NULL;
	try { 
	  	eventLoop->GetSingle( locRFBunch, "CalorimeterOnly" );
	} catch (...) { return NOERROR; }
	double rfTime = locRFBunch->dTime;
	if( locRFBunch->dNumParticleVotes < 2 ) return NOERROR;
	
	
	
	
	DVector3 vertex;
	vertex.SetXYZ( m_beamX, m_beamY, m_beamZ );
	
	
	
	
	vector< const DBeamPhoton* > beam_photons;
	vector< const DCCALShower* > ccal_showers;
	vector< const DFCALShower* > fcal_showers;
	vector< const DTOFPoint*   >  tof_points;
	
	eventLoop->Get( beam_photons );
	eventLoop->Get( ccal_showers );
	eventLoop->Get( fcal_showers );
	eventLoop->Get(  tof_points  );
	
	vector< const DFCALShower* > ComptonShowers_FCAL;
	vector< const DCCALShower* > ComptonShowers_CCAL;
	
	
	
	
	japp->RootFillLock(this);  // Acquire root lock
	
	
	
	/*
	
	Check the timing of all showers. Select events where there's only one shower 
	in the CCAL and one shower in the FCAL correlated with the event RF time.
	
	*/
	
	int n_fcal_showers = 0;	
	for( vector< const DFCALShower* >::const_iterator show = fcal_showers.begin(); 
		show != fcal_showers.end(); show++ ) {
		
		double loc_E = (*show)->getEnergy();
		
		DVector3 pos = (*show)->getPosition() - vertex;
		double dt    = (*show)->getTime() - (pos.Mag()/c) - rfTime;
		
		h_fcal_rf_dt->Fill( dt );
		
		if( fabs(dt) < RF_TIME_CUT  &&  loc_E > MIN_FCAL_ENERGY_CUT ) {
		  	n_fcal_showers++;
		  	ComptonShowers_FCAL.push_back( (*show) );
		}
		
	}	
	
	
	int n_ccal_showers = 0;	
	for( vector< const DCCALShower* >::const_iterator show = ccal_showers.begin(); 
		show != ccal_showers.end(); show++ ) {
		
		double loc_E = (*show)->E;
		
		DVector3 pos( (*show)->x1, (*show)->y1, (*show)->z );
		pos -= vertex;
		double dt    = (*show)->time - (pos.Mag()/c) - rfTime;
		
		h_ccal_rf_dt->Fill( dt );
		
		if( fabs(dt) < RF_TIME_CUT  &&  loc_E > MIN_CCAL_ENERGY_CUT ) {
			n_ccal_showers++;
			ComptonShowers_CCAL.push_back( (*show) );
		}
		
	}
	
	
	
	if( n_fcal_showers != 1 || n_ccal_showers != 1 ) {
		
		japp->RootFillUnLock(this);  // Release root lock
		return NOERROR;
		
	}
	
	
	
	
	const DFCALShower* show1 = ComptonShowers_FCAL[0];
	
	double e1      =  show1->getEnergy();
	DVector3 pos1  =  show1->getPosition() - vertex;
	double t1      =  show1->getTime() - (pos1.Mag() / c);
	double phi1    =  atan2( pos1.Y(), pos1.X() ) * (180. / TMath::Pi());
	
	int tof_match  =  check_TOF_matches( pos1, t1, rfTime, vertex, tof_points );
	
	
	
	
	const DCCALShower* show2 = ComptonShowers_CCAL[0];
	
	double e2      =  show2->E;
	DVector3 pos2( show2->x1, show2->y1, show2->z );
	pos2          -=  vertex;
	double t2      =  show2->time - (pos2.Mag() / c);	
	double phi2    =  atan2( pos2.Y(), pos2.X() ) * (180. / TMath::Pi());	
	
	int idmax      =  show2->idmax;
	
	
	
	
	double deltaT   =  t1 - t2;
	double deltaPhi =  fabs( phi1-phi2 );
	
	h_deltaT->Fill( deltaT );
	h_deltaPhi->Fill( deltaPhi );
	
	
	
	
	
	//----------   Apply a Coplanarity cut:
	
	if( fabs( deltaPhi - 180. )  >  COPL_CUT ) return NOERROR;
	
	
	
	
	
	
	
	//----------   Loop over Beam Photons:
	
	for( vector< const DBeamPhoton* >::const_iterator gam = beam_photons.begin();
		gam != beam_photons.end(); gam++ ) {
		
		
		double eb    =  (*gam)->lorentzMomentum().E();
		double tb    =  (*gam)->time();
		double brfdt =   tb - rfTime;
		
		h_beam_rf_dt->Fill( brfdt );
		h_beam_ccal_dt->Fill( tb - t2 );
		h_beam_fcal_dt->Fill( tb - t1 );
		
		int bunch_val;
		double loc_weight = 0.;
		
		if( fabs(brfdt) < RF_TIME_CUT ) {
			
			bunch_val  = 1;
			loc_weight = 1.0;
			
		}
		else if( ( -( RF_TIME_CUT + (3+n_bunches_acc)*4.008 ) <= brfdt 
			&& brfdt <= -( RF_TIME_CUT + 3*4.008 ) ) 
			||
			 (  ( RF_TIME_CUT + 3*4.008 ) <= brfdt 
			&& brfdt <=  ( RF_TIME_CUT + (3+n_bunches_acc)*4.008 ) ) ) {
			
			bunch_val  = 0;
			loc_weight = -0.5 / (double)n_bunches_acc;
			
		} else 
			continue;
		
		
		
		
		if( eb < MIN_BEAM_ENERGY_CUT ) continue;
		
		
		
		
		
		
		double ecomp1 = 1. / ( 1./eb  +  (1. - cos(pos1.Theta())) / m_e );
		double ecomp2 = 1. / ( 1./eb  +  (1. - cos(pos2.Theta())) / m_e );
		
		double deltaK = (ecomp1 + ecomp2) - (eb + m_e);
		double deltaE = (e1     + e2    ) - (eb + m_e);
		
		
		
		
		h_deltaE->Fill( deltaE, loc_weight );
		h_deltaK->Fill( deltaK, loc_weight );
		
		
		if( fabs(deltaK) > DELTA_K_CUT ) continue;
		
		
		h_CompRatio->Fill(  idmax, e2/ecomp2, loc_weight );
		h_CompDiff->Fill(   idmax, e2-ecomp2, loc_weight );
		h_CompRatio_v_E->Fill( e2, e2/ecomp2, loc_weight );
		
		if( tof_match ) {
			
			h_CompRatio_g->Fill(  idmax, e2/ecomp2, loc_weight );
			h_CompDiff_g->Fill(   idmax, e2-ecomp2, loc_weight );
			h_CompRatio_v_E_g->Fill( e2, e2/ecomp2, loc_weight );
			
		} else {
			
			h_CompRatio_e->Fill(  idmax, e2/ecomp2, loc_weight );
			h_CompDiff_e->Fill(   idmax, e2-ecomp2, loc_weight );
			h_CompRatio_v_E_e->Fill( e2, e2/ecomp2, loc_weight );
			
		}
		
		if( bunch_val ) {
			h_xy_fcal->Fill( pos1.X(), pos1.Y() );
			h_xy_ccal->Fill( pos2.X(), pos2.Y() );
		}
		
	}
	
	japp->RootFillUnLock(this);  // Release root lock
	
	
	
	
	
	
  	return NOERROR;
}




//==========================================================
//
//   erun
//
//==========================================================

jerror_t JEventProcessor_CCAL_ComptonGains::erun(void)
{
  	
  	return NOERROR;
}




//==========================================================
//
//   fini
//
//==========================================================

jerror_t JEventProcessor_CCAL_ComptonGains::fini(void)
{
	
  	return NOERROR;
}




//==========================================================
//
//   Get TOF Position from bar
//
//==========================================================

double JEventProcessor_CCAL_ComptonGains::bar2x(int bar) {

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




//==========================================================
//
//   Check for spacetime matches between TOF and FCAL
//
//==========================================================

int JEventProcessor_CCAL_ComptonGains::check_TOF_matches( DVector3 pos, double t1, 
	double rfTime, DVector3 vertex, vector< const DTOFPoint* > tof_points ) 
{
	
	int tof_match = 0;
	
	for( vector< const DTOFPoint* >::const_iterator tof = tof_points.begin();
		tof != tof_points.end(); tof++ ) {
		
		double xt = (*tof)->pos.X() - vertex.X();
		double yt = (*tof)->pos.Y() - vertex.Y();
		double zt = (*tof)->pos.Z() - vertex.Z();		
		double rt = sqrt( xt*xt  +  yt*yt  +  zt*zt );
		
		double tt = (*tof)->t  -  (rt/c);		
		h_tof_rf_dt->Fill( tt - rfTime );
		
		if( fabs(tt - rfTime) > RF_TIME_CUT ) continue;
		
		xt *= (pos.Z() / zt);
		yt *= (pos.Z() / zt);
		
		int hbar  = (*tof)->dHorizontalBar;
		int hstat = (*tof)->dHorizontalBarStatus;
		int vbar  = (*tof)->dVerticalBar;
		int vstat = (*tof)->dVerticalBarStatus;
		
		double dx, dy;
		
		if( hstat==3 && vstat==3 ) // both planes have hits in both ends
		{
			dx = pos.X() - xt;
			dy = pos.Y() - yt;
		} else if( hstat==3 ) // only good position information in horizontal plane
		{
			dx = pos.X() - xt;
			dy = pos.Y() - (bar2x(hbar) - vertex.Y()) * (pos.Z()/zt);
		} else if( vstat==3 ) 
		{
			dx = pos.X() - (bar2x(vbar) - vertex.X()) * (pos.Z()/zt);
			dy = pos.Y() - yt;
		} else
		{
			dx = pos.X() - (bar2x(vbar) - vertex.X()) * (pos.Z()/zt);
			dy = pos.Y() - (bar2x(hbar) - vertex.Y()) * (pos.Z()/zt);
		}
		
		
		if( hstat==3 ) 
			h_fcal_tof_dx->Fill( dx );
		else 
			h_fcal_tof_dx_bar->Fill( dx );
		
		
		if( vstat==3 ) 
			h_fcal_tof_dy->Fill( dy );
		else 
			h_fcal_tof_dy_bar->Fill( dy );
		
		
		h_fcal_tof_dt->Fill( t1 - tt );
		
		
		
		if( (fcal_tof_x1 < dx && dx < fcal_tof_x2) && (fcal_tof_y1 < dy && dy < fcal_tof_y2) ) 
			tof_match = 1;
		
		h_tof_match->Fill( tof_match );
		
	}
	
	
	
	
	
	
	return tof_match;
}



