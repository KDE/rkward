/***************************************************************************
                          rksettings  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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

#include <qdialog.h>

#include <qvaluelist.h>

class RKSettingsModule;
class QTabWidget;
class QPushButton;
class KConfig;
class RKwardApp;

/**
The main settings-dialog. Contains subsections (tabs) for different modules. Use configureSettings () to invoke or raise the settings dialog

@author Thomas Friedrichsmeier
*/
class RKSettings : public QDialog {
	Q_OBJECT
public:
	enum SettingsPage { NoPage=0, Plugins=1, R=2, PHP=3, LogFiles=4, Output=5, Watch=6 };

	static void configureSettings (SettingsPage page=NoPage, QWidget *parent=0);

	static void loadSettings (KConfig *config);
	static void saveSettings (KConfig *config);
	
	void enableApply ();
public slots:
	void apply ();
	void ok ();
	void cancel ();
protected:
	RKSettings (QWidget *parent = 0, const char *name = 0);
	~RKSettings ();
private:
	void initModules ();
	void raisePage (SettingsPage page);
	static void dialogClosed ();
	
	QTabWidget *tabs;
	
	typedef QValueList<RKSettingsModule *> ModuleList;
	ModuleList modules;
	
	QPushButton *okbutton;
	QPushButton *applybutton;
	QPushButton *cancelbutton;
	
	static RKSettings *settings_dialog;
};

#endif
