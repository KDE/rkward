/*
rkward.cpp - This file is part of RKWard (https://rkward.kde.org). Created: Tue Oct 29 2002
SPDX-FileCopyrightText: 2002-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkward.h"

// include files for QT
#include <qtimer.h>
#include <QCloseEvent>
#include <QPointer>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QComboBox>

// include files for KDE
#include <kmessagebox.h>
#include <kencodingfiledialog.h>
#include <KLocalizedString>
#include <kconfig.h>
#include <kstandardaction.h>
#include <kmultitabbar.h>
#include <ksqueezedtextlabel.h>
#include <kparts/partmanager.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <krecentfilesaction.h>
#include <ktoolbar.h>
#include <kactionmenu.h>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KColorScheme>
#include <kwidgetsaddons_version.h>
#include <KMessageWidget>

// application specific includes
#include "core/rkmodificationtracker.h"
#include "plugin/rkcomponentmap.h"
#include "settings/rksettings.h"
#include "settings/rksettingsmoduleplugins.h"
#include "settings/rksettingsmodulegeneral.h"
#include "settings/rksettingsmoduleoutput.h"
#include "settings/rksettingsmodulecommandeditor.h"
#include "settings/rksettingsmodulekateplugins.h"
#include "settings/rkrecenturls.h"
#include "rbackend/rkrinterface.h"
#include "core/robjectlist.h"
#include "core/renvironmentobject.h"
#include "misc/rkstandardicons.h"
#include "misc/rkcommandlineargs.h"
#include "misc/rkcommonfunctions.h"
#include "misc/rkxmlguisyncer.h"
#include "misc/rkdialogbuttonbox.h"
#include "misc/rkstyle.h"
#include "dialogs/rkloadlibsdialog.h"
#include "dialogs/rkimportdialog.h"
#include "dialogs/rkrecoverdialog.h"
#include "dialogs/rksetupwizard.h"
#include "dialogs/rksavemodifieddialog.h"
#include "agents/rksaveagent.h"
#include "agents/rkloadagent.h"
#include "agents/rkquitagent.h"
#include "windows/robjectbrowser.h"
#include "windows/rcontrolwindow.h"
#include "windows/rkhtmlwindow.h"
#include "windows/rkworkplaceview.h"
#include "windows/rkworkplace.h"
#include "windows/rkcommandlog.h"
#include "windows/rkhelpsearchwindow.h"
#include "windows/rktoplevelwindowgui.h"
#include "windows/rkfilebrowser.h"
#include "windows/rktoolwindowlist.h"
#include "windows/rkdebugconsole.h"
#include "windows/rkcallstackviewer.h"
#include "windows/rkdebugmessagewindow.h"
#include "windows/katepluginintegration.h"
#include "rkconsole.h"
#include "debug.h"

#include "agents/showedittextfileagent.h"	// TODO: see below: needed purely for linking!
#include "dialogs/rkreadlinedialog.h"	// TODO: see below: needed purely for linking!
#include "dialogs/rkselectlistdialog.h"	// TODO: see below: needed purely for linking!
#include "windows/detachedwindowcontainer.h"	// TODO: see below: needed purely for linking!
#include "dataeditor/rkeditordataframe.h"	// TODO: see below: needed purely for linking!
#include "agents/rkeditobjectagent.h"	// TODO: see below: needed purely for linking!
#include "agents/rkprintagent.h"	// TODO: see below: needed purely for linking!

// This nevers gets called. It's needed to trick ld into linking correctly. Nothing else.
void bogusCalls () {
#ifndef CPPCHECK_ONLY  // it would rightfully complain about some of these
	ShowEditTextFileAgent::showEditFiles(nullptr);                                   // TODO: AAAAAAAARGGGH!!!! It won't link without this bogus line!!!
	RKReadLineDialog::readLine(nullptr, QString(), QString(), nullptr, nullptr);     // TODO: see above
	RKSelectListDialog::doSelect(nullptr, QString(), QStringList(), QStringList(), false); // TODO: see above
	new RKEditorDataFrame(nullptr, nullptr);
	DetachedWindowContainer(nullptr, false);
	new RKWorkplaceView(nullptr);
	new RKEditObjectAgent(QStringList(), nullptr);
	RKPrintAgent::printPostscript(QString(), false);
#endif
}

/** Main window **/

//static
RKWardMainWindow *RKWardMainWindow::rkward_mainwin = nullptr;

RKWardMainWindow::RKWardMainWindow() : KParts::MainWindow() {
	RK_TRACE(APP);
	RK_ASSERT(rkward_mainwin == nullptr);

	Q_INIT_RESOURCE(resources);
	Q_INIT_RESOURCE(icons);
	testmode_suppress_dialogs = false;
	gui_rebuild_locked = true;
	no_ask_save = true;
	workspace_modified = false;
	merge_loads = false;
	rkward_mainwin = this;
	katepluginintegration = nullptr;
	active_ui_buddy = nullptr;
	RKCommonFunctions::getRKWardDataDir(); // call this before any forking, in order to avoid potential race conditions during initialization of data dir
	RKSettings::settings_tracker = new RKSettingsTracker (this);

	///////////////////////////////////////////////////////////////////
	// call inits to invoke all other construction parts
	RKStandardIcons::initIcons ();
	initActions();

	new RKWorkplace (this);
	RKWorkplace::mainWorkplace ()->initActions (actionCollection ());
	setCentralWidget (RKWorkplace::mainWorkplace ());
	connect (RKWorkplace::mainWorkplace ()->view (), &RKWorkplaceView::captionChanged, this, static_cast<void (RKWardMainWindow::*)(const QString&)>(&RKWardMainWindow::setCaption));
	connect (RKWorkplace::mainWorkplace (), &RKWorkplace::workspaceUrlChanged, this, [this](const QUrl &url) { RKRecentUrls::addRecentUrl(RKRecentUrls::workspaceId(), url); setCaption(QString()); });
	initStatusBar();

	part_manager = new KParts::PartManager (this);
	// When the manager says the active part changes,
	// the builder updates (recreates) the GUI
	connect (partManager (), &KParts::PartManager::activePartChanged, this, &RKWardMainWindow::partChanged);

	readOptions();
	new RKModificationTracker(this);
	initToolViewsAndR();

	///////////////////////////////////////////////////////////////////
	// build the interface

	setHelpMenuEnabled (false);
	setXMLFile ("rkwardui.rc");
	insertChildClient (toplevel_actions = new RKTopLevelWindowGUI (this));
	insertChildClient (katePluginIntegration ()->mainWindow ());
	createShellGUI (true);
	// This is pretty convoluted, but while loading plugins the katePluginIntegration-client may gain new actions and thus needs
	// to be reloaded. We cannot - currently, KF5.65 - delay loading the UI defintion(s), because plugins rely on it having a GUI factory.
	katePluginIntegration ()->loadPlugins (RKSettingsModuleKatePlugins::pluginsToLoad ());
// TODO: initToolWindowActions() should be called after all plugins are loaded (and have registered their tool views). However
//       that may be a problem, if there is no KXMLGUIFactory around, yet. So, annoyingly, we need to create the GUI, before we
//       have everything to populate it. Therefore, after init, the client is removed and re-added in order to trigger a UI
//       refresh.
	toplevel_actions->initToolWindowActions ();
	factory()->removeClient (toplevel_actions);
	factory()->addClient (toplevel_actions);
	RKXMLGUISyncer::self ()->watchXMLGUIClientUIrc (this);

	// replicate File->import and export menus into the Open/Save toolbar button menus
	QMenu *menu = dynamic_cast<QMenu*>(guiFactory ()->container ("import", this));
	if (menu) open_any_action->addAction (menu->menuAction ());
	menu = dynamic_cast<QMenu*>(guiFactory ()->container ("export", this));
	if (menu) save_any_action->addAction (menu->menuAction ());

	RKComponentMap::initialize ();

	// stuff which should wait until the event loop is running
	QTimer::singleShot(0, this, &RKWardMainWindow::doPostInit);
}

RKWardMainWindow::~RKWardMainWindow() {
	RK_TRACE (APP);

	// these would not be strictly necessary, as we're exiting the app, anyway. (TODO: some deleted as QObject children, anyway. Clean up.)
	delete RObjectList::getObjectList ();
	delete RObjectBrowser::mainBrowser ();
	delete RKCommandLog::getLog ();
	delete RKConsole::mainConsole ();
	delete RKHelpSearchWindow::mainHelpSearch ();
	delete RInterface::instance();
	delete RControlWindow::getControl ();
	factory ()->removeClient (RKComponentMap::getMap ());
	delete RKComponentMap::getMap ();
	delete RKStyle::_view_scheme;
}

KatePluginIntegrationApp* RKWardMainWindow::katePluginIntegration () {
	RK_TRACE (APP);

	if (!katepluginintegration) {
		katepluginintegration = new KatePluginIntegrationApp (this);
	}
	return katepluginintegration;
}

void RKWardMainWindow::closeEvent (QCloseEvent *e) {
	RK_TRACE (APP);

	if (RKQuitAgent::quittingInProgress ()) {
		KParts::MainWindow::closeEvent (e);
		return;
	}

	e->ignore();
	if (doQueryQuit()) {
		Q_EMIT aboutToQuitRKWard();
		new RKQuitAgent(this);
	}
}

bool RKWardMainWindow::event(QEvent *e) {
	if (e->type() == QEvent::ApplicationPaletteChange) {
		delete RKStyle::_view_scheme;
		RKStyle::_view_scheme = nullptr;
	}
	return KParts::MainWindow::event(e);
}

void RKWardMainWindow::doPostInit () {
	RK_TRACE (APP);

	QStringList open_urls = RKCommandLineArgs::get(RKCommandLineArgs::UrlArgs).toStringList();
	QString evaluate_code = RKCommandLineArgs::get(RKCommandLineArgs::Evaluate).toString();

	initPlugins ();
	gui_rebuild_locked = false;

	show ();
	if (!testmode_suppress_dialogs) RKSetupWizard::doAutoCheck();
	KMessageBox::enableMessage ("external_link_warning");  // can only be disabled per session

	QUrl recover_url = RKRecoverDialog::checkRecoverCrashedWorkspace ();
	if (!recover_url.isEmpty ()) {
		open_urls.clear ();    // Well, not a perfect solution. But we certainly don't want to overwrite the just recovered workspace.
		open_urls.append (recover_url.url ());
	}

	for (int i = 0; i < open_urls.size (); ++i) {
		// make sure local urls are absolute, as we may be changing wd before loading
		QUrl url = QUrl::fromUserInput (open_urls[i], QDir::currentPath(), QUrl::AssumeLocalFile);
		RK_ASSERT (!url.isRelative ());
		open_urls[i] = url.url ();
	}

	if (!open_urls.isEmpty()) {
		// this is also done when there are no urls specified on the command line. But in that case _after_ loading any workspace, so
		// the help window will be on top
		if (RKSettingsModuleGeneral::showHelpOnStartup ()) toplevel_actions->showRKWardHelp ();

		openUrlsFromCommandLineOrExternal(true, open_urls);
	} else {
		if (RKSettingsModuleGeneral::openRestoreFileOnLoad() && QFile::exists(".RData")) {
			// setNoAskSave(true); was called earlier
			askOpenWorkspace(QUrl::fromLocalFile(QFileInfo(".RData").absoluteFilePath()));
		}

		if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
			RKWorkplace::mainWorkplace ()->restoreWorkplace (RKSettingsModuleGeneral::getSavedWorkplace (KSharedConfig::openConfig ().data ()).split ('\n'));
		}
		if (RKSettingsModuleGeneral::showHelpOnStartup() && !testmode_suppress_dialogs) toplevel_actions->showRKWardHelp ();
	}
	setNoAskSave (false);

	// up to this point, no "real" save-worthy stuff can be pending in the backend. So mark this point as "clean".
	RCommand *command = new RCommand (QString (), RCommand::EmptyCommand | RCommand::Sync | RCommand::App);
	connect (command->notifier (), &RCommandNotifier::commandFinished, this, &RKWardMainWindow::setWorkspaceUnmodified);
	RInterface::issueCommand (command);

	if (!evaluate_code.isEmpty ()) RKConsole::pipeUserCommand (evaluate_code);

	updateCWD ();
	setCaption (QString ());	// our version of setCaption takes care of creating a correct caption, so we do not need to provide it here
}

void RKWardMainWindow::openUrlsFromCommandLineOrExternal(bool no_warn_external, QStringList _urls) {
	RK_TRACE (APP);

	bool any_dangerous_urls = false;
	QList<QUrl> urls;
	for (int i = 0; i < _urls.size (); ++i) {
		QUrl url = QUrl::fromUserInput (_urls[i], QString (), QUrl::AssumeLocalFile);
		if (url.scheme () == "rkward" && url.host () == "runplugin") {
			any_dangerous_urls = true;
		}
		urls.append (url);
	}

	// --nowarn-external, if used cross-process, must be set on the commandline in both this, *and* the calling process
	if (any_dangerous_urls && !(no_warn_external && RKCommandLineArgs::get(RKCommandLineArgs::NoWarnExternal).toBool())) {
		RK_ASSERT (urls.size () == 1);
		QString message = i18n ("<p>You are about to start an RKWard dialog from outside of RKWard, probably by clicking on an 'rkward://'-link, somewhere. In case you have found this link on an external website, please bear in mind that R can be used to run arbitrary commands on your computer, <b>potentially including downloading and installing malicious software</b>. If you do not trust the source of the link you were following, you should press 'Cancel', below.</p><p>In case you click 'Continue', no R code will be run, unless and until you click 'Submit' in the dialog window, and you are encouraged to review the generated R code, before doing so.</p><p><i>Note</i>: Checking 'Do not ask again' will suppress this message for the remainder of this session, only.");
		if (KMessageBox::warningContinueCancel (this, message, i18n ("A note on external links"), KStandardGuiItem::cont (), KStandardGuiItem::cancel (), "external_link_warning") != KMessageBox::Continue) return;
	}

	RKWardMainWindow::getMain ()->setMergeLoads (true);
	for (int i = 0; i < urls.size (); ++i) {
		RKWorkplace::mainWorkplace ()->openAnyUrl (urls[i], QString (), false);
	}
	RKWardMainWindow::getMain ()->setMergeLoads (false);
}

void RKWardMainWindow::initPlugins (const QStringList &automatically_added) {
	RK_TRACE (APP);
	slotSetStatusBarText(i18n("Setting up plugins..."));

	QStringList all_maps = RKSettingsModulePlugins::pluginMaps ();

	factory ()->removeClient (RKComponentMap::getMap ());
	RKComponentMap::getMap ()->clearAll ();

	QStringList completely_broken_maps;
	QStringList completely_broken_maps_details;
	QStringList somewhat_broken_maps;
	QStringList somewhat_broken_maps_details;
	for (int i = 0; i < all_maps.size (); ++i) {
		const QString &map = all_maps[i];
		RKPluginMapParseResult result = RKComponentMap::getMap ()->addPluginMap (map);
		if (!result.valid_plugins) {
			RKSettingsModulePlugins::markPluginMapState(map, RKSettingsModulePlugins::Broken);
			completely_broken_maps.append (map);
			completely_broken_maps_details.append (result.detailed_problems);
		} else if (!result.detailed_problems.isEmpty ()) {
			if (RKSettingsModulePlugins::markPluginMapState(map, RKSettingsModulePlugins::Quirky)) {
				somewhat_broken_maps.append (map);
				somewhat_broken_maps_details.append (result.detailed_problems);
			}
		} else {
			RKSettingsModulePlugins::markPluginMapState(map, RKSettingsModulePlugins::Working);
		}
	}

	RKComponentMap::getMap ()->finalizeAll ();
	factory ()->addClient (RKComponentMap::getMap ());

	if (!automatically_added.isEmpty ()) {
		// NOTE: When plugins are added from R, these must be fully initialized *before* showing any dialog, which is modal, i.e. has an event loop. Otherwise, subsequent calls e.g. to rk.call.plugin() could sneak in front of this.
		// This is the reason for handling notification about automatically_added plugins, here.
		KMessageBox::informationList (RKWardMainWindow::getMain (), i18n ("New RKWard plugin packs (listed below) have been found, and have been activated, automatically. To de-activate selected plugin packs, use Settings->Configure RKWard->Plugins."), automatically_added, i18n ("New plugins found"), "new_plugins_found");
	}
	if (!completely_broken_maps.isEmpty ()) {
		QString maplist = "<ul><li>" + completely_broken_maps.join ("</li>\n<li>") + "</li></ul>";
		KMessageBox::detailedError(nullptr, QString("<p>%1</p><p>%2</p>").arg(i18n("The following RKWard pluginmap files could not be loaded, and have been disabled. This could be because they are broken, not compatible with this version of RKWard, or not meant for direct loading (see the 'Details' for more information). They have been disabled."), maplist), completely_broken_maps_details.join ("\n"), i18n("Failed to load some plugin maps"));
	}
	if (!somewhat_broken_maps.isEmpty ()) {
		QString maplist = "<ul><li>" + somewhat_broken_maps.join ("</li>\n<li>") + "</li></ul>";
		KMessageBox::detailedError(nullptr, QString("<p>%1</p><p>%2</p><p>%3</p>").arg(i18n("Some errors were encountered while loading the following RKWard pluginmap files. This could be because individual plugins are broken or not compatible with this version of RKWard (see the 'Details' for more information). Other plugins were loaded, successfully, however."), maplist, i18n("Note: You will not be warned about these pluginmap files again, until you upgrade RKWard, or remove and re-add them in Settings->Configure RKWard->Plugins.")), somewhat_broken_maps_details.join("\n"), i18n("Failed to load some plugin maps"));
	}

	slotSetStatusReady ();
}

void RKWardMainWindow::startR () {
	RK_TRACE (APP);

	setRStatus(RInterface::Starting);
	// make sure our general purpose files directory exists
	QString packages_path = RKSettingsModuleGeneral::filesPath() + "/.rkward_packages";
	bool ok = QDir ().mkpath (packages_path);
	RK_ASSERT (ok);

	// Copy RKWard R source packages to general  purpose files directory (if still needed).
	// This may look redundant at first (since the package still needs to be installed from the
	// backend. However, if frontend and backend are on different machines (eventually), only  the
	// filesPath is shared between both.
	QStringList packages({"rkward.tgz", "rkwardtests.tgz"});
	for (int i = 0; i < packages.size (); ++i) {
		QString package = QDir (packages_path).absoluteFilePath (packages[i]);
		if (RKSettingsModuleGeneral::rkwardVersionChanged() || RKCommandLineArgs::get(RKCommandLineArgs::Setup).toBool()) {
			RK_DEBUG(APP, DL_INFO, "RKWard version changed or setup requested. Discarding cached package at %s", qPrintable(package));
			if(QFileInfo::exists(package)) RK_ASSERT(QFile::remove(package));
		}
		if (!QFileInfo::exists(package)) {
			QString source = RKCommonFunctions::getRKWardDataDir() + "/rpackages/" + packages[i];
			RK_ASSERT(QFileInfo::exists(source));
			RK_DEBUG(APP, DL_INFO, "Copying rkward R source package %s to %s", qPrintable(source), qPrintable(package));
			RK_ASSERT(QFile::copy(source, package));
		}
	}

	RInterface::create();
	Q_EMIT backendCreated();
	connect(RInterface::instance(), &RInterface::backendStatusChanged, this, &RKWardMainWindow::setRStatus);
	connect(RInterface::instance(), &RInterface::backendWorkdirChanged, this, &RKWardMainWindow::updateCWD);
	RObjectList::init();

	RObjectBrowser::mainBrowser ()->unlock ();
}

void RKWardMainWindow::slotConfigure () {
	RK_TRACE (APP);
	RKSettings::configureSettings (RKSettings::NoPage, this);
}

void RKWardMainWindow::slotCancelAllCommands () {
	RK_TRACE(APP);
	RK_ASSERT(RInterface::instance());
	RInterface::instance()->cancelAll();
}

void RKWardMainWindow::configureCarbonCopy () {
	RK_TRACE (APP);

	QDialog *dialog = new QDialog ();
	dialog->setWindowTitle (i18n ("Carbon Copy Settings"));
	QVBoxLayout *layout = new QVBoxLayout (dialog);
	RKCarbonCopySettings *settings = new RKCarbonCopySettings (dialog, nullptr);
	layout->addWidget (settings);

	RKDialogButtonBox *box = new RKDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel, dialog);
	dialog->setAttribute (Qt::WA_DeleteOnClose);
	connect (dialog, &QDialog::accepted, settings, &RKCarbonCopySettings::applyChanges);
	connect (box->button (QDialogButtonBox::Apply), &QPushButton::clicked, settings, &RKCarbonCopySettings::applyChanges);
	layout->addWidget (box);

	dialog->show ();
}

void RKWardMainWindow::initToolViewsAndR () {
	RK_TRACE (APP);

	RObjectBrowser::object_browser = new RObjectBrowser(nullptr, true);
	RObjectBrowser::mainBrowser ()->setCaption (i18n ("Workspace"));
	RKToolWindowList::registerToolWindow (RObjectBrowser::mainBrowser (), "workspace", RKToolWindowList::Left, Qt::AltModifier | Qt::Key_1);

	RKCommandLog::rkcommand_log = new RKCommandLog(nullptr, true);
	RKToolWindowList::registerToolWindow (RKCommandLog::rkcommand_log, "commandlog", RKToolWindowList::Bottom, Qt::AltModifier | Qt::Key_3);

	startR ();

	RKFileBrowser::main_browser = new RKFileBrowser(nullptr, true);
	RKFileBrowser::main_browser->setCaption (i18n ("Files"));
	RKToolWindowList::registerToolWindow (RKFileBrowser::main_browser, "filebrowser", RKToolWindowList::Left, Qt::AltModifier | Qt::Key_2);

	RControlWindow::control_window = new RControlWindow(nullptr, true);
	RControlWindow::getControl ()->setCaption (i18n ("Pending Jobs"));
	RKToolWindowList::registerToolWindow (RControlWindow::getControl (), "pendingjobs", RKToolWindowList::Nowhere, Qt::AltModifier | Qt::Key_4);

	RKConsole *console = new RKConsole(nullptr, true);
	RKConsole::setMainConsole (console);
	RKToolWindowList::registerToolWindow (console, "console", RKToolWindowList::Bottom, Qt::AltModifier | Qt::Key_5);

	RKHelpSearchWindow *help_search = new RKHelpSearchWindow(nullptr, true);
	RKHelpSearchWindow::main_help_search = help_search;
	RKToolWindowList::registerToolWindow (help_search, "helpsearch", RKToolWindowList::Bottom, Qt::AltModifier | Qt::Key_6);

	RKCallstackViewer::_instance = new RKCallstackViewer(nullptr, true);
	RKCallstackViewer::instance ()->setCaption (i18n ("Debugger Frames"));
	RKToolWindowList::registerToolWindow (RKCallstackViewer::instance (), "debugframes", RKToolWindowList::Right, Qt::AltModifier | Qt::Key_8);

	// HACK: Creating this _after_ the callstackviewer is important, so the debug console will end up the active window when entering a debug context
	RKDebugConsole::_instance = new RKDebugConsole(nullptr, true);
	RKDebugConsole::instance ()->setCaption (i18n ("Debugger Console"));
	RKToolWindowList::registerToolWindow (RKDebugConsole::instance (), "debugconsole", RKToolWindowList::Nowhere, Qt::AltModifier | Qt::Key_7);

	RKDebugMessageWindow::_instance = new RKDebugMessageWindow(nullptr, true);
	RKDebugMessageWindow::instance ()->setCaption (i18n ("RKWard Debug Messages"));
	RKToolWindowList::registerToolWindow (RKDebugMessageWindow::instance (), "rkdebugmessages", RKToolWindowList::Nowhere, QKeyCombination());

	RKWorkplace::mainWorkplace ()->placeToolWindows ();
}

void RKWardMainWindow::initActions() {  
	RK_TRACE (APP);
	QAction *action;

	// TODO: is there a way to insert actions between standard actions without having to give all standard actions custom ids?
	new_data_frame = actionCollection()->addAction("new_data_frame", this, &RKWardMainWindow::slotNewDataFrame);
	new_data_frame->setText (i18n ("Dataset"));
	new_data_frame->setIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowDataFrameEditor));
	new_data_frame->setWhatsThis(i18n ("Creates new empty dataset and opens it for editing"));

	new_command_editor = actionCollection()->addAction(KStandardAction::New, "new_command_editor", this, &RKWardMainWindow::slotNewCommandEditor);
	new_command_editor->setText (i18n ("Script File"));
	new_command_editor->setIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowCommandEditor));

	new_output = actionCollection()->addAction("new_output", this, &RKWardMainWindow::slotNewOutput);
	new_output->setText(i18n("Output document"));
	new_output->setIcon(RKStandardIcons::getIcon(RKStandardIcons::WindowOutput));
	new_output->setWhatsThis(i18n("Creates and activates a new output document"));

	fileOpenScript = actionCollection()->addAction(KStandardAction::Open, "file_open_script");
	connect(fileOpenScript, &QAction::triggered, this, [this]() { slotOpenCommandEditor(); });
	actionCollection()->setDefaultShortcut(fileOpenScript, Qt::ControlModifier | Qt::AltModifier | Qt::Key_O);
	fileOpenScript->setText(i18n("Open R Script File..."));

	fileOpenOutput = actionCollection()->addAction(KStandardAction::Open, "file_open_output", this, [this](){ slotOpenOutput(); });
	actionCollection()->setDefaultShortcut(fileOpenOutput, QKeySequence());
	fileOpenOutput->setText(i18n("Open RKWard Output File..."));

	QAction *file_open_any = actionCollection ()->addAction (KStandardAction::Open, "file_open_any");
	connect (file_open_any, &QAction::triggered, this, &RKWardMainWindow::openAnyFile);
	file_open_any->setText (i18n ("Open any File..."));

	fileOpenRecent = RKRecentUrls::claimAction(RKRecentUrls::scriptsId());
	actionCollection ()->addAction("file_open_recenty", fileOpenRecent);
	connect(fileOpenRecent, &KRecentFilesAction::urlSelected, this, [this](const QUrl &url) { slotOpenCommandEditor(url); });
	fileOpenRecent->setText(i18n("Open Recent R Script File"));

	action = actionCollection()->addAction("import_data");
	connect(action, &QAction::triggered, this, &RKWardMainWindow::importData);
	action->setText(i18n("Import Assistant"));
	action->setWhatsThis(i18n("Assistant to find the best method to import data from a variety of formats"));

	fileOpenWorkspace = actionCollection()->addAction(KStandardAction::Open, "file_openx");
	connect(fileOpenWorkspace, &QAction::triggered, this, [this](){ askOpenWorkspace(); });
	fileOpenWorkspace->setText (i18n ("Open Workspace..."));
	actionCollection ()->setDefaultShortcut (fileOpenWorkspace, Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_O);
	fileOpenWorkspace->setWhatsThis(i18n ("Opens an existing document"));

	fileOpenRecentWorkspace = RKRecentUrls::claimAction(RKRecentUrls::workspaceId());
	actionCollection ()->addAction("file_open_recentx", fileOpenRecentWorkspace);
	connect(fileOpenRecentWorkspace, &KRecentFilesAction::urlSelected, this, &RKWardMainWindow::askOpenWorkspace);
	fileOpenRecentWorkspace->setText (i18n ("Open Recent Workspace"));
	fileOpenRecentWorkspace->setWhatsThis(i18n ("Opens a recently used file"));

	fileSaveWorkspace = actionCollection()->addAction(KStandardAction::Save, "file_savex", this, [](){ RKSaveAgent::saveWorkspace(); });
	fileSaveWorkspace->setText (i18n ("Save Workspace"));
	actionCollection ()->setDefaultShortcut (fileSaveWorkspace, Qt::ControlModifier | Qt::AltModifier | Qt::Key_S);
	fileSaveWorkspace->setWhatsThis(i18n ("Saves the actual document"));

	fileSaveWorkspaceAs = actionCollection()->addAction(KStandardAction::SaveAs, "file_save_asx", this, [](){ RKSaveAgent::saveWorkspaceAs(); });
	actionCollection ()->setDefaultShortcut (fileSaveWorkspaceAs, Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier | Qt::Key_S);
	fileSaveWorkspaceAs->setText (i18n ("Save Workspace As"));
	fileSaveWorkspaceAs->setWhatsThis(i18n ("Saves the actual document as..."));

	fileQuit = actionCollection()->addAction(KStandardAction::Quit, "file_quitx", this, &RKWardMainWindow::close);
	fileQuit->setWhatsThis(i18n ("Quits the application"));

	interrupt_all_commands = actionCollection()->addAction("cancel_all_commands", this, &RKWardMainWindow::slotCancelAllCommands);
	interrupt_all_commands->setText (i18n ("Interrupt all commands"));
	actionCollection ()->setDefaultShortcut (interrupt_all_commands, Qt::ShiftModifier | Qt::Key_Escape);
	interrupt_all_commands->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionInterrupt));
	interrupt_all_commands->setEnabled (false);		// enabled from within setRStatus()

	action = actionCollection()->addAction("carbon_copy", this, &RKWardMainWindow::configureCarbonCopy);
	action->setText (i18n ("CC commands to output..."));

	// These two currently do the same thing
	action = actionCollection()->addAction("load_unload_libs", this, &RKWardMainWindow::slotFileLoadLibs); // TODO: set page
	action->setText (i18n ("Manage R packages and plugins..."));
	action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionConfigurePackages));

	action = actionCollection()->addAction("configure_packages", this, &RKWardMainWindow::slotFileLoadLibs); // TODO: set page
	action->setText (i18n ("Manage R packages and plugins..."));
	action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionConfigurePackages));

	setStandardToolBarMenuEnabled (true);
	createStandardStatusBarAction ();

	close_all_editors = actionCollection()->addAction("close_all_editors", this, &RKWardMainWindow::slotCloseAllEditors);
	close_all_editors->setText (i18n ("Close All Data"));
	close_all_editors->setWhatsThis(i18n ("Closes all open data editors"));

	action = actionCollection()->addAction(KStandardAction::Close, "window_close", this, &RKWardMainWindow::slotCloseWindow);

	window_close_all = actionCollection()->addAction("window_close_all", this, &RKWardMainWindow::slotCloseAllWindows);
	window_close_all->setText (i18n ("Close All"));

	window_detach = actionCollection()->addAction("window_detach", this, &RKWardMainWindow::slotDetachWindow);
	window_detach->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionDetachWindow));
	window_detach->setText (i18n ("Detach"));

	configure = actionCollection()->addAction(KStandardAction::Preferences, "options_configure", this, &RKWardMainWindow::slotConfigure);
	action = actionCollection()->addAction("setup_wizard");
	action->setText(i18n("Check installation"));
	connect(action, &QAction::triggered, &RKSetupWizard::manualCheck);

	edit_menu_dummy = actionCollection ()->addAction ("edit_menu_dummy", this);
	edit_menu_dummy->setText (i18n ("[No actions available for current view]"));
	edit_menu_dummy->setEnabled (false);
	view_menu_dummy = actionCollection ()->addAction ("view_menu_dummy", this);
	view_menu_dummy->setText (edit_menu_dummy->text ());
	view_menu_dummy->setEnabled (false);

	// collections for the toolbar:
	open_any_action = new KActionMenu (QIcon::fromTheme("document-open-folder"), i18n ("Open..."), this);
	actionCollection ()->addAction ("open_any", open_any_action);

	open_any_action->addAction (fileOpenWorkspace);
	open_any_action->addAction (fileOpenRecentWorkspace);
	open_any_action->addSeparator ();
	open_any_action->addAction (fileOpenScript);
	open_any_action->addAction (fileOpenOutput);
	open_any_action->addSeparator ();
	open_any_action->addAction (fileOpenRecent);
	open_any_action->addAction (file_open_any);
	open_any_action->addSeparator ();
	//open_any_action->addAction (proxy_import); -> later

	KActionMenu* new_any_action = new KActionMenu (QIcon::fromTheme("document-new"), i18n ("Create..."), this);
	actionCollection ()->addAction ("new_any", new_any_action);

	new_any_action->addAction (new_data_frame);
	new_any_action->addAction (new_command_editor);
	new_any_action->addAction (new_output);

	save_any_action = new KActionMenu (QIcon::fromTheme("document-save"), i18n ("Save..."), this);
	actionCollection ()->addAction ("save_any", save_any_action);

	open_any_action->setPopupMode(QToolButton::InstantPopup);
	new_any_action->setPopupMode(QToolButton::InstantPopup);
	save_any_action->setPopupMode(QToolButton::InstantPopup);

	save_any_action->addAction (fileSaveWorkspace);
	save_any_action->addAction (fileSaveWorkspaceAs);
	save_any_action->addSeparator ();
// TODO: A way to add R-script-save actions, dynamically, would be nice
	save_actions_plug_point = save_any_action->addSeparator ();
	//save_any_action->addAction (proxy_export); -> later

	restart_r = actionCollection()->addAction("restart_r");
	restart_r->setIcon(QIcon::fromTheme("view-refresh"));
	restart_r->setText(i18n("Restart R Backend"));
	connect(restart_r, &QAction::triggered, this, &RKWardMainWindow::triggerBackendRestart);
}

bool RKWardMainWindow::triggerBackendRestart(bool promptsave) {
	RK_TRACE (APP);

	promptsave = promptsave && !suppressModalDialogsForTesting();
	QString message = i18n("<p><b>Restarting the backend will discard all unsaved data in this workspace!</b> All data editors, and graphics windows will be closed.</p><p>Are you sure you want to proceed?</p>");
	if (promptsave && (KMessageBox::warningContinueCancel(this, message, i18n("Restart R backend"), KGuiItem(i18n("Restart R backend"))) != KMessageBox::Continue)) {
		return false;
	}

	bool forced = RInterface::instance()->backendIsDead();
	while (!RInterface::instance()->backendIsDead() && !RInterface::instance()->backendIsIdle()) {
		RK_DEBUG(APP, DL_DEBUG, "Backend not idle while restart requested.");
		message = i18n("<p>One or more operations are pending.</p><p>If you have recently chosen to save your workspace, and you see this message, <b>your data may not be saved, yet!</b></p><p>How do you want to proceed?</p>");
		auto res = KMessageBox::warningTwoActionsCancel(this, message, i18n("R commands still pending"), KGuiItem(i18n("Force restart now")), KGuiItem(i18n("Check again")), KGuiItem(i18n("Cancel restarting")));
		if (res == KMessageBox::PrimaryAction) {
			forced = true;
			break;
		} else if (res == KMessageBox::Cancel) {
			return false;
		}
		// KMessageBox::No means to loop: Commands may have finished, meanwhile. This is not really pretty, a proper progress control would be nicer,
		// but then, this is a corner case to a feature that is not really targetted at a mainstream audience, anyway.
	}

	RKWorkplace::mainWorkplace()->closeAll(RKMDIWindow::X11Window);
	slotCloseAllEditors();
	auto restart_now = [this]() {
		RK_DEBUG(APP, DL_DEBUG, "Backend restart now");
		delete RInterface::instance();  // NOTE: Do not use deleteLater(), here. It is important to fully tear down the old backend, before creating the new one,
						//       as code is written around the assumption that RInterface and friends are singletons. (RInterface::instance(), etc.)
		RKWorkplace::mainWorkplace()->setWorkspaceURL(QUrl());
		startR();
	};
	if (forced) {
		RKConsole::mainConsole()->resetConsole();
		restart_now();
	} else {
		RCommand *c = new RCommand(QString("# Quit (restarting)"), RCommand::App | RCommand::EmptyCommand | RCommand::QuitCommand);
		c->whenFinished(this, [this, restart_now]() { QTimer::singleShot(0, this, restart_now); });
		RInterface::issueCommand(c);
	}

	return true;
}

/*
// debug code: prints out all current actions
void printActionsRecursive (QAction* action, const QString &prefix) {
	if (action->menu ()) {
		for (QAction *a : action->menu ()->actions ()) printActionsRecursive (a, prefix + action->text () + "->");
	} else {
		qDebug ("%s", qPrintable (prefix + action->text ()));
	}
}
*/

void updateEmptyMenuIndicator (QAction* indicator, const QMenu *menu) {
	if (!menu) {
		indicator->setVisible (false);
		return;
	}

	// NOTE: QMenu::isEmpty () does not work, here
	QList<QAction *> actions = menu->actions ();
	for (int i = 0; i < actions.size (); ++i) {
		if (actions[i] == indicator) continue;
		if (actions[i]->isSeparator ()) continue;
		if (actions[i]->isVisible ()) {
			indicator->setVisible (false);
			return;
		}
	}

	indicator->setVisible (true);
}

void RKWardMainWindow::partChanged(KParts::Part *part) {
	RK_TRACE (APP);

	if (gui_rebuild_locked) return;
	if (!part) return;
	createGUI(part);
	const auto actionsList = actions();
	for (QAction *a : actionsList) {
		if (a->statusTip().isEmpty()) a->setStatusTip(a->whatsThis());
	}

	if (!guiFactory()) {
		RK_ASSERT (false);
		return;
	}
	// Check for additional "buddy" that should be loaded
	RKMDIWindow *w = RKWorkplace::mainWorkplace()->windowForPart(part);
	KXMLGUIClient* buddy = w->uiBuddy();
	// NOTE: Even if the buddy stays the same, we have to remove and re-add it to keep the order of menu actions stable
	if (active_ui_buddy) factory()->removeClient(active_ui_buddy);
	if (buddy) factory()->addClient(buddy);
	active_ui_buddy = buddy;

	updateEmptyMenuIndicator(edit_menu_dummy, dynamic_cast<QMenu*>(guiFactory()->container("edit", this)));
	updateEmptyMenuIndicator(view_menu_dummy, dynamic_cast<QMenu*>(guiFactory()->container("view", this)));

	// plug save file actions into the toolbar collections
	RK_ASSERT(save_any_action);
	for (int i = 0; i < plugged_save_actions.size(); ++i) {
		QAction* a = plugged_save_actions[i].data();
		if (a) save_any_action->removeAction(a);
	}
	plugged_save_actions.clear ();

	if (w) {
		QAction *a = w->fileSaveAction ();
		if (a) plugged_save_actions.append (a);
		a = w->fileSaveAsAction ();
		if (a) plugged_save_actions.append (a);
	}
	for (int i = 0; i < plugged_save_actions.size(); ++i) {
		save_any_action->insertAction(save_actions_plug_point, plugged_save_actions[i]);
	}
/*
	// debug code: prints out all current actions
	for (QAction *action : menuBar ()->actions ()) printActionsRecursive (action, QString ());
*/
}

void RKWardMainWindow::lockGUIRebuild (bool lock) {
	RK_TRACE (APP);

	if (lock) {
		RK_ASSERT (!gui_rebuild_locked);
		gui_rebuild_locked = true;
	} else {
		gui_rebuild_locked = false;
		partChanged (part_manager->activePart ());
	}
}

void RKWardMainWindow::initStatusBar () {
	RK_TRACE (APP);

	// The regular status bar is broken in KF5 (since when? At least 5.68.0), but also it takes up too much space.
	// Instead, we use a right-aligned bar merged into the bottom toolbar -> no space wasted.
	//statusBar()->hide(); -> after geomtry has been set
	auto realbar = RKWorkplace::mainWorkplace()->statusBar();
	connect(statusBar(), &QStatusBar::messageChanged, this, [this](const QString &message) {
		if(message.isEmpty()) updateCWD();
		else statusbar_cwd->setText(message);
		// realbar->showMessage(message);  // why doesn't this work, instead? Qt 5.12.8
	});

	statusbar_cwd = new KSqueezedTextLabel();
	statusbar_cwd->setAlignment(Qt::AlignRight);
	statusbar_cwd->setToolTip(i18n("Current working directory"));
	realbar->addWidget(statusbar_cwd, 10);
	updateCWD();

	auto box = new QWidget();
	QHBoxLayout *boxl = new QHBoxLayout(box);
	boxl->setSpacing(0);
	statusbar_r_status = new QLabel("&nbsp;<b>R</b>&nbsp;");
	statusbar_r_status->setFixedHeight(realbar->fontMetrics().height() + 2);
	boxl->addWidget(statusbar_r_status);

	QToolButton* dummy = new QToolButton();
	dummy->setPopupMode(QToolButton::InstantPopup);
	dummy->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionShowMenu));
	dummy->setMenu(new QMenu(dummy));
	dummy->menu()->addAction(interrupt_all_commands);
	dummy->menu()->addAction(restart_r);
	dummy->menu()->addSeparator();
	QAction *a = new QAction(i18n("Configure R backend"), this);
	connect(a, &QAction::triggered, this, []() { RKSettings::configureSettings(RKSettings::PageR); });
	dummy->menu()->addAction(a);
	dummy->setFixedHeight(statusbar_r_status->height());
	boxl->addWidget(dummy);

	realbar->addPermanentWidget(box, 0);
}

void RKWardMainWindow::saveOptions () {
	RK_TRACE (APP);
	KSharedConfig::Ptr config = KSharedConfig::openConfig ();

	KConfigGroup cg = config->group ("main window options");
	saveMainWindowSettings (cg);

	cg = config->group ("General Options");
// TODO: WORKAROUND. See corresponding line in readOptions ()
	cg.writeEntry("Geometry", size ());

	RKRecentUrls::saveConfig();
	RKSettings::saveSettings(config.data());

	config->sync ();
}


void RKWardMainWindow::readOptions () {
	RK_TRACE (APP);

	KSharedConfig::Ptr config = KSharedConfig::openConfig ();

	applyMainWindowSettings (config->group ("main window options"));

// TODO: WORKAROUND: Actually applyMainWindowSettings could/should do this, but apparently this just does not work for maximized windows. Therefore we use our own version instead.
// KDE4: still needed?
// KF5 TODO: still needed?
	KConfigGroup cg = config->group ("General Options");
	QSize size = cg.readEntry ("Geometry", QSize ());
	if (size.isEmpty ()) {
		size = screen() ? screen()->availableGeometry().size() : QApplication::primaryScreen()->availableGeometry().size();
	}
	resize (size);

	RKSettings::loadSettings (config.data ());

	statusBar()->hide();
}


bool RKWardMainWindow::doQueryQuit () {
	RK_TRACE (APP);

	slotSetStatusBarText (i18n ("Exiting..."));
	saveOptions ();
	// TODO: This may not be correct, some urls may still change while closing Workspace (due to save as)!
	if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
		RKSettingsModuleGeneral::setSavedWorkplace (RKWorkplace::mainWorkplace ()->makeWorkplaceDescription ().join ("\n"), KSharedConfig::openConfig ().data ());
	}
	if (!RKWorkplace::mainWorkplace()->closeWorkspace()) {
		slotSetStatusReady ();
		return false;
	}

	return true;
}

void RKWardMainWindow::slotNewDataFrame () {
	RK_TRACE (APP);
	bool ok;

	QString name = QInputDialog::getText (this, i18n ("New dataset"), i18n ("Enter name for the new dataset"), QLineEdit::Normal, "my.data", &ok);

	if (ok) RKWorkplace::mainWorkplace ()->editNewDataFrame (name);
}

void RKWardMainWindow::askOpenWorkspace (const QUrl &url) {
	RK_TRACE (APP);

	if (!no_ask_save && !merge_loads) {
		if (!RKWorkplace::mainWorkplace()->closeWorkspace()) return;
	}

	slotSetStatusBarText(i18n("Opening workspace..."));
	QUrl lurl = url;
	if (lurl.isEmpty ()) {
		lurl = QFileDialog::getOpenFileUrl(this, i18n("Select workspace to open..."), RKRecentUrls::mostRecentUrl(RKRecentUrls::workspaceId()).adjusted(QUrl::RemoveFilename), i18n("R Workspace Files [%1](%1);;All files [*](*)", RKSettingsModuleGeneral::workspaceFilenameFilter()));
	}
	if (!lurl.isEmpty ()) {
		new RKLoadAgent(lurl, merge_loads);
	}
	slotSetStatusReady();
}

void RKWardMainWindow::slotFileLoadLibs () {
	RK_TRACE (APP);
	RKLoadLibsDialog *dial = new RKLoadLibsDialog(this, nullptr);
	dial->show ();
}

void RKWardMainWindow::updateCWD () {
	RK_TRACE (APP);

	statusbar_cwd->setText (QDir::currentPath ());
}

void RKWardMainWindow::slotSetStatusBarText (const QString &text) {
	RK_TRACE (APP);

//KDE4: still needed?
	QString ntext = text.trimmed ();
	ntext.replace ("<qt>", QString ());	// WORKAROUND: what the ?!? is going on? The KTHMLPart seems to post such messages.
	if (ntext.isEmpty ()) {
		statusBar ()->clearMessage ();
	} else {
		statusBar ()->showMessage (ntext);
	}
}

void RKWardMainWindow::slotCloseWindow () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->closeActiveWindow ();
}

void RKWardMainWindow::slotCloseAllWindows () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->closeAll ();
}

void RKWardMainWindow::slotCloseAllEditors () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->closeAll (RKMDIWindow::DataEditorWindow);
}

void RKWardMainWindow::slotDetachWindow () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->detachWindow (RKWorkplace::mainWorkplace ()->activeWindow (RKMDIWindow::Attached));
}

void setIndictatorColor(QWidget *widget, KColorScheme::ForegroundRole fg, KColorScheme::BackgroundRole bg) {
	QPalette palette = widget->palette();
	palette.setBrush(widget->backgroundRole(), RKStyle::viewScheme()->background(bg));
	palette.setBrush(widget->foregroundRole(), RKStyle::viewScheme()->foreground(fg));
	widget->setAutoFillBackground(true);
	widget->setPalette(palette);
}

void RKWardMainWindow::setRStatus (int status) {
	RK_TRACE (APP);
	static KMessageWidget* rstatus_message = nullptr;

	if (status == RInterface::Busy) {
		setIndictatorColor(statusbar_r_status, KColorScheme::NegativeText, KColorScheme::NegativeBackground);
		statusbar_r_status->setToolTip(i18n("The <b>R</b> engine is busy."));
		interrupt_all_commands->setEnabled(true);
	} else if (status == RInterface::Idle) {
		setIndictatorColor(statusbar_r_status, KColorScheme::PositiveText, KColorScheme::PositiveBackground);
		statusbar_r_status->setToolTip(i18n("The <b>R</b> engine is idle."));
		interrupt_all_commands->setEnabled(false);
	} else if (status == RInterface::Starting) {
		if (rstatus_message) rstatus_message->hide();
		setIndictatorColor(statusbar_r_status, KColorScheme::NeutralText, KColorScheme::NeutralBackground);
		statusbar_r_status->setToolTip(i18n("The <b>R</b> engine is being initialized."));
	} else {
		setIndictatorColor(statusbar_r_status, KColorScheme::NegativeText, KColorScheme::NegativeBackground);
		statusbar_r_status->setToolTip(i18n("The <b>R</b> engine is unavailable."));
		interrupt_all_commands->setEnabled(false);
		if (RInterface::instance()->backendFailedToStart()) {
			RKSetupWizard::doAutoCheck();  // The wizard itself will prevent recursion, if alread active
		}
		if (!rstatus_message) {
			rstatus_message = new KMessageWidget(i18n("R engine unavailable. See <a href=\"rkward://page/rkward_trouble_shooting\">troubleshooting</a> for possible solutions."));
			rstatus_message->setMessageType(KMessageWidget::Error);
			rstatus_message->setCloseButtonVisible(false);
			connect(rstatus_message, &KMessageWidget::linkActivated, this, [](const QString& url) { RKWorkplace::mainWorkplace()->openAnyUrl(QUrl(url)); });
			RKWorkplace::mainWorkplace()->addMessageWidget(rstatus_message);
		}
	}
}

void RKWardMainWindow::importData() {
	RK_TRACE (APP);

	RKImportDialog d("import", this);
	d.exec();
}

void RKWardMainWindow::slotNewCommandEditor () {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace()->openScriptEditor();
}

void RKWardMainWindow::openAnyFile () {
	RK_TRACE (APP);

	QFileDialog* dialog = new QFileDialog(nullptr, QString(), RKRecentUrls::mostRecentUrl(RKRecentUrls::scriptsId()).adjusted(QUrl::RemoveFilename).toLocalFile(), QString ("*|All Files (*)\n%1|R Script Files (%1)").arg(RKSettingsModuleCommandEditor::scriptFileFilter()));
	dialog->setFileMode(QFileDialog::ExistingFiles);

// Create a type selection widget, and hack it into the dialog:
	QFrame* dummy = new QFrame (this);
	dummy->setWindowTitle (i18n ("Open"));
	QVBoxLayout* layout = new QVBoxLayout (dummy);
	QHBoxLayout* hbox = new QHBoxLayout;
	layout->addLayout (hbox);
	hbox->addWidget (new QLabel (i18n ("Opening mode:")));
	QComboBox* open_mode = new QComboBox;
	open_mode->addItems (QStringList () << i18n ("Guess file type, automatically") << i18n ("Open as text / script file") << i18n ("Open as text file and force R highlighting") << ("Open as R workspace"));
	hbox->addWidget (open_mode);
	hbox->setStretchFactor (open_mode, 100);

	dummy->setWindowFlags (dialog->windowFlags ());
	dialog->setOption (QFileDialog::DontUseNativeDialog);
	dialog->setWindowFlags (Qt::Widget);
	layout->addWidget (dialog);
	dummy->show ();
	auto res = dialog->exec ();
	QUrl url = QUrl::fromLocalFile (dialog->selectedFiles ().value (0));
	int mode = open_mode->currentIndex ();
	delete dummy;
	if (res != QDialog::Accepted) return;

	if (mode == 0) {
		RKWorkplace::mainWorkplace ()->openAnyUrl (url);
	} else if (mode == 1) {
		RKWorkplace::mainWorkplace ()->openScriptEditor (url);
	} else if (mode == 2) {
		RKWorkplace::mainWorkplace ()->openScriptEditor (url, QString (), RKCommandEditorFlags::DefaultFlags | RKCommandEditorFlags::ForceRHighlighting);
	} else if (mode == 3) {
		askOpenWorkspace(url);
	}
}

void RKWardMainWindow::slotNewOutput() {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace()->openOutputWindow(QUrl(), true);
}

void RKWardMainWindow::slotOpenOutput(const QUrl &_url) {
	RK_TRACE (APP);

	QUrl url(_url);
	if (url.isEmpty()) {
		url = QFileDialog::getOpenFileUrl(this, i18n("Select RKWard Output file to open..."), RKRecentUrls::mostRecentUrl(RKRecentUrls::outputId()).adjusted(QUrl::RemoveFilename), i18n("RKWard Output Files [*.rko](*.rko);;All files [*](*)"));
	}
	RKWorkplace::mainWorkplace()->openOutputWindow(url);
}

void RKWardMainWindow::slotOpenCommandEditor (const QUrl &url, const QString &encoding) {
	RK_TRACE (APP);

	if (url.isEmpty()) {
		auto res = KEncodingFileDialog::getOpenUrlsAndEncoding(QString(), RKRecentUrls::mostRecentUrl(RKRecentUrls::scriptsId()).adjusted(QUrl::RemoveFilename), QString("%1|R Script Files (%1)\n*|All Files (*)").arg(RKSettingsModuleCommandEditor::scriptFileFilter()), this, i18n("Open script file(s)"));
		for (int i = 0; i < res.URLs.size(); ++i) {
				RKWorkplace::mainWorkplace ()->openScriptEditor(res.URLs[i], res.encoding);
		}
	} else {
		RKWorkplace::mainWorkplace ()->openScriptEditor(url, encoding);
	}
}

void RKWardMainWindow::setCaption (const QString &) {
	RK_TRACE (APP);

	QString wcaption = RKWorkplace::mainWorkplace ()->workspaceURL ().fileName ();
	if (wcaption.isEmpty ()) wcaption = RKWorkplace::mainWorkplace ()->workspaceURL ().toDisplayString ();
	if (wcaption.isEmpty ()) wcaption = i18n ("[Unnamed Workspace]");
	RKMDIWindow *window = RKWorkplace::mainWorkplace ()->view ()->activePage ();
	if (window) wcaption.append (" - " + window->fullCaption ());
	KParts::MainWindow::setCaption (wcaption);
}


