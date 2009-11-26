/***************************************************************************
                          rksettingsmoduler  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004, 2007, 2009 by Thomas Friedrichsmeier
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
	
/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();

/** retrieve the (probable) base url of help pages. May change across R sessions */
	static QString helpBaseUrl () { return help_base_url; };

	static int getDefaultWidth () { return options_width; };
public slots:
	void boxChanged (int);
	void pathChanged ();
	void textChanged (const QString &);
private:
	QLineEdit *outdec_input;
	KIntSpinBox *width_input;
	QComboBox *warn_input;
	KIntSpinBox *warningslength_input;
	KIntSpinBox *maxprint_input;
	QComboBox *keepsource_input;
	QComboBox *keepsourcepkgs_input;
	KIntSpinBox *expressions_input;
	KIntSpinBox *digits_input;
	QComboBox *checkbounds_input;
	QLineEdit *printcmd_input;
	QComboBox *editor_input;
	QComboBox *pager_input;

	static QString options_outdec;
	static int options_width;
	static int options_warn;
	static int options_warningslength;
	static int options_maxprint;
	static bool options_keepsource;
	static bool options_keepsourcepkgs;
	static int options_expressions;
	static int options_digits;
	static bool options_checkbounds;
	static QString options_printcmd;
	static QString options_editor;
	static QString options_pager;

// constants
	static QString builtin_editor;

// session constants
	friend class RThread;
	static QString help_base_url;
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

/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();

	static bool archivePackages () { return archive_packages; }
//	static QStringList getPackageRepositories () { return package_repositories; };

/** returns the list of packages which are essential to rkward. This is hard-coded. */
	static QStringList essentialPackages () { return essential_packages.split ("\n"); };

	QString caption ();
public slots:
	void listChanged ();
	void boxChanged (int);
	void addLibLoc (QStringList *string_list);
	void addRepository (QStringList *string_list);
private:
	MultiStringSelector *libloc_selector;
	QCheckBox *archive_packages_box;
	MultiStringSelector *repository_selector;
	QComboBox *cran_mirrors;
	QStringList cran_mirror_list;

	static QStringList cran_url_list;
	static int cran_mirror_index;
	static QString cran_mirror_url;
	static QStringList liblocs;
	static bool archive_packages;
	static QStringList package_repositories;

	friend class RThread;
	static QStringList defaultliblocs;
	static QString essential_packages;
};

#endif
