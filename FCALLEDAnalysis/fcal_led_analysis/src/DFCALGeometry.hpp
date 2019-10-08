// $Id$
//
//    File: DFCALGeometry.hpp
// Created: Wed Aug 24 10:09:27 EST 2005
// Creator: shepherd (on Darwin 129-79-159-16.dhcp-bl.indiana.edu 8.2.0 powerpc)
//

#ifndef _DFCALGeometry_hpp_
#define _DFCALGeometry_hpp_

#include <iostream>

#include <TVector2.h>

class DFCALGeometry {

public:

	DFCALGeometry();
	~DFCALGeometry(){}

	// these numbers are fixed for the FCAL as constructed
	// it probably doesn't make sense to retrieve them
	// from a database as they aren't going to change unless
	// the detector is reconstructed

	enum { kBlocksWide = 59 };
	enum { kBlocksTall = 59 };
	enum { kMaxChannels = kBlocksWide * kBlocksTall };
	enum { kMidBlock = ( kBlocksWide - 1 ) / 2 };
	enum { kBeamHoleSize = 3 };
	enum { k_cm = 1 };
	enum { k_m = 100 };

	static double blockSize()  { return 4.0157*k_cm; }
	static double radius()  { return 1.20471*k_m; }
	static double blockLength()  { return 45.0*k_cm; }
	//	static double fcalFaceZ()  { return 625.3*k_cm; }

	//        static double fcalMidplane() { return fcalFaceZ() + 0.5 * blockLength() ; }

	bool isBlockActive( int row, int column ) const;
	int  numActiveBlocks() const { return m_numActiveBlocks; }


	TVector2 positionOnFace( int row, int column ) const;
	TVector2 positionOnFace( int channel ) const;

	int channel( int row, int column ) const;

	int row   ( int channel ) const { return m_row[channel];    }
	int column( int channel ) const { return m_column[channel]; }

	// get row and column from x and y positions
	int row   ( float y ) const;
	int column( float x ) const;


private:

	bool   m_activeBlock[kBlocksTall][kBlocksWide];
	TVector2 m_positionOnFace[kBlocksTall][kBlocksWide];

	int    m_channelNumber[kBlocksTall][kBlocksWide];
	int    m_row[kMaxChannels];
	int    m_column[kMaxChannels];

	int    m_numActiveBlocks;
};

#endif // _DFCALGeometry_hpp

