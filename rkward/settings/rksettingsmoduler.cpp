/***************************************************************************
                          rksettingsmoduler  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004-2018 by Thomas Friedrichsmeier
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
#include "rksettingsmoduler.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KUrlRequester>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QInputDialog>

#include "rksettingsmodulegeneral.h"
#include "../core/robject.h"
#include "../dialogs/rksetupwizard.h"
#include "../misc/multistringselector.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkcommonfunctions.h"
#include "../rbackend/rkrinterface.h"
#include "../rbackend/rksessionvars.h"
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
QStringList RKSettingsModuleR::options_addpaths;
// static constants
QString RKSettingsModuleR::builtin_editor = "<rkward>";
// session constants
QString RKSettingsModuleR::help_base_url;

RKSettingsModuleR::RKSettingsModuleR (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	main_vbox->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("The following settings mostly affect R behavior in the console. It is generally safe to keep these unchanged.")));

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
	connect (warn_input, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (warn_input, row, 1);

	// options (OutDec)
	grid->addWidget (new QLabel (i18n ("Decimal character (only for printing)"), this), ++row, 0);
	outdec_input = new QLineEdit (options_outdec, this);
	outdec_input->setMaxLength (1);
	connect (outdec_input, &QLineEdit::textChanged, this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (outdec_input, row, 1);

	// options (width)
	grid->addWidget (new QLabel (i18n ("Output width (characters)"), this), ++row, 0);
	width_input = new QSpinBox(this);
	width_input->setMaximum(10000);
	width_input->setMinimum(10);
	width_input->setSingleStep(1);
	width_input->setValue(options_width);
	connect (width_input, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (width_input, row, 1);

	// options (max.print)
	grid->addWidget (new QLabel (i18n ("Maximum number of elements shown in print"), this), ++row, 0);
	maxprint_input = new QSpinBox(this);
	maxprint_input->setMaximum(INT_MAX);
	maxprint_input->setMinimum(100);
	maxprint_input->setSingleStep(1);
	maxprint_input->setValue(options_maxprint);
	connect (maxprint_input, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (maxprint_input, row, 1);

	// options (warnings.length)
	grid->addWidget (new QLabel (i18n ("Maximum length of warnings/errors to print"), this), ++row, 0);
	warningslength_input = new QSpinBox(this);
	warningslength_input->setMaximum(8192);
	warningslength_input->setMinimum(100);
	warningslength_input->setSingleStep(1);
	warningslength_input->setValue(options_warningslength);
	connect (warningslength_input, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (warningslength_input, row, 1);

	// options (keep.source)
	grid->addWidget (new QLabel (i18n ("Keep comments in functions"), this), ++row, 0);
	keepsource_input = new QComboBox (this);
	keepsource_input->setEditable (false);
	keepsource_input->addItem (i18n ("TRUE (default)"), true);
	keepsource_input->addItem (i18n ("FALSE"), false);
	keepsource_input->setCurrentIndex (options_keepsource ? 0 : 1);
	connect (keepsource_input, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (keepsource_input, row, 1);

	// options (keep.source.pkgs)
	grid->addWidget (new QLabel (i18n ("Keep comments in packages"), this), ++row, 0);
	keepsourcepkgs_input = new QComboBox (this);
	keepsourcepkgs_input->setEditable (false);
	keepsourcepkgs_input->addItem (i18n ("TRUE"), true);
	keepsourcepkgs_input->addItem (i18n ("FALSE (default)"), false);
	keepsourcepkgs_input->setCurrentIndex (options_keepsourcepkgs ? 0 : 1);
	connect (keepsourcepkgs_input, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (keepsourcepkgs_input, row, 1);

	// options (expressions)
	grid->addWidget (new QLabel (i18n ("Maximum level of nested expressions"), this), ++row, 0);
	expressions_input = new QSpinBox(this);
	expressions_input->setMaximum(500000);
	expressions_input->setMinimum(25);
	expressions_input->setSingleStep(1);
	expressions_input->setValue(options_expressions);
	connect (expressions_input, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (expressions_input, row, 1);

	// options (digits)
	grid->addWidget (new QLabel (i18n ("Default decimal precision in print ()"), this), ++row, 0);
	digits_input = new QSpinBox(this);
	digits_input->setMaximum(22);
	digits_input->setMinimum(1);
	digits_input->setSingleStep(1);
	digits_input->setValue(options_digits);
	connect (digits_input, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (digits_input, row, 1);

	// options (check.bounds)
	grid->addWidget (new QLabel (i18n ("Check vector bounds (warn)"), this), ++row, 0);
	checkbounds_input = new QComboBox (this);
	checkbounds_input->setEditable (false);
	checkbounds_input->addItem (i18n ("TRUE"), true);
	checkbounds_input->addItem (i18n ("FALSE (default)"), false);
	checkbounds_input->setCurrentIndex (options_checkbounds ? 0 : 1);
	connect (checkbounds_input, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (checkbounds_input, row, 1);

	grid->addWidget (new QLabel (i18n ("Editor command"), this), ++row, 0);
	editor_input = new QComboBox (this);
	editor_input->setEditable (true);
	editor_input->addItem (builtin_editor);
	if (options_editor != builtin_editor) {
		editor_input->addItem (options_editor);
		editor_input->setCurrentIndex (1);
	}
	connect (editor_input, &QComboBox::editTextChanged, this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (editor_input, row, 1);

	grid->addWidget (new QLabel (i18n ("Pager command"), this), ++row, 0);
	pager_input = new QComboBox (this);
	pager_input->setEditable (true);
	pager_input->addItem (builtin_editor);
	if (options_pager != builtin_editor) {
		pager_input->addItem (options_pager);
		pager_input->setCurrentIndex (1);
	}
	connect (pager_input, &QComboBox::editTextChanged, this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (pager_input, row, 1);

	grid->addWidget (new QLabel (i18n ("Further (option) commands to run in each session"), this), ++row, 0, 1, 2);
	further_input = new QTextEdit (this);
	further_input->setWordWrapMode (QTextOption::NoWrap);
	further_input->setAcceptRichText (false);
	further_input->setPlainText (options_further);
	connect (further_input, &QTextEdit::textChanged, this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (further_input, ++row, 0, 1, 2);

	main_vbox->addStretch ();

	addpaths_selector = new MultiStringSelector (i18n ("Addition search paths for utilities used by R"), this);
	addpaths_selector->setValues (options_addpaths);
	connect (addpaths_selector, &MultiStringSelector::listChanged, this, &RKSettingsModuleR::settingChanged);
	connect (addpaths_selector, &MultiStringSelector::getNewStrings, this, &RKSettingsModuleR::addPaths);
	main_vbox->addWidget (addpaths_selector);
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
	// normalize system paths before adding
	QStringList paths = addpaths_selector->getValues ();
	options_addpaths.clear ();
	for (int i = 0; i < paths.count (); ++i) {
		QString path = QDir::cleanPath (paths[i]);
		if (!options_addpaths.contains (path)) options_addpaths.append (path);
	}

// apply run time options in R
	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString (), 0, 0, commandChain ());
	}
}

void RKSettingsModuleR::addPaths(QStringList* string_list) {
	RK_TRACE (SETTINGS);

	QDialog dialog (this);
	dialog.setWindowTitle (i18n ("Add System Path Directory"));
	QVBoxLayout *layout = new QVBoxLayout (&dialog);
	layout->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("Specify or select directory to add to the system file path of the running R session")));

	KUrlRequester *req = new KUrlRequester ();
	req->setMode (KFile::Directory);
	layout->addWidget (req);

	QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttons->button(QDialogButtonBox::Ok)->setText (i18nc ("Add directory to list", "Add"));
	connect (buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect (buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	layout->addWidget (buttons);

	if (dialog.exec () == QDialog::Accepted) {
		if (!req->text ().isEmpty ()) (*string_list).append (req->text ());
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
	if (!options_further.isEmpty ()) list.append (options_further + '\n');
	if (!options_addpaths.isEmpty ()) {
		QString command = "rk.adjust.system.path (add=c(";
		foreach (const QString &p, options_addpaths) {
			command.append (RObject::rQuote (p));
		}
		list.append (command + "))\n");
	}

#ifdef __GNUC__
#	warning TODO make the following options configurable
#endif
	list.append ("options (help_type=\"html\")\n");		// for R 2.10.0 and above
	list.append ("try ({options (htmlhelp=TRUE); options (chmhelp=FALSE)})\n");	// COMPAT: for R 2.9.x and below
	list.append ("options (browser=rk.show.html)\n");
	list.append ("options (askYesNo=rk.askYesNo)\n"); // for R 3.5.0 and above

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
	cg.writeEntry ("max.print", options_maxprint);
	cg.writeEntry ("warnings.length", options_warningslength);
	cg.writeEntry ("keep.source", options_keepsource);
	cg.writeEntry ("keep.source.pkgs", options_keepsourcepkgs);
	cg.writeEntry ("expressions", options_expressions);
	cg.writeEntry ("digits", options_digits);
	cg.writeEntry ("check.bounds", options_checkbounds);
	cg.writeEntry ("editor", options_editor);
	cg.writeEntry ("pager", options_pager);
	cg.writeEntry ("further init commands", options_further);
	cg.writeEntry ("addsyspaths", options_addpaths);
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
	options_addpaths = cg.readEntry ("addsyspaths", QStringList ());
}

//#################################################
//############### RKSettingsModuleRPackages ################
//#################################################

// static members
QStringList RKSettingsModuleRPackages::liblocs;
QStringList RKSettingsModuleRPackages::defaultliblocs;
QString RKSettingsModuleRPackages::r_libs_user;
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
	connect (cran_mirror_input, &QLineEdit::textChanged, this, &RKSettingsModuleRPackages::settingChanged);
	hbox->addWidget (cran_mirror_input);
	QPushButton* cran_mirror_button = new QPushButton (i18n ("Select mirror"), this);
	connect (cran_mirror_button, &QPushButton::clicked, this, &RKSettingsModuleRPackages::selectCRANMirror);
	hbox->addWidget (cran_mirror_button);

	repository_selector = new MultiStringSelector (i18n ("Additional package repositories (where libraries are downloaded from)"), this);
	repository_selector->setValues (package_repositories);
	connect (repository_selector, &MultiStringSelector::listChanged, this, &RKSettingsModuleRPackages::settingChanged);
	connect (repository_selector, &MultiStringSelector::getNewStrings, this, &RKSettingsModuleRPackages::addRepository);
	main_vbox->addWidget (repository_selector);

	archive_packages_box = new QCheckBox (i18n ("Archive downloaded packages"), this);
	archive_packages_box->setChecked (archive_packages);
	connect (archive_packages_box, &QCheckBox::stateChanged, this, &RKSettingsModuleRPackages::settingChanged);
	main_vbox->addWidget (archive_packages_box);

#if defined Q_OS_WIN || defined Q_OS_MACOS
	source_packages_box = new QCheckBox (i18n ("Build packages from source"), this);
	source_packages_box->setChecked (source_packages);
#else
	source_packages_box = new QCheckBox (i18n ("Build packages from source (not configurable on this platform)"), this);
	source_packages_box->setChecked (true);
	source_packages_box->setEnabled (false);
#endif
	RKCommonFunctions::setTips (QString ("<p>%1</p>").arg (i18n ("Installing packages from pre-compiled binaries (if available) is generally faster, and does not require an installation of development tools and libraries. On the other hand, building packages from source provides best compatibility. On Mac OS X and Linux, building packages from source is currently recommended.")), source_packages_box);
	connect (source_packages_box, &QCheckBox::stateChanged, this, &RKSettingsModuleRPackages::settingChanged);
	main_vbox->addWidget (source_packages_box);

	main_vbox->addStretch ();

	libloc_selector = new MultiStringSelector (i18n ("R Library locations (where libraries get installed to, locally)"), this);
	libloc_selector->setValues (liblocs);
	connect (libloc_selector, &MultiStringSelector::listChanged, this, &RKSettingsModuleRPackages::settingChanged);
	connect (libloc_selector, &MultiStringSelector::getNewStrings, this, &RKSettingsModuleRPackages::addLibLoc);
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

QStringList expandLibLocs (const QStringList &in) {
	QStringList ret;
	for (int i = 0; i < in.size (); ++i) {
		QString dummy = in[i];
		ret.append (dummy.replace (QLatin1String ("%v"), RKSessionVars::RVersion (true)));
	}
	return ret;
}

QString RKSettingsModuleRPackages::userLibraryLocation () {
	if (!r_libs_user.isEmpty()) return r_libs_user;
	return QDir (RKSettingsModuleGeneral::filesPath ()).absoluteFilePath ("library/" + RKSessionVars::RVersion (true));
}

QStringList RKSettingsModuleRPackages::libraryLocations () {
	return (QStringList (userLibraryLocation ()) + expandLibLocs (liblocs + defaultliblocs));
}

QStringList RKSettingsModuleRPackages::addUserLibLocTo (const QStringList& liblocs) {
	if (!liblocs.contains(userLibraryLocation ())) return (QStringList (userLibraryLocation ()) + liblocs);
	return liblocs;
}

void RKSettingsModuleRPackages::settingChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleRPackages::addLibLoc (QStringList *string_list) {
	RK_TRACE (SETTINGS);

	QDialog dialog (this);
	dialog.setWindowTitle (i18n ("Add R Library Directory"));
	QVBoxLayout *layout = new QVBoxLayout (&dialog);
	layout->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("Specify or select library location to add.\nNote that locations may contain a '%v', which will expand to the first "
	                                  "two components of the R version number (e.g. to 3.5), automatically. Including this is recommended, because R packages "
	                                  "compiled for one version of R will often fail to work correctly in a different version of R.")));

	KUrlRequester *req = new KUrlRequester ();
	req->setText (QDir (RKSettingsModuleGeneral::filesPath ()).absoluteFilePath ("library/%v"));
	req->setMode (KFile::Directory);
	layout->addWidget (req);

	QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttons->button(QDialogButtonBox::Ok)->setText (i18nc ("Add file to list", "Add"));
	connect (buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect (buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	layout->addWidget (buttons);

	if (dialog.exec () == QDialog::Accepted) {
		if (!req->text ().isEmpty ()) (*string_list).append (req->text ());
	}
}

void RKSettingsModuleRPackages::addRepository (QStringList *string_list) {
	RK_TRACE (SETTINGS);
	bool ok;
	QString new_string = QInputDialog::getText (this, i18n ("Add repository"), i18n ("Add URL of new repository"), QLineEdit::Normal, QString (), &ok);
	if (ok) (*string_list).append (new_string);
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

	// For additional library locations configured inside RKWard, try to create them, as needed.
	// This is especially important for versioned dirs (which will not exist after upgrading R, for instance)
	QString command;
	if (!liblocs.isEmpty()) {
		bool first = true;
		command = "local({\naddpaths <- unique (c(";
		QStringList ll = expandLibLocs(liblocs);
		foreach (const QString& libloc, ll) {
			if (first) first = false;
			else command.append (", ");
			command.append (RObject::rQuote (libloc));
		}
		command.append ("))\nlapply(addpaths, function(p) { if (!dir.exists(p)) try(dir.create(p, recursive=TRUE)) })\n})\n");
	}

	// For add library locations set "the R way", try to interfere as little as possible.
	command.append(".libPaths (unique (c (");
	bool first = true;
	QStringList ll = libraryLocations ();
	foreach (const QString& libloc, ll) {
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
#if defined Q_OS_WIN || defined Q_OS_MACOS
	ret.append ("options (pkgType=\"");
	if (source_packages) ret.append ("source");
	else if (RKSessionVars::compareRVersion ("3.1.3") <= 0) ret.append ("binary");   // "automatically select appropriate binary", unfortunately it's only available from R 3.1.3. onwards.
#	if defined Q_OS_WIN
	else ret.append ("win.binary");
#	else
	else if (RKSessionVars::compareRVersion ("3.0.0") > 0) {
		ret.append ("mac.binary.leopard");
	} else {
		// OS X binary packages have switched repo locations and package type in R >= 3.0.0
		ret.append ("mac.binary");
	}
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

#if defined Q_OS_WIN || defined Q_OS_MACOS
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
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString (), 0, 0, commandChain ());
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
	const QString rkward_repo ("http://files.kde.org/rkward/R/");
	package_repositories = cg.readEntry ("Repositories", QStringList (rkward_repo));
	if (RKSettingsModuleGeneral::storedConfigVersion () <= RKSettingsModuleGeneral::RKWardConfig_Pre0_5_7) {
		package_repositories.removeAll ("@CRAN@");	// COMPAT: Cran mirror was part of this list before 0.5.3
		if (package_repositories.isEmpty ()) package_repositories.append (rkward_repo);
	} else if (RKSettingsModuleGeneral::storedConfigVersion () < RKSettingsModuleGeneral::RKWardConfig_0_6_3) {
		package_repositories.removeAll ("http://rkward.sf.net/R");
		package_repositories.append (rkward_repo);
	}

	liblocs = cg.readEntry ("LibraryLocations", QStringList ());
	archive_packages = cg.readEntry ("archive packages", false);
#if defined Q_OS_WIN || defined Q_OS_MACOS
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

#include <QGroupBox>
#include <QRadioButton>

void RKSettingsModuleRPackages::validateSettingsInteractive (QList<RKSetupWizardItem*>* items) {
	RK_TRACE (SETTINGS);

	if (RKSettingsModuleGeneral::storedConfigVersion () < RKSettingsModuleGeneral::RKWardConfig_0_7_1) {
		QString legacy_libloc = QDir (RKSettingsModuleGeneral::filesPath ()).absoluteFilePath ("library");
		if (liblocs.contains (legacy_libloc)) {
			auto item = new RKSetupWizardItem(i18n("Unversioned library location"), i18n("The configured library locations (where R packages will be installed on this system) contains the directory '%1', "
			                                  "which was suggested as a default library location in earlier versions of RKWard. Use of this directory is no longer "
			                                  "recommended, as it is not accessible to R sessions outside of RKWard (unless configured, explicitly). Also due to the lack "
			                                  "of an R version number in the directory name, it offers no protection against using packages built for an incompatible "
			                                  "version of R.", legacy_libloc), RKSetupWizardItem::Warning);
			item->addOption(i18nc("verb", "Rename"), i18n("Rename this location to include the version number of the currently running R. Packages will continue "
			                                        "to work (if they are compatible with this version of R)."), [legacy_libloc](RKSetupWizard*) {
									liblocs.removeAll(legacy_libloc);
									QString new_loc = legacy_libloc + '-' + RKSessionVars::RVersion (true);
									RKGlobals::rInterface ()->issueCommand (QString ("file.rename(%1, %2)\n").arg (RObject::rQuote (legacy_libloc)).arg (RObject::rQuote (new_loc)), RCommand::App);
									liblocs.prepend (legacy_libloc + QStringLiteral ("-%v"));
									RKGlobals::rInterface ()->issueCommand (libLocsCommand(), RCommand::App);
								});
			item->addOption(i18nc("verb", "Remove"), i18n("Remove this location from the configuration (it will not be deleted on disk). You will have to "
			                                        "re-install any packages that you want to keep."), [legacy_libloc](RKSetupWizard*) {
									liblocs.removeAll(legacy_libloc);
									RKGlobals::rInterface ()->issueCommand (libLocsCommand(), RCommand::App);
								});
			item->addOption(i18nc("verb", "Keep"), i18n("Keep this location (do not change anything)"), [](RKSetupWizard*) {});

			items->append(item);
		}
	}
}
