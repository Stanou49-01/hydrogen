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

#include "HydrogenApp.h"
#include "Skin.h"
#include "PreferencesDialog.h"
#include "MainForm.h"
#include "PlayerControl.h"
#include "AudioEngineInfoForm.h"
#include "HelpBrowser.h"
#include "DrumkitManager.h"
#include "LadspaFXProperties.h"
#include "InstrumentRack.h"

#include "PatternEditor/PatternEditorPanel.h"
#include "InstrumentEditor/InstrumentEditorPanel.h"
#include "Mixer/Mixer.h"
#include "SongEditor/SongEditor.h"
#include "SongEditor/SongEditorPanel.h"

#include <hydrogen/Hydrogen.h>
#include <hydrogen/EventQueue.h>
#include <hydrogen/fx/LadspaFX.h>
#include <hydrogen/Preferences.h>

#include <QHBoxLayout>
#include <QSplitter>
#include <QDir>
#include <QPixmap>
#include <QToolBar>
#include <QDockWidget>

using namespace H2Core;

HydrogenApp* HydrogenApp::m_pInstance = NULL;

HydrogenApp::HydrogenApp( MainForm *pMainForm, Song *pFirstSong )
 : Object( "HydrogenApp" )
 , m_pMainForm( pMainForm )
 , m_pMixer( NULL )
 , m_pPatternEditorPanel( NULL )
 , m_pAudioEngineInfoForm( NULL )
 , m_pSongEditorPanel( NULL )
 , m_pHelpBrowser( NULL )
 , m_pFirstTimeInfo( NULL )
 , m_pPlayerControl( NULL )
{
	m_pInstance = this;

	m_pEventQueueTimer = new QTimer(this);
	connect( m_pEventQueueTimer, SIGNAL( timeout() ), this, SLOT( onEventQueueTimer() ) );
	m_pEventQueueTimer->start(50);	// update at 20 fps


	// Create the audio engine :)
	Hydrogen::getInstance();
	Hydrogen::getInstance()->setSong( pFirstSong );
	Preferences::getInstance()->setLastSongFilename( pFirstSong->getFilename() );

	// set initial title
	QString extraVersion = " (Development version) ";
	QString qsSongName( pFirstSong->m_sName.c_str() );
	m_pMainForm->setWindowTitle( ( "Hydrogen " + QString(VERSION) + extraVersion + QString( " - " ) + qsSongName ) );

	Preferences *pPref = Preferences::getInstance();

	setupSinglePanedInterface();

	// restore audio engine form properties
	m_pAudioEngineInfoForm = new AudioEngineInfoForm( 0 );
	WindowProperties audioEngineInfoProp = pPref->getAudioEngineInfoProperties();
	m_pAudioEngineInfoForm->move( audioEngineInfoProp.x, audioEngineInfoProp.y );
	if ( audioEngineInfoProp.visible ) {
		m_pAudioEngineInfoForm->show();
	}
	else {
		m_pAudioEngineInfoForm->hide();
	}

	showInfoSplash();	// First time information
}



HydrogenApp::~HydrogenApp()
{
	INFOLOG( "[~HydrogenApp]" );
	m_pEventQueueTimer->stop();

	delete m_pHelpBrowser;
	delete m_pAudioEngineInfoForm;
	delete m_pDrumkitManager;
	delete m_pMixer;

	Hydrogen *engine = Hydrogen::getInstance();
	if (engine) {
		delete engine->getSong();
		delete engine;
	}

	#ifdef LADSPA_SUPPORT
	for (uint nFX = 0; nFX < MAX_FX; nFX++) {
		delete m_pLadspaFXProperties[nFX];
	}
	#endif
}



/// Return an HydrogenApp m_pInstance
HydrogenApp* HydrogenApp::getInstance() {
	if (m_pInstance == NULL) {
		std::cerr << "Error! HydrogenApp::getInstance (m_pInstance = NULL)" << std::endl;
	}
	return m_pInstance;
}




void HydrogenApp::setupSinglePanedInterface()
{
	Preferences *pPref = Preferences::getInstance();

	// MAINFORM
	WindowProperties mainFormProp = pPref->getMainFormProperties();
	m_pMainForm->resize( mainFormProp.width, mainFormProp.height );
	m_pMainForm->move( mainFormProp.x, mainFormProp.y );

	QSplitter *pSplitter = new QSplitter( NULL );
	pSplitter->setOrientation( Qt::Vertical );
	pSplitter->setOpaqueResize( true );

	// SONG EDITOR
	m_pSongEditorPanel = new SongEditorPanel( pSplitter );
	WindowProperties songEditorProp = pPref->getSongEditorProperties();
	m_pSongEditorPanel->resize( songEditorProp.width, songEditorProp.height );

	// this HBox will contain the InstrumentRack and the Pattern editor
	QWidget *pSouthPanel = new QWidget( pSplitter );
	QHBoxLayout *pEditorHBox = new QHBoxLayout();
	pEditorHBox->setSpacing( 5 );
	pEditorHBox->setMargin( 0 );
	pSouthPanel->setLayout( pEditorHBox );

	// INSTRUMENT RACK
	m_pInstrumentRack = new InstrumentRack( NULL );

	// PATTERN EDITOR
	m_pPatternEditorPanel = new PatternEditorPanel( NULL );
	WindowProperties patternEditorProp = pPref->getPatternEditorProperties();
	m_pPatternEditorPanel->resize( patternEditorProp.width, patternEditorProp.height );

	pEditorHBox->addWidget( m_pPatternEditorPanel );
	pEditorHBox->addWidget( m_pInstrumentRack );

	// PLayer control
	m_pPlayerControl = new PlayerControl( NULL );


	QWidget *mainArea = new QWidget( m_pMainForm );	// this is the main widget
	m_pMainForm->setCentralWidget( mainArea );

	// LAYOUT!!
	QVBoxLayout *pMainVBox = new QVBoxLayout();
	pMainVBox->setSpacing( 5 );
	pMainVBox->setMargin( 0 );
	pMainVBox->addWidget( m_pPlayerControl );
	pMainVBox->addWidget( pSplitter );

	mainArea->setLayout( pMainVBox );




	// MIXER
	m_pMixer = new Mixer(0);
	WindowProperties mixerProp = pPref->getMixerProperties();
	m_pMixer->resize( mixerProp.width, mixerProp.height );
	m_pMixer->move( mixerProp.x, mixerProp.y );
	if ( mixerProp.visible ) {
		m_pMixer->show();
	}
	else {
		m_pMixer->hide();
	}


	// DRUMKIT MANAGER
	m_pDrumkitManager = new OldDrumkitManager( 0 );
	WindowProperties drumkitMngProp = pPref->getDrumkitManagerProperties();
	m_pDrumkitManager->move( drumkitMngProp.x, drumkitMngProp.y );
	if ( drumkitMngProp.visible ) {
		m_pDrumkitManager->show();
	}
	else {
		m_pDrumkitManager->hide();
	}

	// HELP BROWSER
	string sDocPath = string( DataPath::getDataPath() ) + "/doc";
	string sDocURI = sDocPath + "/manual.html";
	m_pHelpBrowser = new SimpleHTMLBrowser( NULL, sDocPath, sDocURI, SimpleHTMLBrowser::MANUAL );

#ifdef LADSPA_SUPPORT
	// LADSPA FX
	for (uint nFX = 0; nFX < MAX_FX; nFX++) {
		m_pLadspaFXProperties[nFX] = new LadspaFXProperties( NULL, nFX );
		m_pLadspaFXProperties[nFX]->hide();
		WindowProperties prop = pPref->getLadspaProperties(nFX);
		m_pLadspaFXProperties[nFX]->move( prop.x, prop.y );
		if ( prop.visible ) {
			m_pLadspaFXProperties[nFX]->show();
		}
		else {
			m_pLadspaFXProperties[nFX]->hide();
		}
	}
#endif

//	m_pMainForm->showMaximized();
}



void HydrogenApp::setSong(Song* song)
{

#ifdef LADSPA_SUPPORT
	for (uint nFX = 0; nFX < MAX_FX; nFX++) {
		m_pLadspaFXProperties[nFX]->hide();
	}
#endif

	Song* oldSong = (Hydrogen::getInstance())->getSong();
	if (oldSong != NULL) {
		(Hydrogen::getInstance())->removeSong();
		delete oldSong;
		oldSong = NULL;
	}

	Hydrogen::getInstance()->setSong( song );
	Preferences::getInstance()->setLastSongFilename( song->getFilename() );

	m_pSongEditorPanel->updateAll();

	QString songName( song->m_sName.c_str() );
	m_pMainForm->setWindowTitle( ( "Hydrogen " + QString(VERSION) + QString( " - " ) + songName ) );

	m_pMainForm->updateRecentUsedSongList();
}



void HydrogenApp::showMixer(bool show)
{
	m_pMixer->setVisible( show );
}



void HydrogenApp::showPreferencesDialog()
{
	PreferencesDialog preferencesDialog(m_pMainForm);
	preferencesDialog.exec();
}




void HydrogenApp::setStatusBarMessage( const QString& msg, int msec )
{
	getPlayerControl()->showMessage( msg, msec );
}





void HydrogenApp::showAudioEngineInfoForm()
{
	m_pAudioEngineInfoForm->hide();
	m_pAudioEngineInfoForm->show();
}



void HydrogenApp::showInfoSplash()
{
	QString sDocPath( DataPath::getDataPath().append( "/doc/infoSplash" ).c_str() );

	QDir dir(sDocPath);
	if ( !dir.exists() ) {
		ERRORLOG( string("[showInfoSplash] Directory ").append( sDocPath.toStdString() ).append( " not found." ) );
		return;
	}

	string sFilename = "";
	int nNewsID = 0;
	QFileInfoList list = dir.entryInfoList();

	for ( int i =0; i < list.size(); ++i ) {
		string sFile = list.at( i ).fileName().toStdString();

		if ( sFile == "." || sFile == ".." ) {
			continue;
		}

		int nPos = sFile.rfind("-");
		string sNewsID = sFile.substr( nPos + 1, sFile.length() - nPos - 1 );
		int nID = atoi( sNewsID.c_str() );
		if ( nID > nNewsID ) {
			sFilename = sFile;
		}
//		INFOLOG( "news: " + sFilename + " id: " + sNewsID );
	}
	INFOLOG( "[showInfoSplash] Selected news: " + sFilename );

	string sLastRead = Preferences::getInstance()->getLastNews();
	if ( sLastRead != sFilename && sFilename != "" ) {
		string sDocURI = sDocPath.toStdString();
		sDocURI.append( "/" ).append( sFilename.c_str() );
		SimpleHTMLBrowser *m_pFirstTimeInfo = new SimpleHTMLBrowser( m_pMainForm, sDocPath.toStdString(), sDocURI, SimpleHTMLBrowser::WELCOME );
		if ( m_pFirstTimeInfo->exec() == QDialog::Accepted ) {
			Preferences::getInstance()->setLastNews( sFilename );
		}
		else {
		}
	}
}



void HydrogenApp::onEventQueueTimer()
{
	EventQueue *pQueue = EventQueue::getInstance();

	Event event;
	while ( ( event = pQueue->popEvent() ).m_type != EVENT_NONE ) {
		for (int i = 0; i < (int)m_eventListeners.size(); i++ ) {
			EventListener *pListener = m_eventListeners[ i ];

			switch ( event.m_type ) {
				case EVENT_STATE:
					pListener->stateChangedEvent( event.m_nValue );
					break;

				case EVENT_PATTERN_CHANGED:
					pListener->patternChangedEvent();
					break;

				case EVENT_PATTERN_MODIFIED:
					pListener->patternModifiedEvent();
					break;

				case EVENT_SELECTED_PATTERN_CHANGED:
					pListener->selectedPatternChangedEvent();
					break;

				case EVENT_SELECTED_INSTRUMENT_CHANGED:
					pListener->selectedInstrumentChangedEvent();
					break;

				case EVENT_MIDI_ACTIVITY:
					pListener->midiActivityEvent();
					break;

				case EVENT_NOTEON:
					pListener->noteOnEvent( event.m_nValue );
					break;

				case EVENT_ERROR:
					pListener->errorEvent( event.m_nValue );
					break;

				case EVENT_XRUN:
					pListener->XRunEvent();
					break;

				case EVENT_METRONOME:
					pListener->metronomeEvent( event.m_nValue );
					break;

				case EVENT_PROGRESS:
					pListener->progressEvent( event.m_nValue );
					break;

				default:
					ERRORLOG( "[onEventQueueTimer] Unhandled event: " + toString( event.m_type ) );
			}

		}
	}
}


void HydrogenApp::addEventListener( EventListener* pListener )
{
	if (pListener) {
		m_eventListeners.push_back( pListener );
	}
}


void HydrogenApp::removeEventListener( EventListener* pListener )
{
	for ( uint i = 0; i < m_eventListeners.size(); i++ ) {
		if ( pListener == m_eventListeners[ i ] ) {
			m_eventListeners.erase( m_eventListeners.begin() + i );
		}
	}
}
