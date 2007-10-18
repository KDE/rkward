/***************************************************************************
                          rksettingsmoduler  -  description
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
#include "rksettingsmoduler.h"

#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kinputdialog.h>
#include <knuminput.h>
#include <kfiledialog.h>

#include <qlayout.h>
#include <qlabel.h>
#include <Q3VButtonGroup>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "../misc/multistringselector.h"
#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
QString RKSettingsModuleR::options_outdec;
int RKSettingsModuleR::options_width;
int RKSettingsModuleR::options_warn;
int RKSettingsModuleR::options_warningslength;
int RKSettingsModuleR::options_maxprint;
bool RKSettingsModuleR::options_keepsource;
bool RKSettingsModuleR::options_keepsourcepkgs;
int RKSettingsModuleR::options_expressions;
int RKSettingsModuleR::options_digits;
bool RKSettingsModuleR::options_checkbounds;
QString RKSettingsModuleR::options_printcmd;

RKSettingsModuleR::RKSettingsModuleR (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	Q3VBoxLayout *main_vbox = new Q3VBoxLayout (this, RKGlobals::marginHint ());

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	QLabel *label = new QLabel (i18n ("The following settings mostly affect R behavior in the console. It is generally safe to keep these unchanged."), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);

	Q3GridLayout *grid = new Q3GridLayout (main_vbox, 1, 2, RKGlobals::spacingHint ());
	int row = -1;

	// options (warn)
	grid->addWidget (new QLabel (i18n ("Display warnings"), this), ++row, 0);
	warn_input = new QComboBox (false, this);
	warn_input->insertItem (i18n ("Suppress warnings"));			// do not change the order of options! See also: applyChanges ()
	warn_input->insertItem (i18n ("Print warnings later (default)"));
	warn_input->insertItem (i18n ("Print warnings immediately"));
	warn_input->insertItem (i18n ("Convert warnings to errors"));
	warn_input->setCurrentItem (options_warn + 1);
	connect (warn_input, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (warn_input, row, 1);

	// options (OutDec)
	grid->addWidget (new QLabel (i18n ("Decimal character (only for printing)"), this), ++row, 0);
	outdec_input = new QLineEdit (options_outdec, this);
	outdec_input->setMaxLength (1);
	connect (outdec_input, SIGNAL (textChanged (const QString &)), this, SLOT (textChanged (const QString &)));
	grid->addWidget (outdec_input, row, 1);

	// options (width)
	grid->addWidget (new QLabel (i18n ("Output width (characters)"), this), ++row, 0);
	width_input = new KIntSpinBox (10, 10000, 1, options_width, this);
	connect (width_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (width_input, row, 1);

	// options (max.print)
	grid->addWidget (new QLabel (i18n ("Maximum number of elements shown in print"), this), ++row, 0);
	maxprint_input = new KIntSpinBox (100, INT_MAX, 1, options_maxprint, this);
	connect (maxprint_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (maxprint_input, row, 1);

	// options (warnings.length)
	grid->addWidget (new QLabel (i18n ("Maximum length of warnings/errors to print"), this), ++row, 0);
	warningslength_input = new KIntSpinBox (100, 8192, 1, options_warningslength, this);
	connect (warningslength_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (warningslength_input, row, 1);

	// options (keep.source)
	grid->addWidget (new QLabel (i18n ("Keep comments in functions"), this), ++row, 0);
	keepsource_input = new QComboBox (false, this);
	keepsource_input->insertItem (i18n ("TRUE (default)"));			// do not change the order of options! See also: applyChanges ()
	keepsource_input->insertItem (i18n ("FALSE"));
	keepsource_input->setCurrentItem (options_keepsource ? 0 : 1);
	connect (keepsource_input, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (keepsource_input, row, 1);

	// options (keep.source.pkgs)
	grid->addWidget (new QLabel (i18n ("Keep comments in packages"), this), ++row, 0);
	keepsourcepkgs_input = new QComboBox (false, this);
	keepsourcepkgs_input->insertItem (i18n ("TRUE)"));			// do not change the order of options! See also: applyChanges ()
	keepsourcepkgs_input->insertItem (i18n ("FALSE (default)"));
	keepsourcepkgs_input->setCurrentItem (options_keepsourcepkgs ? 0 : 1);
	connect (keepsourcepkgs_input, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (keepsourcepkgs_input, row, 1);

	// options (expressions)
	grid->addWidget (new QLabel (i18n ("Maximum level of nested expressions"), this), ++row, 0);
	expressions_input = new KIntSpinBox (25, 500000, 1, options_expressions, this);
	connect (expressions_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (expressions_input, row, 1);

	// options (digits)
	grid->addWidget (new QLabel (i18n ("Default decimal precision in print ()"), this), ++row, 0);
	digits_input = new KIntSpinBox (1, 22, 1, options_digits, this);
	connect (digits_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (digits_input, row, 1);

	// options (check.bounds)
	grid->addWidget (new QLabel (i18n ("Check vector bounds (warn)"), this), ++row, 0);
	checkbounds_input = new QComboBox (false, this);
	checkbounds_input->insertItem (i18n ("TRUE)"));			// do not change the order of options! See also: applyChanges ()
	checkbounds_input->insertItem (i18n ("FALSE (default)"));
	checkbounds_input->setCurrentItem (options_checkbounds ? 0 : 1);
	connect (checkbounds_input, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (checkbounds_input, row, 1);

	grid->addWidget (new QLabel (i18n ("Command used to send files to printer"), this), ++row, 0);
	printcmd_input = new QLineEdit (options_printcmd, this);
	connect (printcmd_input, SIGNAL (textChanged (const QString &)), this, SLOT (textChanged (const QString &)));
	grid->addWidget (printcmd_input, row, 1);

	main_vbox->addStretch ();
}

RKSettingsModuleR::~RKSettingsModuleR() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleR::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleR::pathChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleR::textChanged (const QString &) {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModuleR::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("R-Backend"));
}

bool RKSettingsModuleR::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleR::applyChanges () {
	RK_TRACE (SETTINGS);

	options_outdec = outdec_input->text ();
	options_width = width_input->value ();
	options_warn = warn_input->currentItem () - 1;
	options_warningslength = warningslength_input->value ();
	options_maxprint = maxprint_input->value ();
	options_keepsource = (keepsource_input->currentItem () == 0);
	options_keepsourcepkgs = (keepsourcepkgs_input->currentItem () == 0);
	options_expressions = expressions_input->value ();
	options_digits = digits_input->value ();
	options_checkbounds = (checkbounds_input->currentItem () == 0);
	options_printcmd = printcmd_input->text ();

// apply run time options in R
	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString::null, 0, 0, commandChain ());
	}
}

//static
QStringList RKSettingsModuleR::makeRRunTimeOptionCommands () {
	QStringList list;

	QString tf;
	list.append ("options (OutDec=\"" + options_outdec.left (1) + "\")\n");
	list.append ("options (width=" + QString::number (options_width) + ")\n");
	list.append ("options (warn=" + QString::number (options_warn) + ")\n");
	list.append ("options (max.print=" + QString::number (options_maxprint) + ")\n");
	list.append ("options (warnings.length=" + QString::number (options_warningslength) + ")\n");
	if (options_keepsource) tf = "TRUE"; else tf = "FALSE";
	list.append ("options (keep.source=" + tf + ")\n");
	if (options_keepsourcepkgs) tf = "TRUE"; else tf = "FALSE";
	list.append ("options (keep.source.pkgs=" + tf + ")\n");
	list.append ("options (expressions=" + QString::number (options_expressions) + ")\n");
	list.append ("options (digits=" + QString::number (options_digits) + ")\n");
	if (options_checkbounds) tf = "TRUE"; else tf = "FALSE";
	list.append ("options (checkbounds=" + tf + ")\n");
	list.append ("options (printcmd=\"" + options_printcmd + "\")\n");

	return list;
}

void RKSettingsModuleR::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}

void RKSettingsModuleR::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("R Settings");
	cg.writeEntry ("OutDec", options_outdec);
	cg.writeEntry ("width", options_width);
	cg.writeEntry ("warn", options_warn);
	cg.writeEntry ("maxprint", options_maxprint);
	cg.writeEntry ("warnings.length", options_warningslength);
	cg.writeEntry ("keep.source", options_keepsource);
	cg.writeEntry ("keep.source.pkgs", options_keepsourcepkgs);
	cg.writeEntry ("expressions", options_expressions);
	cg.writeEntry ("digits", options_digits);
	cg.writeEntry ("check.bounds", options_checkbounds);
	cg.writeEntry ("printcmd", options_printcmd);
}

void RKSettingsModuleR::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("R Settings");
	options_outdec = cg.readEntry ("OutDec", ".");
	options_width = cg.readEntry ("width", 80);
	options_warn = cg.readEntry ("warn", 0);
	options_maxprint = cg.readEntry ("max.print", 99999);
	options_warningslength = cg.readEntry ("warnings.length", 1000);
	options_keepsource = cg.readEntry ("keep.source", true);
	options_keepsourcepkgs = cg.readEntry ("keep.source.pkgs", false);
	options_expressions = cg.readEntry ("expressions", 5000);
	options_digits = cg.readEntry ("digits", 7);
	options_checkbounds = cg.readEntry ("check.bounds", false);
	options_printcmd = cg.readEntry ("printcmd", "kprinter");
}

//#################################################
//############### RKSettingsModuleRPackages ################
//#################################################

// static members
QStringList RKSettingsModuleRPackages::liblocs;
QStringList RKSettingsModuleRPackages::defaultliblocs;
bool RKSettingsModuleRPackages::archive_packages;
QStringList RKSettingsModuleRPackages::package_repositories;

RKSettingsModuleRPackages::RKSettingsModuleRPackages (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	Q3VBoxLayout *main_vbox = new Q3VBoxLayout (this, RKGlobals::marginHint ());

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	repository_selector = new MultiStringSelector (i18n ("Package repositories (where libraries are downloaded from)"), this);
	repository_selector->setValues (package_repositories);
	connect (repository_selector, SIGNAL (listChanged ()), this, SLOT (listChanged ()));
	connect (repository_selector, SIGNAL (getNewStrings (QStringList*)), this, SLOT (addRepository (QStringList*)));
	main_vbox->addWidget (repository_selector);

	archive_packages_box = new QCheckBox (i18n ("Archive downloaded packages"), this);
	archive_packages_box->setChecked (archive_packages);
	connect (archive_packages_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (archive_packages_box);	

	main_vbox->addStretch ();

	libloc_selector = new MultiStringSelector (i18n ("R Library locations  (where libraries get installed to, locally)"), this);
	libloc_selector->setValues (liblocs);
	connect (libloc_selector, SIGNAL (listChanged ()), this, SLOT (listChanged ()));
	connect (libloc_selector, SIGNAL (getNewStrings (QStringList*)), this, SLOT (addLibLoc (QStringList*)));
	main_vbox->addWidget (libloc_selector);
	QLabel *label = new QLabel (i18n ("Note: The startup defaults will always be used in addition to the locations you specify in this list"), this);
	main_vbox->addWidget (label);

	main_vbox->addStretch ();
}

RKSettingsModuleRPackages::~RKSettingsModuleRPackages () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleRPackages::listChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleRPackages::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleRPackages::addLibLoc (QStringList *string_list) {
	RK_TRACE (SETTINGS);
	QString new_string = KFileDialog::getExistingDirectory (KUrl (), this, i18n ("Add R Library Directory"));
	if (!new_string.isEmpty ()) {
		(*string_list).append (new_string);
	}
}

void RKSettingsModuleRPackages::addRepository (QStringList *string_list) {
	RK_TRACE (SETTINGS);
	QString new_string = KInputDialog::getText (i18n ("Add repository"), i18n ("Add URL of new repository\n(Enter \"@CRAN@\" for the standard CRAN-mirror)"), QString::null, 0, this);
	(*string_list).append (new_string);
}

QString RKSettingsModuleRPackages::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("R-Packages"));
}

bool RKSettingsModuleRPackages::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

//static
QStringList RKSettingsModuleRPackages::makeRRunTimeOptionCommands () {
	QStringList list;

// package repositories
	QString command = "options (repos=c (";
	for (QStringList::const_iterator it = package_repositories.begin (); it != package_repositories.end (); ++it) {
		if (it != package_repositories.begin ()) {
			command.append (", ");
		}
		if (*it == "@CRAN@") command.append ("CRAN="); 
		command.append ("\"" + *it + "\"");
	}
	list.append (command + "))\n");

// library locations
	command = ".libPaths (unique (c (";
	bool first = true;
	for (QStringList::const_iterator it = liblocs.begin (); it != liblocs.end (); ++it) {
		if (first) first = false;
		else command.append (", ");
		command.append ("\"" + *it + "\"");
	}
	for (QStringList::const_iterator it = defaultliblocs.begin (); it != defaultliblocs.end (); ++it) {
		if (first) first = false;
		else command.append (", ");
		command.append ("\"" + *it + "\"");
	}
	command.append (")))");
	list.append (command);

	return list;
}

void RKSettingsModuleRPackages::applyChanges () {
	RK_TRACE (SETTINGS);

	archive_packages = archive_packages_box->isChecked ();
	package_repositories = repository_selector->getValues ();
	liblocs = libloc_selector->getValues ();

// apply options in R
	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString::null, 0, 0, commandChain ());
	}
}

void RKSettingsModuleRPackages::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}

void RKSettingsModuleRPackages::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("R Settings");
	cg.writeEntry ("archive packages", archive_packages);
	cg.writeEntry ("Repositories", package_repositories);
	cg.writeEntry ("LibraryLocations", liblocs);
}

void RKSettingsModuleRPackages::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("R Settings");
	archive_packages = cg.readEntry ("archive packages", false);
	package_repositories = cg.readEntry ("Repositories", QStringList ("@CRAN@"));

	liblocs = cg.readEntry ("LibraryLocations", QStringList ());
}

#include "rksettingsmoduler.moc"
