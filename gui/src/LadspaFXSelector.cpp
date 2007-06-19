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

#include "LadspaFXSelector.h"
#include "HydrogenApp.h"
#include <hydrogen/Hydrogen.h>
#include "Skin.h"
#include <hydrogen/Song.h>

#include <hydrogen/fx/Effects.h>

#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QListWidget>
#include <QListWidgetItem>
using namespace std;
using namespace H2Core;

LadspaFXSelector::LadspaFXSelector(int nLadspaFX)
 : QDialog( NULL )
 , Object( "LadspaFXSelector" )
{
	//INFOLOG( "INIT" );

	setupUi( this );

	setFixedSize( width(), height() );

	setWindowTitle( trUtf8( "Select LADSPA FX" ) );
	setWindowIcon( QPixmap( Skin::getImagePath() + "/icon16.png" ) );

	m_sSelectedPluginName = "";

	m_nameLbl->setText( QString("") );
	m_labelLbl->setText( QString("") );
	m_typeLbl->setText( QString("") );
	m_pIDLbl->setText( QString("") );
	m_pMakerLbl->setText( QString("") );
	m_pCopyrightLbl->setText( QString("") );
	m_pPluginsListBox->clear();
	m_pOkBtn->setEnabled(false);

	m_pGroupsListView->setHeaderLabels( QStringList( trUtf8( "Groups" ) ) );

#ifdef LADSPA_SUPPORT
	//Song *pSong = Hydrogen::getInstance()->getSong();
	LadspaFX *pFX = Effects::getInstance()->getLadspaFX(nLadspaFX);
	if (pFX) {
		m_sSelectedPluginName = pFX->getPluginName();
	}
	buildLadspaGroups();

	m_pGroupsListView->sortItems( 0 , Qt::AscendingOrder);
	m_pGroupsListView->setItemHidden( m_pGroupsListView->headerItem(), true );


//	LadspaFXGroup* pFXGroup = LadspaFX::getLadspaFXGroup();
//	vector<LadspaFXInfo*> list = findPluginsInGroup( m_sSelectedPluginName, pFXGroup );
//	for (uint i = 0; i < list.size(); i++) {
//		m_pPluginsListBox->addItem( list[i]->m_sName.c_str() );
//	}
#endif

	connect( m_pPluginsListBox, SIGNAL( itemSelectionChanged () ), this, SLOT( pluginSelected() ) );
}



LadspaFXSelector::~LadspaFXSelector()
{
	//INFOLOG( "DESTROY" );
}



void LadspaFXSelector::buildLadspaGroups()
{
#ifdef LADSPA_SUPPORT
	m_pGroupsListView->clear();

	QTreeWidgetItem* pRootItem = new QTreeWidgetItem( );
	pRootItem->setText( 0, trUtf8("Groups") );
	m_pGroupsListView->addTopLevelItem( pRootItem );
	m_pGroupsListView->setItemExpanded( pRootItem, true );

	H2Core::LadspaFXGroup* pFXGroup = Effects::getInstance()->getLadspaFXGroup();
	for (uint i = 0; i < pFXGroup->getChildList().size(); i++) {
		H2Core::LadspaFXGroup *pNewGroup = ( pFXGroup->getChildList() )[ i ];
		addGroup( pRootItem, pNewGroup );
	}
#endif
}



#ifdef LADSPA_SUPPORT
void LadspaFXSelector::addGroup( QTreeWidgetItem *pItem, H2Core::LadspaFXGroup *pGroup )
{
	QString sGroupName = QString(pGroup->getName().c_str());
	if (sGroupName == QString("Uncategorized")) {
		sGroupName = trUtf8("Uncategorized");
	}
	else if (sGroupName == QString("Categorized(LRDF)")) {
		sGroupName = trUtf8("Categorized (LRDF)");
	}
	QTreeWidgetItem* pNewItem = new QTreeWidgetItem( pItem );
	pNewItem->setText( 0, sGroupName );


	for ( uint i = 0; i < pGroup->getChildList().size(); i++ ) {
		H2Core::LadspaFXGroup *pNewGroup = ( pGroup->getChildList() )[ i ];

		addGroup( pNewItem, pNewGroup );
	}
	for(uint i = 0; i < pGroup->getLadspaInfo().size(); i++) {
		H2Core::LadspaFXInfo* pInfo = (pGroup->getLadspaInfo())[i];
		if (pInfo->m_sName == m_sSelectedPluginName) {
			m_pGroupsListView->setItemSelected(pNewItem, true);
			break;
		}
	}
}
#endif



std::string LadspaFXSelector::getSelectedFX()
{
	return m_sSelectedPluginName;
}



void LadspaFXSelector::pluginSelected()
{
#ifdef LADSPA_SUPPORT
	//INFOLOG( "[pluginSelected]" );

	QString sSelected = m_pPluginsListBox->selectedItems().first()->text();
	m_sSelectedPluginName = sSelected.toStdString();


	std::vector<H2Core::LadspaFXInfo*> pluginList = Effects::getInstance()->getPluginList();
	for (uint i = 0; i < pluginList.size(); i++) {
		H2Core::LadspaFXInfo *pFXInfo = pluginList[i];
		if (pFXInfo->m_sName == m_sSelectedPluginName ) {

			m_nameLbl->setText( QString( pFXInfo->m_sName.c_str() ) );
			m_labelLbl->setText( QString( pFXInfo->m_sLabel.c_str() ) );

			if ( ( pFXInfo->m_nIAPorts == 2 ) && ( pFXInfo->m_nOAPorts == 2 ) ) {		// Stereo plugin
				m_typeLbl->setText( trUtf8("Stereo") );
			}
			else if ( ( pFXInfo->m_nIAPorts == 1 ) && ( pFXInfo->m_nOAPorts == 1 ) ) {	// Mono plugin
				m_typeLbl->setText( trUtf8("Mono") );
			}
			else {
				// not supported
				m_typeLbl->setText( trUtf8("Not supported") );
			}

			m_pIDLbl->setText( QString( pFXInfo->m_sID.c_str() ) );
			m_pMakerLbl->setText( QString( pFXInfo->m_sMaker.c_str() ) );
			m_pCopyrightLbl->setText( QString( pFXInfo->m_sCopyright.c_str() ) );

			break;
		}
	}
	m_pOkBtn->setEnabled(true);
#endif
}



void LadspaFXSelector::on_m_pGroupsListView_currentItemChanged( QTreeWidgetItem * currentItem, QTreeWidgetItem * previous )
{
	UNUSED( previous );
#ifdef LADSPA_SUPPORT
	//INFOLOG( "new selection: " + currentItem->text(0).toStdString() );

	m_pOkBtn->setEnabled(false);
	m_nameLbl->setText( QString("") );
	m_labelLbl->setText( QString("") );
	m_typeLbl->setText( QString("") );
	m_pIDLbl->setText( QString("") );
	m_pMakerLbl->setText( QString("") );
	m_pCopyrightLbl->setText( QString("") );

	// nothing was selected
	if ( m_pGroupsListView->selectedItems().size() == 0 ) {
		return;
	}


	std::string itemText = currentItem->text( 0 ).toStdString();

	//m_pPluginsListBox->clear();

	while( m_pPluginsListBox->count() != 0) {
		m_pPluginsListBox->takeItem( 0 );
	}

	H2Core::LadspaFXGroup* pFXGroup = Effects::getInstance()->getLadspaFXGroup();

	std::vector<H2Core::LadspaFXInfo*> pluginList = findPluginsInGroup( itemText, pFXGroup );
	for (uint i = 0; i < pluginList.size(); i++) {
		//INFOLOG( "adding plugin: " + pluginList[ i ]->m_sName );
		m_pPluginsListBox->addItem( pluginList[ i ]->m_sName.c_str() );
		if ( pluginList[ i ]->m_sName == m_sSelectedPluginName ) {
			m_pPluginsListBox->setCurrentRow( i );
		}
	}
	m_pPluginsListBox->sortItems();
#endif
}


#ifdef LADSPA_SUPPORT
std::vector<H2Core::LadspaFXInfo*> LadspaFXSelector::findPluginsInGroup( const std::string& sSelectedGroup, H2Core::LadspaFXGroup *pGroup )
{
	//INFOLOG( "group: " + sSelectedGroup );
	vector<H2Core::LadspaFXInfo*> list;

	if ( pGroup->getName() == sSelectedGroup ) {
		//INFOLOG( "found..." );
		for ( uint i = 0; i < pGroup->getLadspaInfo().size(); ++i ) {
			H2Core::LadspaFXInfo *pInfo = ( pGroup->getLadspaInfo() )[i];
			list.push_back( pInfo );
		}
		return list;
	}
	else {
		//INFOLOG( "not found...searching in the child groups" );
		for ( uint i = 0; i < pGroup->getChildList().size(); ++i ) {
			H2Core::LadspaFXGroup *pNewGroup = ( pGroup->getChildList() )[ i ];
			list = findPluginsInGroup( sSelectedGroup, pNewGroup );
			if (list.size() != 0) {
				return list;
			}
		}
	}

	//WARNINGLOG( "[findPluginsInGroup] no group found ('" + sSelectedGroup + "')" );
	return list;
}
#endif