/***************************************************************************
                          rksettings  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RKSETTINGS_H
#define RKSETTINGS_H

#include <kpagedialog.h>

#include <q3valuelist.h>

class RKSettingsModule;
class KConfig;
class RKWardMainWindow;
class RKSettingsTracker;
class RCommandChain;

/**
The main settings-dialog. Contains subsections (tabs) for different modules. Use configureSettings () to invoke or raise the settings dialog

@author Thomas Friedrichsmeier
*/
class RKSettings : public KPageDialog {
	Q_OBJECT
public:
	enum SettingsPage {
		NoPage=0,
		Plugins=1,
		R=2,
		RPackages=3,
		PHP=4,
		General=5,
		Output=6,
		Watch=7,
		Console=8,
		ObjectBrowser=9,
		NumPages = ObjectBrowser + 1
	};

	static void configureSettings (SettingsPage page=NoPage, QWidget *parent=0, RCommandChain *chain=0);

	static void loadSettings (KConfig *config);
	static void saveSettings (KConfig *config);
	
	void enableApply ();
	
	static RKSettingsTracker* tracker () { return settings_tracker; };
public slots:
	void pageAboutToBeShown (QWidget *page);
protected:
	void slotButtonClicked (int button);
protected:
	RKSettings (QWidget *parent = 0);
	~RKSettings ();
private:
	void applyAll ();
	void initModules ();
	void raisePage (SettingsPage page);
	static void dialogClosed ();

	typedef Q3ValueList<RKSettingsModule *> ModuleList;
	ModuleList modules;
	KPageWidgetItem *pages[NumPages];

	static RKSettings *settings_dialog;
friend class RKWardMainWindow;
	static RKSettingsTracker *settings_tracker;
};

/** This class represents a very simple QObject. It's only purpose is to emit signals when certain settings have changed. Classes that need to
update themselves on certain changed settings should connect to those signals. */
class RKSettingsTracker : public QObject {
	Q_OBJECT
public:
	RKSettingsTracker (QObject *parent);
	~RKSettingsTracker ();

	void settingsChangedObjectBrowser ();
	void settingsChangedMaxCommandLogLines () { maxCommandLogLinesChanged (); };
signals:
	void objectBrowserSettingsChanged ();
	void maxCommandLogLinesChanged ();
};

#endif
