/***************************************************************************
                          rksettingsmoduler  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004-2022 by Thomas Friedrichsmeier
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
#ifndef RKSETTINGSMODULER_H
#define RKSETTINGSMODULER_H

#include "rksettingsmodule.h"

#include <qstring.h>
#include <qstringlist.h>

class QComboBox;
class QLineEdit;
class MultiStringSelector;
class QSpinBox;
class QTextEdit;

/**
Configure the R-backend

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleR : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleR (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleR ();

	void applyChanges () override;
	void save (KConfig *config) override;

	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	QString caption () override;
	
/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();

/** retrieve the (probable) base url of help pages. May change across R sessions */
	static QString helpBaseUrl () { return help_base_url; };

	static int getDefaultWidth () { return options_width; };
public slots:
	void settingChanged ();
private slots:
	void addPaths (QStringList *string_list);
private:
	QLineEdit *outdec_input;
	QSpinBox *width_input;
	QComboBox *warn_input;
	QSpinBox *warningslength_input;
	QSpinBox *maxprint_input;
	QComboBox *keepsource_input;
	QComboBox *keepsourcepkgs_input;
	QSpinBox *expressions_input;
	QSpinBox *digits_input;
	QComboBox *checkbounds_input;
	QComboBox *editor_input;
	QComboBox *pager_input;
	QTextEdit *further_input;
	MultiStringSelector *addpaths_selector;

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
	static QString options_editor;
	static QString options_pager;
	static QString options_further;
	static QStringList options_addpaths;

// constants
	static QString builtin_editor;

// session constants
	friend class RInterface;
	static QString help_base_url;
};

#include "../rbackend/rcommandreceiver.h"

/**
Configure packages and library paths

@author Thomas Friedrichsmeier
*/
class RKSettingsModuleRPackages : public RKSettingsModule, public RCommandReceiver {
	Q_OBJECT
public:
	RKSettingsModuleRPackages (RKSettings *gui, QWidget *parent);
	~RKSettingsModuleRPackages ();

	void applyChanges () override;
	void save (KConfig *config) override;

	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*);

/** generate the commands needed to set the R run time options */
	static QStringList makeRRunTimeOptionCommands ();

	static bool archivePackages () { return archive_packages; }
	static QString pkgTypeOption ();
	static void addLibraryLocation (const QString& new_loc, RCommandChain *chain);
	static QStringList libraryLocations ();
/** Add a reasonable user writable path to the given library locations. */
	static QStringList addUserLibLocTo (const QStringList &liblocs);
	static QString userLibraryLocation ();

/** returns the list of packages which are essential to rkward. This is hard-coded. */
	static QStringList essentialPackages () { return essential_packages.split ("\n"); };

	QString caption () override;
public slots:
	void settingChanged ();
	void addLibLoc (QStringList *string_list);
	void addRepository (QStringList *string_list);
	void selectCRANMirror ();
protected:
	void rCommandDone (RCommand *command) override;
private:
	static QString libLocsCommand ();

	MultiStringSelector *libloc_selector;
	MultiStringSelector *repository_selector;
	QLineEdit* cran_mirror_input;

	static QString cran_mirror_url;
	static QStringList liblocs;
	static RKConfigValue<bool> archive_packages;
	static RKConfigValue<bool> source_packages;
	static QStringList package_repositories;

	friend class RInterface;
	static QString r_libs_user;
	static QStringList defaultliblocs;
	static QString essential_packages;
};

#endif
