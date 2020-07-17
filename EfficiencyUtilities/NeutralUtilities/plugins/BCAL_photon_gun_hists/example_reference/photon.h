// $Id$
//
//    File: DEventProcessor_photoneff_hists.h
// Created: Thu Feb 12 09:43:13 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _photon_
#define _photon_

#include <TObject.h>
#include <TVector3.h>


class photon:public TObject{

	public:

		TVector3 pthrown;
		TVector3 pfit;
		double chisq;
		int Ndof;
		double delta_E_over_E;
		double delta_theta;	// mrad
		double delta_phi;		// mrad
		bool isreconstructable;
		int Nbcal;
		int Nfcal;
		unsigned long event;

	private:
		ClassDef(photon,1);

};

#endif // _photon_

