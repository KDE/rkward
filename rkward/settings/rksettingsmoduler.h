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
	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	QString caption() const override;
	
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
	QComboBox *editor_input;
	QComboBox *pager_input;
	QTextEdit *further_input;
	MultiStringSelector *addpaths_selector;

	static RKConfigValue<QString> options_outdec;
	static RKConfigValue<int> options_width;
	static RKConfigValue<int> options_warn;
	static RKConfigValue<int> options_warningslength;
	static RKConfigValue<int> options_maxprint;
	static RKConfigValue<bool> options_keepsource;
	static RKConfigValue<bool> options_keepsourcepkgs;
	static RKConfigValue<int> options_expressions;
	static RKConfigValue<int> options_digits;
	static RKConfigValue<bool> options_checkbounds;
	static RKConfigValue<QString> options_editor;
	static RKConfigValue<QString> options_pager;
	static RKConfigValue<QString> options_further;
	static RKConfigValue<QStringList> options_addpaths;

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
	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);
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

	QString caption() const override;
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

	static RKConfigValue<QString> cran_mirror_url;
	static RKConfigValue<QStringList> liblocs;
	static RKConfigValue<bool> archive_packages;
	static RKConfigValue<bool> source_packages;
	static RKConfigValue<QStringList> package_repositories;

	friend class RInterface;
	static QString r_libs_user;
	static QStringList defaultliblocs;
	static QString essential_packages;
};

#endif
