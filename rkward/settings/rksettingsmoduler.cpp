/*
rksettingsmoduler - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
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
#include "../misc/rkspinbox.h"
#include "../misc/rkstandardicons.h"
#include "../rbackend/rkrinterface.h"
#include "../rbackend/rksessionvars.h"
#include "../misc/rkstyle.h"

#include "../debug.h"

// static constants
QString RKSettingsModuleR::builtin_editor = "<rkward>";
// session constants
QString RKSettingsModuleR::help_base_url;
// static members
RKConfigValue<QString> RKSettingsModuleR::options_outdec {"OutDec", "."};
RKConfigValue<int> RKSettingsModuleR::options_width {"width", 80};
RKConfigValue<int> RKSettingsModuleR::options_warn {"warn", 0};
RKConfigValue<int> RKSettingsModuleR::options_warningslength {"warnings.length", 1000};
RKConfigValue<int> RKSettingsModuleR::options_maxprint {"max.print", 99999};
RKConfigValue<bool> RKSettingsModuleR::options_keepsource {"keep.source", true};
RKConfigValue<bool> RKSettingsModuleR::options_keepsourcepkgs {"keep.source.pkgs", false};
RKConfigValue<int> RKSettingsModuleR::options_expressions {"expressions", 5000};
RKConfigValue<int> RKSettingsModuleR::options_digits {"digits", 7};
RKConfigValue<bool> RKSettingsModuleR::options_checkbounds {"check.bounds", false};
RKConfigValue<QString> RKSettingsModuleR::options_editor {"editor", builtin_editor};
RKConfigValue<QString> RKSettingsModuleR::options_pager {"pager", builtin_editor};
RKConfigValue<QString> RKSettingsModuleR::options_further {"further init commands", QString()};
RKConfigValue<QStringList> RKSettingsModuleR::options_addpaths {"addsyspaths", QStringList()};

RKSettingsModuleR::RKSettingsModuleR (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);

	main_vbox->addSpacing (2*RKStyle::spacingHint ());

	main_vbox->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("The following settings mostly affect R behavior in the console. It is generally safe to keep these unchanged.")));

	QGridLayout *grid = new QGridLayout ();
	main_vbox->addLayout (grid);
	int row = -1;

	// options (warn)
	grid->addWidget(new QLabel(i18n("Display warnings")), ++row, 0);
	auto warn_input = options_warn.makeDropDown(RKConfigBase::LabelList(
		{{-1, i18n("Suppress warnings")}, {0, i18n("Print warnings later (default)")}, {1, i18n("Print warnings immediately")}, {2, i18n ("Convert warnings to errors")}}
	), this);
	grid->addWidget(warn_input, row, 1);

	// options (OutDec)
	grid->addWidget (new QLabel (i18n ("Decimal character (only for printing)"), this), ++row, 0);
	outdec_input = new QLineEdit (options_outdec, this);
	outdec_input->setMaxLength (1);
	connect (outdec_input, &QLineEdit::textChanged, this, &RKSettingsModuleR::settingChanged);
	grid->addWidget (outdec_input, row, 1);

	// options (width)
	grid->addWidget(new QLabel(i18n("Output width (characters)")), ++row, 0);
	grid->addWidget(options_width.makeSpinBox(10, 10000, this), row, 1);

	// options (max.print)
	grid->addWidget(new QLabel(i18n("Maximum number of elements shown in print")), ++row, 0);
	grid->addWidget(options_maxprint.makeSpinBox(100, INT_MAX, this), row, 1);

	// options (warnings.length)
	grid->addWidget(new QLabel(i18n("Maximum length of warnings/errors to print")), ++row, 0);
	grid->addWidget(options_warningslength.makeSpinBox(100, 8192, this), row, 1);

	// options (keep.source)
	grid->addWidget(new QLabel(i18n("Keep comments in functions")), ++row, 0);
	auto keepsource_input = options_keepsource.makeDropDown(RKConfigBase::LabelList({{1, i18n("TRUE (default)")}, {0, i18n("FALSE")}}), this);
	grid->addWidget(keepsource_input, row, 1);

	// options (keep.source.pkgs)
	grid->addWidget(new QLabel(i18n("Keep comments in packages")), ++row, 0);
	auto keepsourcepkgs_input = options_keepsourcepkgs.makeDropDown(RKConfigBase::LabelList({{1, i18n("TRUE")}, {0, i18n("FALSE (default)")}}), this);
	grid->addWidget(keepsourcepkgs_input, row, 1);

	// options (expressions)
	grid->addWidget(new QLabel(i18n("Maximum level of nested expressions")), ++row, 0);
	grid->addWidget(options_expressions.makeSpinBox(25, 500000, this), row, 1);

	// options (digits)
	grid->addWidget(new QLabel(i18n("Default decimal precision in print ()")), ++row, 0);
	grid->addWidget(options_digits.makeSpinBox(1, 22, this), row, 1);

	// options (check.bounds)
	grid->addWidget(new QLabel(i18n("Check vector bounds (warn)")), ++row, 0);
	auto checkbounds_input = options_checkbounds.makeDropDown(RKConfigBase::LabelList({{1, i18n("TRUE")}, {0, i18n("FALSE (default)")}}), this);
	grid->addWidget(checkbounds_input, row, 1);

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

QString RKSettingsModuleR::caption() const {
	RK_TRACE(SETTINGS);
	return(i18n("R-Backend"));
}

QIcon RKSettingsModuleR::icon() const {
	RK_TRACE(SETTINGS);
	return QIcon::fromTheme("emblem-system-symbolic");
}

void RKSettingsModuleR::applyChanges () {
	RK_TRACE (SETTINGS);

	options_outdec = outdec_input->text ();
	options_editor = editor_input->currentText ();
	options_pager = pager_input->currentText ();
	options_further = further_input->toPlainText ();
	// normalize system paths before adding
	QStringList paths = addpaths_selector->getValues ();
	QStringList cleanpaths;
	for (int i = 0; i < paths.count(); ++i) {
		QString path = QDir::cleanPath(paths[i]);
		if (!cleanpaths.contains(path)) cleanpaths.append(path);
	}
	options_addpaths = cleanpaths;

// apply run time options in R
	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.cbegin (); it != commands.cend (); ++it) {
		RInterface::issueCommand (*it, RCommand::App, QString (), 0, 0, commandChain ());
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

static QLatin1String RTrueFalse(bool val) {
	if (val) return QLatin1String("TRUE");
	return QLatin1String("FALSE");
}

//static
QStringList RKSettingsModuleR::makeRRunTimeOptionCommands () {
	RK_TRACE (SETTINGS);
	QStringList list;

	QString outdec = options_outdec;
	if (outdec.isEmpty()) outdec = '.';
	list.append ("options (OutDec=\"" + outdec + "\")\n");
	list.append ("options (width=" + QString::number (options_width) + ")\n");
	list.append ("options (warn=" + QString::number (options_warn) + ")\n");
	list.append ("options (max.print=" + QString::number (options_maxprint) + ")\n");
	list.append ("options (warnings.length=" + QString::number (options_warningslength) + ")\n");
	list.append ("options (keep.source=" + RTrueFalse(options_keepsource) + ")\n");
	list.append ("options (keep.source.pkgs=" + RTrueFalse(options_keepsourcepkgs) + ")\n");
	list.append ("options (expressions=" + QString::number (options_expressions) + ")\n");
	list.append ("options (digits=" + QString::number (options_digits) + ")\n");
	list.append ("options (checkbounds=" + RTrueFalse(options_checkbounds) + ")\n");
	if (options_editor == builtin_editor) list.append ("options (editor=rk.edit.files)\n");
	else list.append ("options (editor=\"" + options_editor.get() + "\")\n");
	if (options_pager == builtin_editor) list.append ("options (pager=rk.show.files)\n");
	else list.append ("options (pager=\"" + options_pager.get() + "\")\n");
	if (!options_further.get().isEmpty ()) list.append (options_further.get() + '\n');
	if (!options_addpaths.get().isEmpty ()) {
		QString command = "rk.adjust.system.path (add=c(";
		foreach (const QString &p, options_addpaths.get()) {
			command.append (RObject::rQuote (p));
		}
		list.append (command + "))\n");
	}

	list.append ("options (help_type=\"html\")\n");		// for R 2.10.0 and above
	list.append ("options (browser=rk.show.html)\n");
	list.append ("options (askYesNo=rk.askYesNo)\n"); // for R 3.5.0 and above

	return list;
}

void RKSettingsModuleR::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group("R Settings");
	options_outdec.syncConfig(cg, a);
	options_width.syncConfig(cg, a);
	options_warn.syncConfig(cg, a);
	options_maxprint.syncConfig(cg, a);
	options_warningslength.syncConfig(cg, a);
	options_keepsource.syncConfig(cg, a);
	options_keepsourcepkgs.syncConfig(cg, a);
	options_expressions.syncConfig(cg, a);
	options_digits.syncConfig(cg, a);
	options_checkbounds.syncConfig(cg, a);
	options_editor.syncConfig(cg, a);
	options_pager.syncConfig(cg, a);
	options_further.syncConfig(cg, a);
	options_addpaths.syncConfig(cg, a);
}

//#################################################
//############### RKSettingsModuleRPackages ################
//#################################################

// static members
RKConfigValue<QStringList> RKSettingsModuleRPackages::liblocs {"LibraryLocations", QStringList()};
RKConfigValue<bool> RKSettingsModuleRPackages::archive_packages {"archive packages", false};
#if (defined Q_OS_WIN || defined Q_OS_MACOS)
#	if (defined USE_BINARY_PACKAGES)
#		define USE_SOURCE_PACKAGES false
#	else
#		define USE_SOURCE_PACKAGES true
#	endif
#else
#	define USE_SOURCE_PACKAGES true
#endif
RKConfigValue<bool> RKSettingsModuleRPackages::source_packages {"source_packages", USE_SOURCE_PACKAGES};
#define RKWARD_REPO "http://files.kde.org/rkward/R/"
RKConfigValue<QStringList> RKSettingsModuleRPackages::package_repositories {"Repositories", QStringList(RKWARD_REPO)};
QString RKSettingsModuleRPackages::essential_packages = QString ("base\nmethods\nutils\ngrDevices\ngraphics\nrkward");
RKConfigValue<QString> RKSettingsModuleRPackages::cran_mirror_url {"CRAN mirror url", "@CRAN@"};
QStringList RKSettingsModuleRPackages::defaultliblocs;
QString RKSettingsModuleRPackages::r_libs_user;

RKSettingsModuleRPackages::RKSettingsModuleRPackages (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent), RCommandReceiver () {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);

	main_vbox->addSpacing (2*RKStyle::spacingHint ());

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

	main_vbox->addWidget(archive_packages.makeCheckbox(i18n("Archive downloaded packages"), this));

	auto source_packages_box = source_packages.makeCheckbox(i18n ("Build packages from source"), this);
#if !(defined Q_OS_WIN || defined Q_OS_MACOS)
	source_packages_box->setText(i18n("Build packages from source (not configurable on this platform)"));
	source_packages_box->setChecked (true);
	source_packages_box->setEnabled (false);
#endif
	RKCommonFunctions::setTips (QString ("<p>%1</p>").arg (i18n ("Installing packages from pre-compiled binaries (if available) is generally faster, and does not require an installation of development tools and libraries. On the other hand, building packages from source provides best compatibility. On Mac OS X and Linux, building packages from source is currently recommended.")), source_packages_box);
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

	if (!libraryLocations ().contains (new_loc)) liblocs.get().prepend (new_loc);

	// update the backend in any case. User might have changed liblocs, there.
	RInterface::issueCommand (".libPaths (unique (c (" + RObject::rQuote (new_loc) + ", .libPaths ())))", RCommand::App | RCommand::Sync, QString (), 0, 0, chain);
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

QString RKSettingsModuleRPackages::caption() const {
	RK_TRACE(SETTINGS);
	return(i18n("R-Packages"));
}

QIcon RKSettingsModuleRPackages::icon() const {
	RK_TRACE(SETTINGS);
	return RKStandardIcons::getIcon(RKStandardIcons::ObjectPackageEnvironment);
}

#define SELECT_CRAN_MIRROR_COMMAND 123
void RKSettingsModuleRPackages::selectCRANMirror () {
	RK_TRACE (SETTINGS);
	QString title = i18n ("Select CRAN mirror");
	
	RCommand* command = new RCommand ("rk.select.CRAN.mirror()\n", RCommand::App | RCommand::GetStringVector, title, this, SELECT_CRAN_MIRROR_COMMAND);

	RKProgressControl* control = new RKProgressControl (this, title, title, RKProgressControl::CancellableProgress);
	control->addRCommand (command, true);
	RInterface::issueCommand (command, commandChain ());
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
	if (!liblocs.get().isEmpty()) {
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
	ret.append("options (pkgType=\"");
	if (source_packages) ret.append("source");
	else ret.append("binary");   // "automatically select appropriate binary"
	ret.append("\")\n");
#endif
	return ret;
}

//static
QStringList RKSettingsModuleRPackages::makeRRunTimeOptionCommands () {
	RK_TRACE (SETTINGS);
	QStringList list;

// package repositories
	QString command = "options (repos=c (CRAN=" + RObject::rQuote (cran_mirror_url);
	for (auto it = package_repositories.get().constBegin (); it != package_repositories.get().constEnd(); ++it) {
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
	if (cran_mirror_url.get().isEmpty ()) cran_mirror_url = "@CRAN@";

	package_repositories = repository_selector->getValues ();
	liblocs = libloc_selector->getValues ();

// apply options in R
	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.cbegin (); it != commands.cend (); ++it) {
		RInterface::issueCommand (*it, RCommand::App, QString (), 0, 0, commandChain ());
	}
}

void RKSettingsModuleRPackages::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group("R Settings");
	cran_mirror_url.syncConfig(cg, a);
	liblocs.syncConfig(cg, a);
	archive_packages.syncConfig(cg, a);
	source_packages.syncConfig(cg, a);  // NOTE: does not take effect on Linux, see pkgTypeOption
	package_repositories.syncConfig(cg, a);

	if (a == RKConfigBase::LoadConfig) {
		const QString rkward_repo (RKWARD_REPO);
		if (RKSettingsModuleGeneral::storedConfigVersion () <= RKSettingsModuleGeneral::RKWardConfig_Pre0_5_7) {
			auto v = package_repositories.get();
			v.removeAll ("@CRAN@");	// COMPAT: Cran mirror was part of this list before 0.5.3
			if (v.isEmpty ()) v.append(rkward_repo);
			package_repositories = v;
		} else if (RKSettingsModuleGeneral::storedConfigVersion () < RKSettingsModuleGeneral::RKWardConfig_0_6_3) {
			auto v = package_repositories.get();
			v.removeAll("http://rkward.sf.net/R");
			v.append(rkward_repo);
			package_repositories = v;
		}

		if (USE_SOURCE_PACKAGES && (RKSettingsModuleGeneral::storedConfigVersion () < RKSettingsModuleGeneral::RKWardConfig_0_6_1)) {
			// revert default on MacOSX, even if a previous stored setting exists
			source_packages = true;
		}
	}
}

#include <QGroupBox>
#include <QRadioButton>

void RKSettingsModuleRPackages::validateSettingsInteractive (QList<RKSetupWizardItem*>* items) {
	RK_TRACE (SETTINGS);

	if (RKSettingsModuleGeneral::storedConfigVersion () < RKSettingsModuleGeneral::RKWardConfig_0_7_1) {
		QString legacy_libloc = QDir (RKSettingsModuleGeneral::filesPath ()).absoluteFilePath ("library");
		if (liblocs.get().contains(legacy_libloc)) {
			auto item = new RKSetupWizardItem(i18n("Unversioned library location"), i18n("The configured library locations (where R packages will be installed on this system) contains the directory '%1', "
			                                  "which was suggested as a default library location in earlier versions of RKWard. Use of this directory is no longer "
			                                  "recommended, as it is not accessible to R sessions outside of RKWard (unless configured, explicitly). Also due to the lack "
			                                  "of an R version number in the directory name, it offers no protection against using packages built for an incompatible "
			                                  "version of R.", legacy_libloc), RKSetupWizardItem::Warning);
			item->addOption(i18nc("verb", "Rename"), i18n("Rename this location to include the version number of the currently running R. Packages will continue "
			                                        "to work (if they are compatible with this version of R)."), [legacy_libloc](RKSetupWizard*) {
									liblocs.get().removeAll(legacy_libloc);
									QString new_loc = legacy_libloc + '-' + RKSessionVars::RVersion (true);
									RInterface::issueCommand (QString ("file.rename(%1, %2)\n").arg(RObject::rQuote(legacy_libloc), RObject::rQuote (new_loc)), RCommand::App);
									liblocs.get().prepend (legacy_libloc + QStringLiteral ("-%v"));
									RInterface::issueCommand (libLocsCommand(), RCommand::App);
								});
			item->addOption(i18nc("verb", "Remove"), i18n("Remove this location from the configuration (it will not be deleted on disk). You will have to "
			                                        "re-install any packages that you want to keep."), [legacy_libloc](RKSetupWizard*) {
									liblocs.get().removeAll(legacy_libloc);
									RInterface::issueCommand (libLocsCommand(), RCommand::App);
								});
			item->addOption(i18nc("verb", "Keep"), i18n("Keep this location (do not change anything)"), [](RKSetupWizard*) {});

			items->append(item);
		}
	}
}
