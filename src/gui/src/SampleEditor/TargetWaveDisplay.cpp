/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <hydrogen/basics/sample.h>
#include <hydrogen/basics/song.h>
#include <hydrogen/basics/instrument.h>
#include <hydrogen/basics/instrument_layer.h>

#include "HydrogenApp.h"
#include "SampleEditor.h"

using namespace H2Core;

#define UI_WIDTH   841
#define UI_HEIGHT   91

#include <vector>
#include <algorithm>
#include "TargetWaveDisplay.h"
#include "../Skin.h"

const char* TargetWaveDisplay::__class_name = "TargetWaveDisplay";

TargetWaveDisplay::TargetWaveDisplay(QWidget* pParent)
 : QWidget( pParent )
 , Object( __class_name )
 , m_sSampleName( "" )
{
//	setAttribute(Qt::WA_NoBackground);

	//INFOLOG( "INIT" );
	int w = UI_WIDTH;
	int h = UI_HEIGHT;
	resize( w, h );

	bool ok = m_background.load( Skin::getImagePath() + "/waveDisplay/targetsamplewavedisplay.png" );
	if( ok == false ){
		ERRORLOG( "Error loading pixmap" );
	}

	m_pPeakDatal = new int[ w ];
	m_pPeakDatar = new int[ w ];
	m_pvmove = false;
	m_info = "";
	m_x = -10;
	m_y = -10;
	m_plocator = -1;
	m_pupdateposi = false;
}




TargetWaveDisplay::~TargetWaveDisplay()
{
	//INFOLOG( "DESTROY" );

	delete[] m_pPeakDatal;
	delete[] m_pPeakDatar;
}



void TargetWaveDisplay::paintEvent(QPaintEvent *ev)
{
	QPainter painter( this );
	painter.setRenderHint( QPainter::HighQualityAntialiasing );
	painter.drawPixmap( ev->rect(), m_background, ev->rect() );

	painter.setPen( QColor( 252, 142, 73 ));
	int VCenter = height() / 2;
	int lcenter = VCenter -4;
	int rcenter = VCenter +4;
	for ( int x = 0; x < width(); x++ ) {
		painter.drawLine( x, lcenter, x, -m_pPeakDatal[x +1] +lcenter  );
	}

	painter.setPen( QColor( 116, 186, 255 ));
	for ( int x = 0; x < width(); x++ ) {
		painter.drawLine( x, rcenter, x, -m_pPeakDatar[x +1] +rcenter  );
	}

	QFont font;
	font.setWeight( 63 );
	painter.setFont( font );
//start frame pointer
//	painter.setPen( QColor( 99, 175, 254, 200 ) );
//	painter.drawLine( m_pFadeOutFramePosition, 4, m_pFadeOutFramePosition, height() -4 );	
//	painter.drawText( m_pFadeOutFramePosition , 1, 10,20, Qt::AlignRight, "F" );

	for ( int i = 0; i < static_cast<int>(__velocity.size()) -1; i++){
		//volume line
		painter.setPen( QPen(QColor( 255, 255, 255, 200 ) ,1 , Qt::SolidLine) );
		painter.drawLine( __velocity[i].frame, __velocity[i].value, __velocity[i + 1].frame, __velocity[i +1].value );
		painter.setBrush(QColor( 99, 160, 233 ));
		painter.drawEllipse ( __velocity[i].frame - 6/2, __velocity[i].value  - 6/2, 6, 6 );
	}

	for ( int i = 0; i < static_cast<int>(__pan.size()) -1; i++){
		//pan line
		painter.setPen( QPen(QColor( 249, 235, 116, 200 ) ,1 , Qt::SolidLine) );
		painter.drawLine( __pan[i].frame, __pan[i].value, __pan[i + 1].frame, __pan[i +1].value );
		painter.setBrush(QColor( 77, 189, 55 ));
		painter.drawEllipse ( __pan[i].frame - 6/2, __pan[i].value  - 6/2, 6, 6 );
	}


	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.setPen( QPen( QColor( 255, 255, 255 ), 1, Qt::SolidLine ) );
	painter.drawLine( m_plocator, 4, m_plocator, height() -4);

	//volume line
	//first rect 
	painter.setPen( QPen(QColor( 255, 255, 255, 200 ) ,1 , Qt::SolidLine) );
	painter.setBrush(QColor( 99, 160, 233 ));
	painter.drawRect ( __velocity[0].frame - 12/2, __velocity[0].value  - 6/2, 12, 6 );
	//last rect 
	painter.drawRect ( __velocity[__velocity.size() -1].frame - 12/2, __velocity[__velocity.size() -1].value  - 6/2, 12, 6 );

	//pan line
	//first rect 
	painter.setPen( QPen(QColor( 249, 235, 116, 200 ) ,1 , Qt::SolidLine) );
	painter.setBrush(QColor( 77, 189, 55 ));
	painter.drawRect ( __pan[0].frame - 12/2, __pan[0].value  - 6/2, 12, 6 );
	//last rect 
	painter.drawRect ( __pan[__pan.size() -1].frame - 12/2, __pan[__pan.size() -1].value  - 6/2, 12, 6 );


	painter.setPen( QPen( QColor( 255, 255, 255 ), 1, Qt::DotLine ) );
	painter.drawLine( 0, lcenter, UI_WIDTH, lcenter );
	painter.setPen( QPen( QColor( 255, 255, 255 ), 1, Qt::DotLine ) );
	painter.drawLine( 0, rcenter, UI_WIDTH, rcenter );

	if (m_y < 50){
		if (m_x < 790){
			painter.drawText( m_x +5, m_y, 60, 20, Qt::AlignLeft, QString( m_info ) );
		}else
		{
			painter.drawText( m_x - 65, m_y, 60, 20, Qt::AlignRight, QString( m_info ) );
		}
		
	}else
	{
		if (m_x < 790){
			painter.drawText( m_x +5, m_y -20, 60, 20, Qt::AlignLeft, QString( m_info ) );
		}else
		{
			painter.drawText( m_x - 65, m_y -20, 60, 20, Qt::AlignRight, QString( m_info ) );
		}
	}

}


void TargetWaveDisplay::updateDisplayPointer()
{
	update();
}

void TargetWaveDisplay::paintLocatorEventTargetDisplay( int pos, bool updateposi)
{
	m_pupdateposi = updateposi;
	if ( !updateposi ){
		m_plocator = -1;
	}else
	{
		m_plocator = pos;
	}
	update();
}

void TargetWaveDisplay::updateDisplay( H2Core::InstrumentLayer *pLayer )
{
	if ( pLayer && pLayer->get_sample() ) {

		int nSampleLength = pLayer->get_sample()->get_frames();
		float nScaleFactor = nSampleLength / width();

		float fGain = (height() - 8) / 2.0 * pLayer->get_gain();

		float *pSampleDatal = pLayer->get_sample()->get_data_l();
		float *pSampleDatar = pLayer->get_sample()->get_data_r();
		int nSamplePos = 0;
		int nVall;
		int nValr;
		for ( int i = 0; i < width(); ++i ){
			nVall = 0;
			nValr = 0;
			for ( int j = 0; j < nScaleFactor; ++j ) {
				if ( j < nSampleLength ) {
					if ( pSampleDatal[ nSamplePos ] < 0 ){
						int newVal = static_cast<int>( pSampleDatal[ nSamplePos ] * -fGain );
						nVall = newVal;
					}else
					{
						int newVal = static_cast<int>( pSampleDatal[ nSamplePos ] * fGain );
						nVall = newVal;
					}
					if ( pSampleDatar[ nSamplePos ] > 0 ){
						int newVal = static_cast<int>( pSampleDatar[ nSamplePos ] * -fGain );
						nValr = newVal;
					}else
					{
						int newVal = static_cast<int>( pSampleDatar[ nSamplePos ] * fGain );
						nValr = newVal;
					}
				}
				++nSamplePos;
			}
			m_pPeakDatal[ i ] = nVall;
			m_pPeakDatar[ i ] = nValr;
		}
	}

	update();

}


void TargetWaveDisplay::mouseMoveEvent(QMouseEvent *ev)
{
	int snapradius = 10;
	QString editType = HydrogenApp::get_instance()->getSampleEditor()->EditTypeComboBox->currentText();



	///edit volume points
	if( editType == "volume" ){
		m_pvmove = true;
	
		if ( ev->x() <= 0 || ev->x() >= UI_WIDTH || ev->y() < 0 || ev->y() > UI_HEIGHT ){
			update();
			m_pvmove = false;
			return;
		}
		float info = (UI_HEIGHT - ev->y()) / (float)UI_HEIGHT;
		m_info.setNum( info, 'g', 2 );
		m_x = ev->x();
		m_y = ev->y();
	
		for ( int i = 0; i < static_cast<int>(__velocity.size()); i++){
			if ( __velocity[i].frame >= ev->x() - snapradius && __velocity[i].frame <= ev->x() + snapradius ) {
				__velocity.erase( __velocity.begin() + i);
                Sample::EnvelopePoint pt;
                if ( i == 0 ){
                    pt.frame = 0;
                    pt.value = ev->y();
                } else if ( i == static_cast<int>(__velocity.size()) ) {
                    pt.frame = __velocity[i].frame;
                    pt.value = ev->y();

                } else {
                    pt.frame = ev->x();
                    pt.value = ev->y();
                }
                __velocity.push_back( pt );
                sort( __velocity.begin(), __velocity.end(), Sample::EnvelopePoint::Comparator() );
                update();
                return;
			}else
			{
				m_pvmove = false;	
			}
		}
	///edit panorama points
	}else if( editType == "panorama" ){
		m_pvmove = true;
	
		if ( ev->x() <= 0 || ev->x() >= UI_WIDTH || ev->y() < 0 || ev->y() > UI_HEIGHT ){
			update();
			m_pvmove = false;
			return;
		}
		float info = (UI_HEIGHT/2 - ev->y()) / (UI_HEIGHT/2.0);
		m_info.setNum( info, 'g', 2 );
		m_x = ev->x();
		m_y = ev->y();
	
		for ( int i = 0; i < static_cast<int>(__pan.size()); i++){
			if ( __pan[i].frame >= ev->x() - snapradius && __pan[i].frame <= ev->x() + snapradius ) {
				__pan.erase( __pan.begin() + i);
                Sample::EnvelopePoint pt;
                if ( i == 0 ){
                    pt.frame = 0;
                    pt.value = ev->y();
                } else if ( i == static_cast<int>(__pan.size()) ) {
                    pt.frame = __pan[i].frame;
                    pt.value = ev->y();
                } else {
                    pt.frame = ev->x();
                    pt.value = ev->y();
                }
                __pan.push_back( pt );
	            sort( __pan.begin(), __pan.end(), Sample::EnvelopePoint::Comparator() );
                update();
                return;
			}else
			{
				m_pvmove = false;	
			}
		}
	}

	update();
	HydrogenApp::get_instance()->getSampleEditor()->setTrue();
}



void TargetWaveDisplay::mousePressEvent(QMouseEvent *ev)
{
	int snapradius = 6;
	bool newpoint = true;

	// add new point
	QString editType = HydrogenApp::get_instance()->getSampleEditor()->EditTypeComboBox->currentText();


	///edit volume points
	if( editType == "volume" ){
		// test if there is already a point
		for ( int i = 0; i < static_cast<int>(__velocity.size()); ++i){
			if ( __velocity[i].frame >= ev->x() - snapradius && __velocity[i].frame <= ev->x() + snapradius ){
				newpoint = false;
			}
		}
		int x = ev->x();
		int y = ev->y();	
		if (ev->button() == Qt::LeftButton && !m_pvmove && newpoint){
			float info = (UI_HEIGHT - ev->y()) / (float)UI_HEIGHT;
			m_info.setNum( info, 'g', 2 );
			m_x = ev->x();
			m_y = ev->y();
			if ( ev->y() <= 0 ) y = 0;
			if ( ev->y() >= UI_HEIGHT ) y = UI_HEIGHT;
			if ( ev->x() <= snapradius ) x = snapradius;
			if ( ev->x() >= UI_WIDTH-snapradius ) x = UI_WIDTH-snapradius;
			__velocity.push_back( new Sample::EnvelopePoint( x, y ) );
            sort( __velocity.begin(), __velocity.end(), Sample::EnvelopePoint::Comparator() );
		}
	
	
		//remove point
		snapradius = 10;
		if (ev->button() == Qt::RightButton ){
	
			if ( ev->x() <= 0 || ev->x() >= UI_WIDTH ){
				update();
				return;
			}
			m_info = "";

			for ( int i = 0; i < static_cast<int>(__velocity.size()); i++){
				if ( __velocity[i].frame >= ev->x() - snapradius && __velocity[i].frame <= ev->x() + snapradius ){
					if ( __velocity[i].frame == 0 || __velocity[i].frame == UI_WIDTH) return;
					__velocity.erase( __velocity.begin() +  i);
				}
			}	
		}
	}
	///edit panorama points
	else if( editType == "panorama" ){
		// test if there is already a point
		for ( int i = 0; i < static_cast<int>(__pan.size()); ++i){
			if ( __pan[i].frame >= ev->x() - snapradius && __pan[i].frame <= ev->x() + snapradius ){
				newpoint = false;
			}
		}
		int x = ev->x();
		int y = ev->y();	
		if (ev->button() == Qt::LeftButton && !m_pvmove && newpoint){
			float info = (UI_HEIGHT/2 - ev->y()) / (UI_HEIGHT/2.0);
			m_info.setNum( info, 'g', 2 );
			m_x = ev->x();
			m_y = ev->y();
			if ( ev->y() <= 0 ) y = 0;
			if ( ev->y() >= UI_HEIGHT ) y = UI_HEIGHT;
			if ( ev->x() <= snapradius ) x = snapradius;
			if ( ev->x() >= UI_WIDTH-snapradius ) x = UI_WIDTH-snapradius;
			__pan.push_back( new Sample::EnvelopePoint( x, y ) );
	        sort( __pan.begin(), __pan.end(), Sample::EnvelopePoint::Comparator() );
		}
	
	
		//remove point
		snapradius = 10;
		if (ev->button() == Qt::RightButton ){
	
			if ( ev->x() <= 0 || ev->x() >= UI_WIDTH ){
				update();
				return;
			}
			m_info = "";

			for ( int i = 0; i < static_cast<int>(__pan.size()); i++){
				if ( __pan[i].frame >= ev->x() - snapradius && __pan[i].frame <= ev->x() + snapradius ){
					if ( __pan[i].frame == 0 || __pan[i].frame == UI_WIDTH) return;
					__pan.erase( __pan.begin() +  i);
				}
			}	
		}
	}

	update();
	HydrogenApp::get_instance()->getSampleEditor()->setTrue();
}





void TargetWaveDisplay::mouseReleaseEvent(QMouseEvent *ev)
{
	update();
	HydrogenApp::get_instance()->getSampleEditor()->returnAllTargetDisplayValues();
}
