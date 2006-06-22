/***************************************************************************
                          rksettingsmoduler  -  description
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
#ifndef RKSETTINGSMODULER_H
#define RKSETTINGSMODULER_H

#include "rksettingsmodule.h"

#include <qstring.h>
#include <qstringlist.h>

class QCheckBox;
class QComboBox;
class QLineEdit;
class MultiStringSelector;
class KIntSpinBox;

/**
Configure the R-backend

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleR : public RKSettingsModule {
	Q_OBJECT
public:
    RKSettingsModuleR (RKSettings *gui, QWidget *parent);

    ~RKSettingsModuleR ();
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();
	
	static bool archivePackages () { return archive_packages; }
//	static QStringList getPackageRepositories () { return package_repositories; };

/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();
public slots:
	void boxChanged (int);
	void pathChanged ();
	void textChanged (const QString &);
	void addRepository (QStringList *string_list);
private:
	QCheckBox *archive_packages_box;
	MultiStringSelector *repository_selector;
	QLineEdit *outdec_input;
	KIntSpinBox *width_input;
	QComboBox *warn_input;
	KIntSpinBox *warningslength_input;
	QComboBox *keepsource_input;
	QComboBox *keepsourcepkgs_input;
	KIntSpinBox *expressions_input;
	KIntSpinBox *digits_input;
	QComboBox *checkbounds_input;

	static bool archive_packages;
	static QStringList package_repositories;
	static QString options_outdec;
	static int options_width;
	static int options_warn;
	static int options_warningslength;
	static bool options_keepsource;
	static bool options_keepsourcepkgs;
	static int options_expressions;
	static int options_digits;
	static bool options_checkbounds;
};

/**
Configure packages and library paths

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleRPackages : public RKSettingsModule {
	Q_OBJECT
public:
    RKSettingsModuleRPackages (RKSettings *gui, QWidget *parent);

    ~RKSettingsModuleRPackages ();
	
	bool hasChanges ();
	void applyChanges ();
	void save (KConfig *config);
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();
public slots:
	void listChanged ();
	void addLibLoc (QStringList *string_list);
private:
	MultiStringSelector *libloc_selector;

	static QStringList liblocs;
};

#endif
