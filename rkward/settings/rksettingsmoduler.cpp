/*
rksettingsmoduler - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmoduler.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KUrlRequester>

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QInputDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include "../core/robject.h"
#include "../dialogs/rksetupwizard.h"
#include "../misc/multistringselector.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkspinbox.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkstyle.h"
#include "../plugin/rkcomponentmap.h"
#include "../rbackend/rkrinterface.h"
#include "../rbackend/rksessionvars.h"
#include "rksettings.h"
#include "rksettingsmodulegeneral.h"
#include "rksettingsmoduleplugins.h"

#include "../debug.h"

// static constants
QString RKSettingsModuleR::builtin_editor = QStringLiteral("<rkward>");
// session constants
QString RKSettingsModuleR::help_base_url;
// static members
RKConfigValue<QString> RKSettingsModuleR::options_outdec{"OutDec", QStringLiteral(".")};
RKConfigValue<int> RKSettingsModuleR::options_width{"width", 80};
RKConfigValue<int> RKSettingsModuleR::options_warn{"warn", 0};
RKConfigValue<int> RKSettingsModuleR::options_warningslength{"warnings.length", 1000};
RKConfigValue<int> RKSettingsModuleR::options_maxprint{"max.print", 99999};
RKConfigValue<bool> RKSettingsModuleR::options_keepsource{"keep.source", true};
RKConfigValue<bool> RKSettingsModuleR::options_keepsourcepkgs{"keep.source.pkgs", false};
RKConfigValue<int> RKSettingsModuleR::options_expressions{"expressions", 5000};
RKConfigValue<int> RKSettingsModuleR::options_digits{"digits", 7};
RKConfigValue<bool> RKSettingsModuleR::options_checkbounds{"check.bounds", false};
RKConfigValue<QString> RKSettingsModuleR::options_editor{"editor", builtin_editor};
RKConfigValue<QString> RKSettingsModuleR::options_pager{"pager", builtin_editor};
RKConfigValue<QString> RKSettingsModuleR::options_further{"further init commands", QString()};
RKConfigValue<QStringList> RKSettingsModuleR::options_addpaths{"addsyspaths", QStringList()};
RKConfigValue<QString> RKSettingsModuleR::options_r_binary{"user configured R binary", QString()};

class RKSettingsPageR : public RKSettingsModuleWidget {
  public:
	RKSettingsPageR(QWidget *parent, RKSettingsModule *parent_module) : RKSettingsModuleWidget(parent, parent_module, RKSettingsModuleR::page_id) {
		RK_TRACE(SETTINGS);

		setWindowTitle(i18n("R-Backend"));
		setWindowIcon(QIcon::fromTheme(QStringLiteral("emblem-system-symbolic")));

		QVBoxLayout *main_vbox = new QVBoxLayout(this);

		main_vbox->addSpacing(2 * RKStyle::spacingHint());

		main_vbox->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("The following settings mostly affect R behavior in the console. It is generally safe to keep these unchanged.")));

		QGridLayout *grid = new QGridLayout();
		main_vbox->addLayout(grid);
		int row = -1;

		// options (warn)
		grid->addWidget(new QLabel(i18n("Display warnings")), ++row, 0);
		auto warn_input = RKSettingsModuleR::options_warn.makeDropDown(RKConfigBase::LabelList(
		                                                                   {{-1, i18n("Suppress warnings")}, {0, i18n("Print warnings later (default)")}, {1, i18n("Print warnings immediately")}, {2, i18n("Convert warnings to errors")}}),
		                                                               this);
		grid->addWidget(warn_input, row, 1);

		// options (OutDec)
		grid->addWidget(new QLabel(i18n("Decimal character (only for printing)"), this), ++row, 0);
		auto outdec_input = RKSettingsModuleR::options_outdec.makeLineEdit(this);
		outdec_input->setMaxLength(1);
		grid->addWidget(outdec_input, row, 1);

		// options (width)
		grid->addWidget(new QLabel(i18n("Output width (characters)")), ++row, 0);
		grid->addWidget(RKSettingsModuleR::options_width.makeSpinBox(10, 10000, this), row, 1);

		// options (max.print)
		grid->addWidget(new QLabel(i18n("Maximum number of elements shown in print")), ++row, 0);
		grid->addWidget(RKSettingsModuleR::options_maxprint.makeSpinBox(100, INT_MAX, this), row, 1);

		// options (warnings.length)
		grid->addWidget(new QLabel(i18n("Maximum length of warnings/errors to print")), ++row, 0);
		grid->addWidget(RKSettingsModuleR::options_warningslength.makeSpinBox(100, 8192, this), row, 1);

		// options (keep.source)
		grid->addWidget(new QLabel(i18n("Keep comments in functions")), ++row, 0);
		auto keepsource_input = RKSettingsModuleR::options_keepsource.makeDropDown(RKConfigBase::LabelList({{1, i18n("TRUE (default)")}, {0, i18n("FALSE")}}), this);
		grid->addWidget(keepsource_input, row, 1);

		// options (keep.source.pkgs)
		grid->addWidget(new QLabel(i18n("Keep comments in packages")), ++row, 0);
		auto keepsourcepkgs_input = RKSettingsModuleR::options_keepsourcepkgs.makeDropDown(RKConfigBase::LabelList({{1, i18n("TRUE")}, {0, i18n("FALSE (default)")}}), this);
		grid->addWidget(keepsourcepkgs_input, row, 1);

		// options (expressions)
		grid->addWidget(new QLabel(i18n("Maximum level of nested expressions")), ++row, 0);
		grid->addWidget(RKSettingsModuleR::options_expressions.makeSpinBox(25, 500000, this), row, 1);

		// options (digits)
		grid->addWidget(new QLabel(i18n("Default decimal precision in print ()")), ++row, 0);
		grid->addWidget(RKSettingsModuleR::options_digits.makeSpinBox(1, 22, this), row, 1);

		// options (check.bounds)
		grid->addWidget(new QLabel(i18n("Check vector bounds (warn)")), ++row, 0);
		auto checkbounds_input = RKSettingsModuleR::options_checkbounds.makeDropDown(RKConfigBase::LabelList({{1, i18n("TRUE")}, {0, i18n("FALSE (default)")}}), this);
		grid->addWidget(checkbounds_input, row, 1);

		grid->addWidget(new QLabel(i18n("Editor command"), this), ++row, 0);
		editor_input = new QComboBox(this);
		editor_input->setEditable(true);
		editor_input->addItem(RKSettingsModuleR::builtin_editor);
		if (RKSettingsModuleR::options_editor != RKSettingsModuleR::builtin_editor) {
			editor_input->addItem(RKSettingsModuleR::options_editor);
			editor_input->setCurrentIndex(1);
		}
		connect(editor_input, &QComboBox::editTextChanged, this, &RKSettingsPageR::change);
		grid->addWidget(editor_input, row, 1);

		grid->addWidget(new QLabel(i18n("Pager command"), this), ++row, 0);
		pager_input = new QComboBox(this);
		pager_input->setEditable(true);
		pager_input->addItem(RKSettingsModuleR::builtin_editor);
		if (RKSettingsModuleR::options_pager != RKSettingsModuleR::builtin_editor) {
			pager_input->addItem(RKSettingsModuleR::options_pager);
			pager_input->setCurrentIndex(1);
		}
		connect(pager_input, &QComboBox::editTextChanged, this, &RKSettingsPageR::change);
		grid->addWidget(pager_input, row, 1);

		grid->addWidget(new QLabel(i18n("Further (option) commands to run in each session"), this), ++row, 0, 1, 2);
		further_input = new QTextEdit(this);
		further_input->setWordWrapMode(QTextOption::NoWrap);
		further_input->setAcceptRichText(false);
		further_input->setPlainText(RKSettingsModuleR::options_further);
		connect(further_input, &QTextEdit::textChanged, this, &RKSettingsPageR::change);
		grid->addWidget(further_input, ++row, 0, 1, 2);

		main_vbox->addStretch();

		addpaths_selector = new MultiStringSelector(i18n("Addition search paths for utilities used by R"), this);
		addpaths_selector->setValues(RKSettingsModuleR::options_addpaths);
		connect(addpaths_selector, &MultiStringSelector::listChanged, this, &RKSettingsPageR::change);
		connect(addpaths_selector, &MultiStringSelector::getNewStrings, this, [this](QStringList *string_list) {
			QDialog dialog(this);
			dialog.setWindowTitle(i18n("Add System Path Directory"));
			QVBoxLayout *layout = new QVBoxLayout(&dialog);
			layout->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("Specify or select directory to add to the system file path of the running R session")));

			KUrlRequester *req = new KUrlRequester();
			req->setMode(KFile::Directory);
			layout->addWidget(req);

			QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
			buttons->button(QDialogButtonBox::Ok)->setText(i18nc("Add directory to list", "Add"));
			connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
			connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
			layout->addWidget(buttons);

			if (dialog.exec() == QDialog::Accepted) {
				if (!req->text().isEmpty()) (*string_list).append(req->text());
			}
		});
		main_vbox->addWidget(addpaths_selector);
	}
	void applyChanges() override {
		RK_TRACE(SETTINGS);

		RKSettingsModuleR::options_editor = editor_input->currentText();
		RKSettingsModuleR::options_pager = pager_input->currentText();
		RKSettingsModuleR::options_further = further_input->toPlainText();
		// normalize system paths before adding
		QStringList paths = addpaths_selector->getValues();
		QStringList cleanpaths;
		for (int i = 0; i < paths.count(); ++i) {
			QString path = QDir::cleanPath(paths[i]);
			if (!cleanpaths.contains(path)) cleanpaths.append(path);
		}
		RKSettingsModuleR::options_addpaths = cleanpaths;

		// apply run time options in R
		QStringList commands = RKSettingsModuleR::makeRRunTimeOptionCommands();
		for (QStringList::const_iterator it = commands.cbegin(); it != commands.cend(); ++it) {
			RInterface::issueCommand(new RCommand(*it, RCommand::App), parentModule()->commandChain());
		}
	}

  private:
	QComboBox *editor_input;
	QComboBox *pager_input;
	QTextEdit *further_input;
	MultiStringSelector *addpaths_selector;
};

RKSettingsModuleR::RKSettingsModuleR(QObject *parent) : RKSettingsModule(parent) {
	RK_TRACE(SETTINGS);
}

RKSettingsModuleR::~RKSettingsModuleR() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleR::createPages(RKSettings *parent) {
	parent->addSettingsPage(new RKSettingsPageR(parent, this));
}

static QString RTrueFalse(bool val) {
	if (val) return u"TRUE"_s;
	return u"FALSE"_s;
}

// static
QStringList RKSettingsModuleR::makeRRunTimeOptionCommands() {
	RK_TRACE(SETTINGS);
	QStringList list;

	QString outdec = options_outdec;
	if (outdec.isEmpty()) outdec = u'.';
	list.append(u"options (OutDec=\""_s + outdec + u"\")\n"_s);
	list.append(u"options (width="_s + QString::number(options_width) + u")\n"_s);
	list.append(u"options (warn="_s + QString::number(options_warn) + u")\n"_s);
	list.append(u"options (max.print="_s + QString::number(options_maxprint) + u")\n"_s);
	list.append(u"options (warnings.length="_s + QString::number(options_warningslength) + u")\n"_s);
	list.append(u"options (keep.source="_s + RTrueFalse(options_keepsource) + u")\n"_s);
	list.append(u"options (keep.source.pkgs="_s + RTrueFalse(options_keepsourcepkgs) + u")\n"_s);
	list.append(u"options (expressions="_s + QString::number(options_expressions) + u")\n"_s);
	list.append(u"options (digits="_s + QString::number(options_digits) + u")\n"_s);
	list.append(u"options (checkbounds="_s + RTrueFalse(options_checkbounds) + u")\n"_s);
	if (options_editor == builtin_editor) list.append(u"options (editor=rk.edit.files)\n"_s);
	else list.append(u"options (editor=\""_s + options_editor.get() + u"\")\n"_s);
	if (options_pager == builtin_editor) list.append(u"options (pager=rk.show.files)\n"_s);
	else list.append(u"options (pager=\""_s + options_pager.get() + u"\")\n"_s);
	if (!options_further.get().isEmpty()) list.append(options_further.get() + u'\n');
	if (!options_addpaths.get().isEmpty()) {
		QString command = u"rk.adjust.system.path (add=c("_s;
		for (const QString &p : std::as_const(options_addpaths.get())) {
			command.append(RObject::rQuote(p));
		}
		list.append(command + u"))\n"_s);
	}

	list.append(u"options (help_type=\"html\")\n"_s); // for R 2.10.0 and above
	list.append(u"options (browser=rk.show.html)\n"_s);
	list.append(u"options (askYesNo=rk.askYesNo)\n"_s); // for R 3.5.0 and above

	return list;
}

void RKSettingsModuleR::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group(QStringLiteral("R Settings"));
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
	options_r_binary.syncConfig(cg, a);
}

//#################################################
//############### RKSettingsModuleRPackages ################
//#################################################

// static members
RKConfigValue<QStringList> RKSettingsModuleRPackages::liblocs{"LibraryLocations", QStringList()};
RKConfigValue<bool> RKSettingsModuleRPackages::archive_packages{"archive packages", false};
#if (defined Q_OS_WIN || defined Q_OS_MACOS)
#	if (defined USE_BINARY_PACKAGES)
#		define USE_SOURCE_PACKAGES false
#	else
#		define USE_SOURCE_PACKAGES true
#	endif
#else
#	define USE_SOURCE_PACKAGES true
#endif
RKConfigValue<bool> RKSettingsModuleRPackages::source_packages{"source_packages", USE_SOURCE_PACKAGES};
#define RKWARD_REPO "https://files.kde.org/rkward/R/"
RKConfigValue<QStringList> RKSettingsModuleRPackages::package_repositories{"Repositories", QStringList(QStringLiteral(RKWARD_REPO))};
QString RKSettingsModuleRPackages::essential_packages = u"base\nmethods\nutils\ngrDevices\ngraphics\nrkward"_s;
RKConfigValue<QString> RKSettingsModuleRPackages::cran_mirror_url{"CRAN mirror url", u"@CRAN@"_s};
QStringList RKSettingsModuleRPackages::defaultliblocs;
QString RKSettingsModuleRPackages::r_libs_user;

class RKSettingsPageRPackages : public RKSettingsModuleWidget {
  public:
	RKSettingsPageRPackages(QWidget *parent, RKSettingsModule *parent_module) : RKSettingsModuleWidget(parent, parent_module, RKSettingsModuleRPackages::page_id, RKSettingsModulePlugins::addons_superpage_id) {
		RK_TRACE(SETTINGS);

		setWindowTitle(i18n("R-Packages"));
		setWindowIcon(RKStandardIcons::getIcon(RKStandardIcons::ObjectPackageEnvironment));

		QVBoxLayout *main_vbox = new QVBoxLayout(this);

		main_vbox->addSpacing(2 * RKStyle::spacingHint());

		main_vbox->addWidget(new QLabel(i18n("CRAN download mirror (leave empty to be prompted once each session):"), this));
		QHBoxLayout *hbox = new QHBoxLayout();
		main_vbox->addLayout(hbox);
		auto cran_mirror_input = RKSettingsModuleRPackages::cran_mirror_url.makeLineEdit(this);
		if (RKSettingsModuleRPackages::cran_mirror_url == QStringLiteral("@CRAN@")) cran_mirror_input->clear();
		hbox->addWidget(cran_mirror_input);
		QPushButton *cran_mirror_button = new QPushButton(i18n("Select mirror"), this);
		connect(cran_mirror_button, &QPushButton::clicked, this, [this, cran_mirror_input]() {
			QString title = i18n("Select CRAN mirror");
			RCommand *command = new RCommand(QStringLiteral("rk.select.CRAN.mirror()\n"), RCommand::App | RCommand::GetStringVector, title);
			connect(command->notifier(), &RCommandNotifier::commandFinished, cran_mirror_input, [cran_mirror_input](RCommand *command) {
				if (command->succeeded()) {
					RK_ASSERT(command->getDataLength() >= 1);
					cran_mirror_input->setText(command->stringVector().value(0));
				}
			});

			RKProgressControl *control = new RKProgressControl(this, title, title, RKProgressControl::CancellableProgress);
			control->addRCommand(command, true);
			RInterface::issueCommand(command, parentModule()->commandChain());
			control->doModal(true);
		});
		hbox->addWidget(cran_mirror_button);

		repository_selector = new MultiStringSelector(i18n("Additional package repositories (where libraries are downloaded from)"), this);
		repository_selector->setValues(RKSettingsModuleRPackages::package_repositories);
		connect(repository_selector, &MultiStringSelector::listChanged, this, &RKSettingsPageRPackages::change);
		connect(repository_selector, &MultiStringSelector::getNewStrings, this, [this](QStringList *string_list) {
			bool ok;
			QString new_string = QInputDialog::getText(this, i18n("Add repository"), i18n("Add URL of new repository"), QLineEdit::Normal, QString(), &ok);
			if (ok) (*string_list).append(new_string);
		});
		main_vbox->addWidget(repository_selector);

		main_vbox->addWidget(RKSettingsModuleRPackages::archive_packages.makeCheckbox(i18n("Archive downloaded packages"), this));

		auto source_packages_box = RKSettingsModuleRPackages::source_packages.makeCheckbox(i18n("Build packages from source"), this);
#if !(defined Q_OS_WIN || defined Q_OS_MACOS)
		source_packages_box->setText(i18n("Build packages from source (not configurable on this platform)"));
		source_packages_box->setChecked(true);
		source_packages_box->setEnabled(false);
#endif
		RKCommonFunctions::setTips(QStringLiteral("<p>%1</p>").arg(i18n("Installing packages from pre-compiled binaries (if available) is generally faster, and does not require an installation of development tools and libraries. On the other hand, building packages from source provides best compatibility.")), source_packages_box);
		main_vbox->addWidget(source_packages_box);

		hbox = new QHBoxLayout();
		main_vbox->addLayout(hbox);
		auto button = new QPushButton(i18n("Install from git"));
		auto label = RKCommonFunctions::wordWrappedLabel(i18n("Some add-on packages are not available in the CRAN repository, but can be installed from development repositories. Use the button \"%1\", to install such packages, comfortably.", button->text()));
		hbox->addWidget(label);
		hbox->setStretchFactor(label, 2);
		connect(button, &QPushButton::clicked, this, []() { RKComponentMap::getMap()->invokeComponent(QStringLiteral("rkward::install_from_git"), QStringList()); });
		hbox->addWidget(button);

		main_vbox->addStretch();

		libloc_selector = new MultiStringSelector(i18n("R Library locations (where libraries get installed to, locally)"), this);
		libloc_selector->setValues(RKSettingsModuleRPackages::liblocs);
		connect(libloc_selector, &MultiStringSelector::listChanged, this, &RKSettingsPageRPackages::change);
		connect(libloc_selector, &MultiStringSelector::getNewStrings, this, [this](QStringList *string_list) {
			QDialog dialog(this);
			dialog.setWindowTitle(i18n("Add R Library Directory"));
			QVBoxLayout *layout = new QVBoxLayout(&dialog);
			layout->addWidget(RKCommonFunctions::wordWrappedLabel(
			    i18n("Specify or select library location to add.\nNote that locations may contain a '%v', which will expand to the first "
			         "two components of the R version number (e.g. to 3.5), automatically. Including this is recommended, because R packages "
			         "compiled for one version of R will often fail to work correctly in a different version of R.")));
			KUrlRequester *req = new KUrlRequester();
			req->setText(QDir(RKSettingsModuleGeneral::filesPath()).absoluteFilePath(QStringLiteral("library/%v")));
			req->setMode(KFile::Directory);
			layout->addWidget(req);

			QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
			buttons->button(QDialogButtonBox::Ok)->setText(i18nc("Add file to list", "Add"));
			connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
			connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
			layout->addWidget(buttons);

			if (dialog.exec() == QDialog::Accepted) {
				if (!req->text().isEmpty()) (*string_list).append(req->text());
			}
		});
		main_vbox->addWidget(libloc_selector);
		label = new QLabel(i18n("Note: The startup defaults will always be used in addition to the locations you specify in this list"), this);
		main_vbox->addWidget(label);

		main_vbox->addStretch();
	}
	void applyChanges() override {
		RK_TRACE(SETTINGS);

		if (RKSettingsModuleRPackages::cran_mirror_url.get().isEmpty()) RKSettingsModuleRPackages::cran_mirror_url = QStringLiteral("@CRAN@");

		RKSettingsModuleRPackages::package_repositories = repository_selector->getValues();
		RKSettingsModuleRPackages::liblocs = libloc_selector->getValues();

		// apply options in R
		QStringList commands = RKSettingsModuleRPackages::makeRRunTimeOptionCommands();
		for (QStringList::const_iterator it = commands.cbegin(); it != commands.cend(); ++it) {
			RInterface::issueCommand(new RCommand(*it, RCommand::App), parentModule()->commandChain());
		}
	}

  private:
	MultiStringSelector *libloc_selector;
	MultiStringSelector *repository_selector;
};

RKSettingsModuleRPackages::RKSettingsModuleRPackages(QObject *parent) : RKSettingsModule(parent) {
	RK_TRACE(SETTINGS);
}

RKSettingsModuleRPackages::~RKSettingsModuleRPackages() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleRPackages::createPages(RKSettings *parent) {
	parent->addSettingsPage(new RKSettingsPageRPackages(parent, this));
}

void RKSettingsModuleRPackages::addLibraryLocation(const QString &new_loc, RCommandChain *chain) {
	RK_TRACE(SETTINGS);

	if (!libraryLocations().contains(new_loc)) liblocs.get().prepend(new_loc);

	// update the backend in any case. User might have changed liblocs, there.
	RInterface::issueCommand(new RCommand(u".libPaths (unique (c ("_s + RObject::rQuote(new_loc) + u", .libPaths ())))"_s, RCommand::App | RCommand::Sync),
	                         chain);
}

QStringList expandLibLocs(const QStringList &in) {
	QStringList ret;
	for (int i = 0; i < in.size(); ++i) {
		QString dummy = in[i];
		ret.append(dummy.replace("%v"_L1, RKSessionVars::RVersion(true)));
	}
	return ret;
}

QString RKSettingsModuleRPackages::userLibraryLocation() {
	if (!r_libs_user.isEmpty()) return r_libs_user;
	return QDir(RKSettingsModuleGeneral::filesPath()).absoluteFilePath(u"library/"_s + RKSessionVars::RVersion(true));
}

QStringList RKSettingsModuleRPackages::libraryLocations() {
	return (QStringList(userLibraryLocation()) + expandLibLocs(liblocs.get() + defaultliblocs));
}

QStringList RKSettingsModuleRPackages::addUserLibLocTo(const QStringList &liblocs) {
	if (!liblocs.contains(userLibraryLocation())) return (QStringList(userLibraryLocation()) + liblocs);
	return liblocs;
}

QString RKSettingsModuleRPackages::libLocsCommand() {
	RK_TRACE(SETTINGS);

	// For additional library locations configured inside RKWard, try to create them, as needed.
	// This is especially important for versioned dirs (which will not exist after upgrading R, for instance)
	QString command;
	if (!liblocs.get().isEmpty()) {
		bool first = true;
		command = u"local({\naddpaths <- unique (c("_s;
		const QStringList ll = expandLibLocs(liblocs);
		for (const QString &libloc : ll) {
			if (first)
				first = false;
			else
				command.append(u", "_s);
			command.append(RObject::rQuote(libloc));
		}
		command.append(u"))\nlapply(addpaths, function(p) { if (!dir.exists(p)) try(dir.create(p, recursive=TRUE)) })\n})\n"_s);
	}

	// For add library locations set "the R way", try to interfere as little as possible.
	command.append(u".libPaths (unique (c ("_s);
	bool first = true;
	const QStringList ll = libraryLocations();
	for (const QString &libloc : ll) {
		if (first)
			first = false;
		else
			command.append(u", "_s);
		command.append(RObject::rQuote(libloc));
	}
	command.append(u")))"_s);

	return command;
}

// static
QString RKSettingsModuleRPackages::pkgTypeOption() {
	QString ret;
#if defined Q_OS_WIN || defined Q_OS_MACOS
	ret.append(u"options (pkgType=\""_s);
	if (source_packages) ret.append(u"source"_s);
	else ret.append(u"binary"_s); // "automatically select appropriate binary"
	ret.append(u"\")\n"_s);
#endif
	return ret;
}

// static
QStringList RKSettingsModuleRPackages::makeRRunTimeOptionCommands() {
	RK_TRACE(SETTINGS);
	QStringList list;

	// package repositories
	QString command = u"options (repos=c (CRAN="_s + RObject::rQuote(cran_mirror_url);
	for (auto it = package_repositories.get().constBegin(); it != package_repositories.get().constEnd(); ++it) {
		command.append(u", "_s + RObject::rQuote(*it));
	}
	list.append(command + u"))\n"_s);

#if defined Q_OS_WIN || defined Q_OS_MACOS
	list.append(pkgTypeOption());
#endif

	// library locations
	list.append(libLocsCommand());

	return list;
}

void RKSettingsModuleRPackages::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group(u"R Settings"_s);
	cran_mirror_url.syncConfig(cg, a);
	liblocs.syncConfig(cg, a);
	archive_packages.syncConfig(cg, a);
	source_packages.syncConfig(cg, a); // NOTE: does not take effect on Linux, see pkgTypeOption
	package_repositories.syncConfig(cg, a);

	if (a == RKConfigBase::LoadConfig) {
		if (RKSettingsModuleGeneral::storedConfigVersion() < RKSettingsModuleGeneral::RKWardConfig_0_6_3) {
			auto v = package_repositories.get();
			v.removeAll(u"http://rkward.sf.net/R"_s);
			v.append(QStringLiteral(RKWARD_REPO));
			package_repositories = v;
		}
	}
}

void RKSettingsModuleRPackages::validateSettingsInteractive(QList<RKSetupWizardItem *> *items) {
	RK_TRACE(SETTINGS);

	if (RKSettingsModuleGeneral::storedConfigVersion() < RKSettingsModuleGeneral::RKWardConfig_0_7_1) {
		QString legacy_libloc = QDir(RKSettingsModuleGeneral::filesPath()).absoluteFilePath(u"library"_s);
		if (liblocs.get().contains(legacy_libloc)) {
			auto item = new RKSetupWizardItem(i18n("Unversioned library location"), i18n("The configured library locations (where R packages will be installed on this system) contains the directory '%1', "
			                                                                             "which was suggested as a default library location in earlier versions of RKWard. Use of this directory is no longer "
			                                                                             "recommended, as it is not accessible to R sessions outside of RKWard (unless configured, explicitly). Also due to the lack "
			                                                                             "of an R version number in the directory name, it offers no protection against using packages built for an incompatible "
			                                                                             "version of R.",
			                                                                             legacy_libloc),
			                                  RKSetupWizardItem::Warning);
			item->addOption(i18nc("verb", "Rename"), i18n("Rename this location to include the version number of the currently running R. Packages will continue "
			                                              "to work (if they are compatible with this version of R)."),
			                [legacy_libloc](RKSetupWizard *) {
				                liblocs.get().removeAll(legacy_libloc);
				                QString new_loc = legacy_libloc + u'-' + RKSessionVars::RVersion(true);
				                RInterface::issueCommand(u"file.rename(%1, %2)\n"_s.arg(RObject::rQuote(legacy_libloc), RObject::rQuote(new_loc)), RCommand::App);
				                liblocs.get().prepend(legacy_libloc + u"-%v"_s);
				                RInterface::issueCommand(libLocsCommand(), RCommand::App);
			                });
			item->addOption(i18nc("verb", "Remove"), i18n("Remove this location from the configuration (it will not be deleted on disk). You will have to "
			                                              "re-install any packages that you want to keep."),
			                [legacy_libloc](RKSetupWizard *) {
				                liblocs.get().removeAll(legacy_libloc);
				                RInterface::issueCommand(libLocsCommand(), RCommand::App);
			                });
			item->addOption(i18nc("verb", "Keep"), i18n("Keep this location (do not change anything)"), [](RKSetupWizard *) {});

			items->append(item);
		}
	}
}
