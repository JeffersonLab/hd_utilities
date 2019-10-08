// $Id$
//
//    File: DFCALGeometry.cpp
// Created: Wed Aug 24 10:18:56 EST 2005
// Creator: shepherd (on Darwin 129-79-159-16.dhcp-bl.indiana.edu 8.2.0 powerpc)
//

#include <cassert>
#include <cmath>
using namespace std;

#include "DFCALGeometry.hpp"

//---------------------------------
// DFCALGeometry    (Constructor)
//---------------------------------
DFCALGeometry::DFCALGeometry() :
m_numActiveBlocks( 0 )
{
	double innerRadius = ( kBeamHoleSize - 1 ) / 2. * blockSize() * sqrt(2.);

	// inflate the innner radius by 1% to for "safe" comparison
	innerRadius *= 1.01;

	for( int row = 0; row < kBlocksTall; row++ ){
		for( int col = 0; col < kBlocksWide; col++ ){

			// transform to beam axis
			m_positionOnFace[row][col] =
			   TVector2(  ( col - kMidBlock ) * blockSize(),
					     ( row - kMidBlock ) * blockSize() );

			double thisRadius = m_positionOnFace[row][col].Mod();

			if( ( thisRadius < radius() ) && ( thisRadius > innerRadius ) ){

				m_activeBlock[row][col] = true;

				// build the "channel map"
				m_channelNumber[row][col] = m_numActiveBlocks;
				m_row[m_numActiveBlocks] = row;
				m_column[m_numActiveBlocks] = col;

				m_numActiveBlocks++;
			}
			else{

				m_activeBlock[row][col] = false;
			}
		}
	}
}

bool
DFCALGeometry::isBlockActive( int row, int column ) const
{
	// I'm inserting these lines to effectively disable the
	// two assert calls below. They are causing all programs
	// (hd_dump, hdview) to exit, even when I'm not interested
	// in the FCAL. This does not fix the underlying problem
	// of why we're getting invalid row/column values.
	// 12/13/05  DL
	if( row < 0 ||  row >= kBlocksTall )return false;
	if( column < 0 ||  column >= kBlocksWide )return false;

	// assert(    row >= 0 &&    row < kBlocksTall );
	// assert( column >= 0 && column < kBlocksWide );

	return m_activeBlock[row][column];
}

int
DFCALGeometry::row( float y ) const
{
	return static_cast<int>( y / blockSize() + kMidBlock + 0.5);
}

int
DFCALGeometry::column( float x ) const
{
	return static_cast<int>( x / blockSize() + kMidBlock + 0.5);
}

TVector2
DFCALGeometry::positionOnFace( int row, int column ) const
{
  //	assert(    row >= 0 &&    row < kBlocksTall );
  //	assert( column >= 0 && column < kBlocksWide );

  return m_positionOnFace[row][column];
}

TVector2
DFCALGeometry::positionOnFace( int channel ) const
{
	assert( channel >= 0 && channel < m_numActiveBlocks );

	return positionOnFace( m_row[channel], m_column[channel] );
}

int
DFCALGeometry::channel( int row, int column ) const
{
	if( isBlockActive( row, column ) ){

		return m_channelNumber[row][column];
	}
	else{

	  cerr << "ERROR: request for channel number of inactive block!  row "
	       << row << " column " <<  column << endl;
		return -1;
	}
}
