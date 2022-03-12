/***************************************************************************
                          rksettings  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004-2020 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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

#include <qmap.h>

class RKSettingsModule;
class KConfig;
class RKWardMainWindow;
class RKSettingsTracker;
class RCommandChain;
class RKSetupWizardItem;

/**
The main settings-dialog. Contains subsections (tabs) for different modules. Use configureSettings () to invoke or raise the settings dialog

@author Thomas Friedrichsmeier
*/
class RKSettings : public KPageDialog {
	Q_OBJECT
public:
	enum SettingsPage {
		NoPage=0,
		SuperPageAddons,
		PagePlugins,
		PageKatePlugins,
		PageR,
		PageRPackages,
		PageGeneral,
		PageOutput,
		PageX11,
		PageWatch,
		PageConsole,
		PageCommandEditor,
		PageObjectBrowser,
		PageDebug,
		NumPages = PageDebug + 1
	};

	static void configureSettings (SettingsPage page=NoPage, QWidget *parent=0, RCommandChain *chain=0);
	static void configureSettings (const QString& page, QWidget *parent=0, RCommandChain *chain=0);

	static void loadSettings (KConfig *config);
	static void saveSettings (KConfig *config);
	/** Perform any settings validation that may need user interaction (and should happen after a GUI is available, and R has started up) */
	static QList<RKSetupWizardItem*> validateSettingsInteractive ();

	void enableApply ();
	
	static RKSettingsTracker* tracker () { return settings_tracker; };
public slots:
	void pageChange (KPageWidgetItem *current, KPageWidgetItem *before);
protected:
	RKSettings (QWidget *parent = 0);
	~RKSettings ();

	void done (int result) override;
private slots:
	void applyAll ();
	void helpClicked ();
private:
	void initModules ();
	void raisePage (SettingsPage page);
	static void dialogClosed ();

	typedef QMap<int, RKSettingsModule *> ModuleMap;
	ModuleMap modules;
	KPageWidgetItem** pages;

	static RKSettings *settings_dialog;
friend class RKWardMainWindow;
	static RKSettingsTracker *settings_tracker;

	void registerPageModule(SettingsPage super, int child);
};

/** This class represents a very simple QObject. It's only purpose is to emit signals when certain settings have changed. Classes that need to
update themselves on certain changed settings should connect to those signals. */
class RKSettingsTracker : public QObject {
	Q_OBJECT
public:
	explicit RKSettingsTracker (QObject *parent);
	~RKSettingsTracker ();

	void signalSettingsChange (RKSettings::SettingsPage page);
signals:
	void settingsChanged (RKSettings::SettingsPage);
};

#endif
