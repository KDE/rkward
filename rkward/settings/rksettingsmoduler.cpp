/***************************************************************************
                          rksettingsmoduler  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004-2013 by Thomas Friedrichsmeier
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

#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QTextEdit>

#include "rksettingsmodulegeneral.h"
#include "../core/robject.h"
#include "../misc/multistringselector.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkcommonfunctions.h"
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
QString RKSettingsModuleR::options_editor;
QString RKSettingsModuleR::options_pager;
QString RKSettingsModuleR::options_further;
bool RKSettingsModuleR::options_internet2;
// static constants
QString RKSettingsModuleR::builtin_editor = "<rkward>";
// session constants
QString RKSettingsModuleR::help_base_url;

RKSettingsModuleR::RKSettingsModuleR (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	QLabel *label = new QLabel (i18n ("The following settings mostly affect R behavior in the console. It is generally safe to keep these unchanged."), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);

	QGridLayout *grid = new QGridLayout ();
	main_vbox->addLayout (grid);
	int row = -1;

	// options (warn)
	grid->addWidget (new QLabel (i18n ("Display warnings"), this), ++row, 0);
	warn_input = new QComboBox (this);
	warn_input->setEditable (false);
	warn_input->insertItem (0, i18n ("Suppress warnings"));			// do not change the order of options! See also: applyChanges ()
	warn_input->insertItem (1, i18n ("Print warnings later (default)"));
	warn_input->insertItem (2, i18n ("Print warnings immediately"));
	warn_input->insertItem (3, i18n ("Convert warnings to errors"));
	warn_input->setCurrentIndex (options_warn + 1);
	connect (warn_input, SIGNAL (activated (int)), this, SLOT (settingChanged()));
	grid->addWidget (warn_input, row, 1);

	// options (OutDec)
	grid->addWidget (new QLabel (i18n ("Decimal character (only for printing)"), this), ++row, 0);
	outdec_input = new QLineEdit (options_outdec, this);
	outdec_input->setMaxLength (1);
	connect (outdec_input, SIGNAL (textChanged (const QString &)), this, SLOT (settingChanged()));
	grid->addWidget (outdec_input, row, 1);

	// options (width)
	grid->addWidget (new QLabel (i18n ("Output width (characters)"), this), ++row, 0);
	width_input = new KIntSpinBox (10, 10000, 1, options_width, this);
	connect (width_input, SIGNAL (valueChanged (int)), this, SLOT (settingChanged()));
	grid->addWidget (width_input, row, 1);

	// options (max.print)
	grid->addWidget (new QLabel (i18n ("Maximum number of elements shown in print"), this), ++row, 0);
	maxprint_input = new KIntSpinBox (100, INT_MAX, 1, options_maxprint, this);
	connect (maxprint_input, SIGNAL (valueChanged (int)), this, SLOT (settingChanged()));
	grid->addWidget (maxprint_input, row, 1);

	// options (warnings.length)
	grid->addWidget (new QLabel (i18n ("Maximum length of warnings/errors to print"), this), ++row, 0);
	warningslength_input = new KIntSpinBox (100, 8192, 1, options_warningslength, this);
	connect (warningslength_input, SIGNAL (valueChanged (int)), this, SLOT (settingChanged()));
	grid->addWidget (warningslength_input, row, 1);

	// options (keep.source)
	grid->addWidget (new QLabel (i18n ("Keep comments in functions"), this), ++row, 0);
	keepsource_input = new QComboBox (this);
	keepsource_input->setEditable (false);
	keepsource_input->addItem (i18n ("TRUE (default)"), true);
	keepsource_input->addItem (i18n ("FALSE"), false);
	keepsource_input->setCurrentIndex (options_keepsource ? 0 : 1);
	connect (keepsource_input, SIGNAL (activated (int)), this, SLOT (settingChanged()));
	grid->addWidget (keepsource_input, row, 1);

	// options (keep.source.pkgs)
	grid->addWidget (new QLabel (i18n ("Keep comments in packages"), this), ++row, 0);
	keepsourcepkgs_input = new QComboBox (this);
	keepsourcepkgs_input->setEditable (false);
	keepsourcepkgs_input->addItem (i18n ("TRUE"), true);
	keepsourcepkgs_input->addItem (i18n ("FALSE (default)"), false);
	keepsourcepkgs_input->setCurrentIndex (options_keepsourcepkgs ? 0 : 1);
	connect (keepsourcepkgs_input, SIGNAL (activated (int)), this, SLOT (settingChanged()));
	grid->addWidget (keepsourcepkgs_input, row, 1);

	// options (expressions)
	grid->addWidget (new QLabel (i18n ("Maximum level of nested expressions"), this), ++row, 0);
	expressions_input = new KIntSpinBox (25, 500000, 1, options_expressions, this);
	connect (expressions_input, SIGNAL (valueChanged (int)), this, SLOT (settingChanged()));
	grid->addWidget (expressions_input, row, 1);

	// options (digits)
	grid->addWidget (new QLabel (i18n ("Default decimal precision in print ()"), this), ++row, 0);
	digits_input = new KIntSpinBox (1, 22, 1, options_digits, this);
	connect (digits_input, SIGNAL (valueChanged (int)), this, SLOT (settingChanged()));
	grid->addWidget (digits_input, row, 1);

	// options (check.bounds)
	grid->addWidget (new QLabel (i18n ("Check vector bounds (warn)"), this), ++row, 0);
	checkbounds_input = new QComboBox (this);
	checkbounds_input->setEditable (false);
	checkbounds_input->addItem (i18n ("TRUE"), true);
	checkbounds_input->addItem (i18n ("FALSE (default)"), false);
	checkbounds_input->setCurrentIndex (options_checkbounds ? 0 : 1);
	connect (checkbounds_input, SIGNAL (activated (int)), this, SLOT (settingChanged()));
	grid->addWidget (checkbounds_input, row, 1);

	grid->addWidget (new QLabel (i18n ("Editor command"), this), ++row, 0);
	editor_input = new QComboBox (this);
	editor_input->setEditable (true);
	editor_input->addItem (builtin_editor);
	if (options_editor != builtin_editor) {
		editor_input->addItem (options_editor);
		editor_input->setCurrentIndex (1);
	}
	connect (editor_input, SIGNAL (editTextChanged (const QString &)), this, SLOT (settingChanged()));
	grid->addWidget (editor_input, row, 1);

	grid->addWidget (new QLabel (i18n ("Pager command"), this), ++row, 0);
	pager_input = new QComboBox (this);
	pager_input->setEditable (true);
	pager_input->addItem (builtin_editor);
	if (options_pager != builtin_editor) {
		pager_input->addItem (options_pager);
		pager_input->setCurrentIndex (1);
	}
	connect (pager_input, SIGNAL (editTextChanged (const QString &)), this, SLOT (settingChanged()));
	grid->addWidget (pager_input, row, 1);

#ifdef Q_WS_WIN
	grid->addWidget (label = new QLabel (i18n ("Use Internet Explorer functions for internet access"), this), ++row, 0);
	internet2_input = new QCheckBox (this);
	internet2_input->setChecked (options_internet2);
	connect (internet2_input, SIGNAL (stateChanged(int)), this, SLOT (settingChanged()));
	grid->addWidget (internet2_input, row, 1);
	RKCommonFunctions::setTips (i18n ("<p>Use Internet Explorer functions for accessing the internet from R. "
									"Enabling this option may help in case of problems with accessing the internet from R (e.g. for "
									"installing packages).</p>"), internet2_input, label);
#endif

	grid->addWidget (new QLabel (i18n ("Further (option) commands to run in each session"), this), ++row, 0, 1, 2);
	further_input = new QTextEdit (this);
	further_input->setWordWrapMode (QTextOption::NoWrap);
	further_input->setAcceptRichText (false);
	further_input->setPlainText (options_further);
	connect (further_input, SIGNAL (textChanged()), this, SLOT (settingChanged()));
	grid->addWidget (further_input, ++row, 0, 1, 2);

	main_vbox->addStretch ();
}

RKSettingsModuleR::~RKSettingsModuleR() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleR::settingChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModuleR::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("R-Backend"));
}

void RKSettingsModuleR::applyChanges () {
	RK_TRACE (SETTINGS);

	options_outdec = outdec_input->text ();
	options_width = width_input->value ();
	options_warn = warn_input->currentIndex () - 1;
	options_warningslength = warningslength_input->value ();
	options_maxprint = maxprint_input->value ();
	options_keepsource = keepsource_input->itemData (keepsource_input->currentIndex ()).toBool ();
	options_keepsourcepkgs = keepsourcepkgs_input->itemData (keepsourcepkgs_input->currentIndex ()).toBool ();
	options_expressions = expressions_input->value ();
	options_digits = digits_input->value ();
	options_checkbounds = checkbounds_input->itemData (checkbounds_input->currentIndex ()).toBool ();
	options_editor = editor_input->currentText ();
	options_pager = pager_input->currentText ();
	options_further = further_input->toPlainText ();
#ifdef Q_WS_WIN
	options_internet2 = internet2_input->isChecked ();
#endif

// apply run time options in R
	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString::null, 0, 0, commandChain ());
	}
}

//static
QStringList RKSettingsModuleR::makeRRunTimeOptionCommands () {
	RK_TRACE (SETTINGS);
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
	if (options_editor == builtin_editor) list.append ("options (editor=rk.edit.files)\n");
	else list.append ("options (editor=\"" + options_editor + "\")\n");
	if (options_pager == builtin_editor) list.append ("options (pager=rk.show.files)\n");
	else list.append ("options (pager=\"" + options_pager + "\")\n");
	if (!options_further.isEmpty ()) list.append (options_further + "\n");
#ifdef Q_WS_WIN
	list.append (QString ("setInternet2 (") + (options_internet2 ? "TRUE)\n" : "FALSE)\n"));
#endif

#warning TODO make the following options configurable
	list.append ("options (device=\"rk.screen.device\")\n");
	// register as interactive
	list.append ("try (deviceIsInteractive(name=\"rk.screen.device\"))\n");
	list.append ("options (help_type=\"html\")\n");		// for R 2.10.0 and above
	list.append ("try ({options (htmlhelp=TRUE); options (chmhelp=FALSE)})\n");	// COMPAT: for R 2.9.x and below
	list.append ("options (browser=rk.show.html)\n");

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
	cg.writeEntry ("editor", options_editor);
	cg.writeEntry ("pager", options_pager);
	cg.writeEntry ("further init commands", options_further);
#ifdef Q_WS_WIN
	cg.writeEntry ("internet2", options_internet2);
#endif
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
	options_editor = cg.readEntry ("editor", builtin_editor);
	options_pager = cg.readEntry ("pager", builtin_editor);
	options_further = cg.readEntry ("further init commands", QString ());
#ifdef Q_WS_WIN
	options_internet2 = cg.readEntry ("internet2", false);
#endif
}

//#################################################
//############### RKSettingsModuleRPackages ################
//#################################################

// static members
QStringList RKSettingsModuleRPackages::liblocs;
QStringList RKSettingsModuleRPackages::defaultliblocs;
bool RKSettingsModuleRPackages::archive_packages;
bool RKSettingsModuleRPackages::source_packages;
QStringList RKSettingsModuleRPackages::package_repositories;
QString RKSettingsModuleRPackages::essential_packages = QString ("base\nmethods\nutils\ngrDevices\ngraphics\nrkward");
QString RKSettingsModuleRPackages::cran_mirror_url;

RKSettingsModuleRPackages::RKSettingsModuleRPackages (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent), RCommandReceiver () {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	main_vbox->addWidget (new QLabel (i18n ("CRAN download mirror (leave empty to be prompted once each session):"), this));
	QHBoxLayout* hbox = new QHBoxLayout ();
	main_vbox->addLayout (hbox);
	cran_mirror_input = new QLineEdit (cran_mirror_url, this);
	if (cran_mirror_url == "@CRAN@") cran_mirror_input->clear ();
	connect (cran_mirror_input, SIGNAL (textChanged(const QString&)), this, SLOT (settingChanged()));
	hbox->addWidget (cran_mirror_input);
	QPushButton* cran_mirror_button = new QPushButton (i18n ("Select mirror"), this);
	connect (cran_mirror_button, SIGNAL (clicked()), this, SLOT (selectCRANMirror()));
	hbox->addWidget (cran_mirror_button);

	repository_selector = new MultiStringSelector (i18n ("Additional package repositories (where libraries are downloaded from)"), this);
	repository_selector->setValues (package_repositories);
	connect (repository_selector, SIGNAL (listChanged ()), this, SLOT (settingChanged ()));
	connect (repository_selector, SIGNAL (getNewStrings (QStringList*)), this, SLOT (addRepository (QStringList*)));
	main_vbox->addWidget (repository_selector);

	archive_packages_box = new QCheckBox (i18n ("Archive downloaded packages"), this);
	archive_packages_box->setChecked (archive_packages);
	connect (archive_packages_box, SIGNAL (stateChanged (int)), this, SLOT (settingChanged()));
	main_vbox->addWidget (archive_packages_box);

#if defined Q_WS_WIN || defined Q_WS_MAC
	source_packages_box = new QCheckBox (i18n ("Build packages from source"), this);
	source_packages_box->setChecked (source_packages);
#else
	source_packages_box = new QCheckBox (i18n ("Build packages from source (not configurable on this platform)"), this);
	source_packages_box->setChecked (true);
	source_packages_box->setEnabled (false);
#endif
	RKCommonFunctions::setTips (QString ("<p>%1</p>").arg (i18n ("Installing packages from pre-compiled binaries (if available) is generally faster, and does not require an installation of development tools and libraries. On the other hand, building packages from source provides best compatibility. On Mac OS X and Linux, building packages from source is currently recommended.")), source_packages_box);
	connect (source_packages_box, SIGNAL (stateChanged (int)), this, SLOT (settingChanged()));
	main_vbox->addWidget (source_packages_box);

	main_vbox->addStretch ();

	libloc_selector = new MultiStringSelector (i18n ("R Library locations (where libraries get installed to, locally)"), this);
	libloc_selector->setValues (liblocs);
	connect (libloc_selector, SIGNAL (listChanged ()), this, SLOT (settingChanged ()));
	connect (libloc_selector, SIGNAL (getNewStrings (QStringList*)), this, SLOT (addLibLoc (QStringList*)));
	main_vbox->addWidget (libloc_selector);
	QLabel *label = new QLabel (i18n ("Note: The startup defaults will always be used in addition to the locations you specify in this list"), this);
	main_vbox->addWidget (label);

	main_vbox->addStretch ();
}

RKSettingsModuleRPackages::~RKSettingsModuleRPackages () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleRPackages::addLibraryLocation (const QString& new_loc, RCommandChain *chain) {
	RK_TRACE (SETTINGS);

	if (!libraryLocations ().contains (new_loc)) liblocs.prepend (new_loc);

	// update the backend in any case. User might have changed liblocs, there.
	RKGlobals::rInterface ()->issueCommand (".libPaths (unique (c (" + RObject::rQuote (new_loc) + ", .libPaths ())))", RCommand::App | RCommand::Sync, QString (), 0, 0, chain);
}

void RKSettingsModuleRPackages::settingChanged () {
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
	QString new_string = KInputDialog::getText (i18n ("Add repository"), i18n ("Add URL of new repository"), QString::null, 0, this);
	(*string_list).append (new_string);
}

QString RKSettingsModuleRPackages::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("R-Packages"));
}

#define SELECT_CRAN_MIRROR_COMMAND 123
void RKSettingsModuleRPackages::selectCRANMirror () {
	RK_TRACE (SETTINGS);
	QString title = i18n ("Select CRAN mirror");
	
	RCommand* command = new RCommand ("rk.select.CRAN.mirror()\n", RCommand::App | RCommand::GetStringVector, title, this, SELECT_CRAN_MIRROR_COMMAND);

	RKProgressControl* control = new RKProgressControl (this, title, title, RKProgressControl::CancellableProgress);
	control->addRCommand (command, true);
	RKGlobals::rInterface ()->issueCommand (command, commandChain ());
	control->doModal (true);
}

void RKSettingsModuleRPackages::rCommandDone (RCommand *command) {
	RK_TRACE (SETTINGS);

	if (command->getFlags () == SELECT_CRAN_MIRROR_COMMAND) {
		if (command->succeeded ()) {
			RK_ASSERT (command->getDataLength () >= 1);
			cran_mirror_input->setText (command->stringVector ().value (0));
		}
	} else {
		RK_ASSERT (false);
	}
}

QString RKSettingsModuleRPackages::libLocsCommand () {
	RK_TRACE (SETTINGS);

	QString command = ".libPaths (unique (c (";
	bool first = true;
	QStringList ll = libraryLocations ();
	foreach (const QString libloc, ll) {
		if (first) first = false;
		else command.append (", ");
		command.append (RObject::rQuote (libloc));
	}
	command.append (")))");

	return command;
}

//static
QString RKSettingsModuleRPackages::pkgTypeOption () {
	QString ret;
#if defined Q_WS_WIN || defined Q_WS_MAC
	ret.append ("options (pkgType=\"");
	if (source_packages) ret.append ("source");
#	if defined Q_WS_WIN
	else ret.append ("win.binary");
#	else
	else ret.append ("mac.binary.leopard");
#	endif
	ret.append ("\")\n");
#endif
	return ret;
}

//static
QStringList RKSettingsModuleRPackages::makeRRunTimeOptionCommands () {
	RK_TRACE (SETTINGS);
	QStringList list;

// package repositories
	QString command = "options (repos=c (CRAN=" + RObject::rQuote (cran_mirror_url);
	for (QStringList::const_iterator it = package_repositories.begin (); it != package_repositories.end (); ++it) {
		command.append (", " + RObject::rQuote (*it));
	}
	list.append (command + "))\n");

#if defined Q_WS_WIN || defined Q_WS_MAC
	list.append (pkgTypeOption ());
#endif

// library locations
	list.append (libLocsCommand ());

	return list;
}

void RKSettingsModuleRPackages::applyChanges () {
	RK_TRACE (SETTINGS);

	cran_mirror_url = cran_mirror_input->text ();
	if (cran_mirror_url.isEmpty ()) cran_mirror_url = "@CRAN@";

	archive_packages = archive_packages_box->isChecked ();
	source_packages = source_packages_box->isChecked ();
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
	cg.writeEntry ("CRAN mirror url", cran_mirror_url);
	cg.writeEntry ("archive packages", archive_packages);
	cg.writeEntry ("source_packages", source_packages);
	cg.writeEntry ("Repositories", package_repositories);
	cg.writeEntry ("LibraryLocations", liblocs);
}

void RKSettingsModuleRPackages::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("R Settings");

	cran_mirror_url = cg.readEntry ("CRAN mirror url", "@CRAN@");
	const QString rkward_repo ("http://rkward.sf.net/R/");
	package_repositories = cg.readEntry ("Repositories", QStringList (rkward_repo));
	if (RKSettingsModuleGeneral::storedConfigVersion () <= RKSettingsModuleGeneral::RKWardConfig_Pre0_5_7) {
		package_repositories.removeAll ("@CRAN@");	// COMPAT: Cran mirror was part of this list before 0.5.3
		package_repositories.append (rkward_repo);
	}

	liblocs = cg.readEntry ("LibraryLocations", QStringList ());
	archive_packages = cg.readEntry ("archive packages", false);
#if defined Q_WS_WIN || defined Q_WS_MAC
#	if defined USE_BINARY_PACKAGES
#		define USE_SOURCE_PACKAGES false
#	else
#		define USE_SOURCE_PACKAGES true
#endif
	source_packages = cg.readEntry ("source packages", USE_SOURCE_PACKAGES);
	if (USE_SOURCE_PACKAGES && (RKSettingsModuleGeneral::storedConfigVersion () < RKSettingsModuleGeneral::RKWardConfig_0_6_1)) {
		// revert default on MacOSX, even if a previous stored setting exists
		source_packages = true;
	}
#else
	source_packages = true;
#endif
}

#include "rksettingsmoduler.moc"
