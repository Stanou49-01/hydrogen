/*
 * Hydrogen
 * Copyright(c) 2002-2006 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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

#include <QLabel>
#include <QPixmap>
#include <QGridLayout>


#include "InstrumentEditorPanel.h"
#include "../Skin.h"


InstrumentEditorPanel* InstrumentEditorPanel::m_pInstance = NULL;

InstrumentEditorPanel* InstrumentEditorPanel::getInstance()
{
	if ( m_pInstance == NULL  ) {
		m_pInstance = new InstrumentEditorPanel( NULL );
	}
	return m_pInstance;
}



InstrumentEditorPanel::InstrumentEditorPanel( QWidget *pParent )
 : Object( "InstrumentEditorPanel" )
{
	UNUSED( pParent );

	INFOLOG( "INIT" );
	ERRORLOG("CHI CAVOLO LO USA??? Eliminare InstrumentEditorPanel");

	
	m_pInstrumentEditor = new InstrumentEditor( 0 );

	// LAYOUT
	QGridLayout *vbox = new QGridLayout();
	vbox->setSpacing( 0 );
	vbox->setMargin( 0 );

	vbox->addWidget( m_pInstrumentEditor, 0, 0 );

	this->setLayout( vbox );
}



InstrumentEditorPanel::~InstrumentEditorPanel()
{
	INFOLOG( "DESTROY" );
}



void InstrumentEditorPanel::selectLayer( int nLayer )
{
	m_pInstrumentEditor->selectLayer( nLayer );
}

