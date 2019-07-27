/***************************************************************************
                          rkward.cpp  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002
    copyright            : (C) 2002-2019 by Thomas Friedrichsmeier
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

#include "rkward.h"

// include files for QT
#include <qtimer.h>
#include <QDesktopWidget>
#include <QCloseEvent>
#include <QPointer>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>

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

// application specific includes
#include "core/rkmodificationtracker.h"
#include "plugin/rkcomponentmap.h"
#include "settings/rksettings.h"
#include "settings/rksettingsmoduleplugins.h"
#include "settings/rksettingsmodulegeneral.h"
#include "settings/rksettingsmoduleoutput.h"
#include "settings/rksettingsmodulecommandeditor.h"
#include "rbackend/rkrinterface.h"
#include "core/robjectlist.h"
#include "core/renvironmentobject.h"
#include "misc/rkstandardicons.h"
#include "misc/rkcommonfunctions.h"
#include "misc/rkxmlguisyncer.h"
#include "misc/rkdbusapi.h"
#include "misc/rkdialogbuttonbox.h"
#include "rkglobals.h"
#include "dialogs/startupdialog.h"
#include "dialogs/rkloadlibsdialog.h"
#include "dialogs/rkimportdialog.h"
#include "dialogs/rkrecoverdialog.h"
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
	ShowEditTextFileAgent::showEditFiles (0);		// TODO: AAAAAAAARGGGH!!!! It won't link without this bogus line!!!
	RKReadLineDialog::readLine (0, QString(), QString(), 0, 0);	// TODO: see above
	RKSelectListDialog::doSelect (0, QString(), QStringList(), QStringList(), false);	// TODO: see above
	new RKEditorDataFrame (0, 0);
	DetachedWindowContainer (0, false);
	new RKWorkplaceView (0);
	new RKEditObjectAgent (QStringList (), 0);
	RKPrintAgent::printPostscript (QString (), false);
}

/** Main window **/

//static
RKWardMainWindow *RKWardMainWindow::rkward_mainwin = 0;

RKWardMainWindow::RKWardMainWindow () : KParts::MainWindow ((QWidget *)0, (Qt::WindowFlags) KDE_DEFAULT_WINDOWFLAGS) {
	RK_TRACE (APP);
	RK_ASSERT (rkward_mainwin == 0);

	gui_rebuild_locked = true;
	no_ask_save = true;
	workspace_modified = false;
	merge_loads = false;
	rkward_mainwin = this;
	RKGlobals::rinter = 0;
	RKCommonFunctions::getRKWardDataDir(); // call this before any forking, in order to avoid potential race conditions during initialization of data dir
	RKSettings::settings_tracker = new RKSettingsTracker (this);

	///////////////////////////////////////////////////////////////////
	// call inits to invoke all other construction parts
	RKStandardIcons::initIcons ();
	initActions();
	initStatusBar();

	new RKWorkplace (this);
	RKWorkplace::mainWorkplace ()->initActions (actionCollection ());
	setCentralWidget (RKWorkplace::mainWorkplace ());
	connect (RKWorkplace::mainWorkplace ()->view (), &RKWorkplaceView::captionChanged, this, static_cast<void (RKWardMainWindow::*)(const QString&)>(&RKWardMainWindow::setCaption));
	connect (RKWorkplace::mainWorkplace (), &RKWorkplace::workspaceUrlChanged, this, &RKWardMainWindow::addWorkspaceUrl);

	part_manager = new KParts::PartManager (this);
	// When the manager says the active part changes,
	// the builder updates (recreates) the GUI
	connect (partManager (), &KParts::PartManager::activePartChanged, this, &RKWardMainWindow::partChanged);

	readOptions();
	RKGlobals::mtracker = new RKModificationTracker (this);
	initToolViewsAndR ();

	///////////////////////////////////////////////////////////////////
	// build the interface

	setHelpMenuEnabled (false);
	setXMLFile ("rkwardui.rc");
	insertChildClient (toplevel_actions = new RKTopLevelWindowGUI (this));
	createShellGUI (true);
	RKXMLGUISyncer::self ()->watchXMLGUIClientUIrc (this);

	// replicate File->import and export menus into the Open/Save toolbar button menus
	QMenu *menu = dynamic_cast<QMenu*>(guiFactory ()->container ("import", this));
	if (menu) open_any_action->addAction (menu->menuAction ());
	menu = dynamic_cast<QMenu*>(guiFactory ()->container ("export", this));
	if (menu) save_any_action->addAction (menu->menuAction ());

	RKComponentMap::initialize ();

	// stuff which should wait until the event loop is running
	QTimer::singleShot (0, this, SLOT (doPostInit()));
}

RKWardMainWindow::~RKWardMainWindow() {
	RK_TRACE (APP);

	// these would not be strictly necessary, as we're exiting the app, anyway.
	delete RObjectList::getObjectList ();
	delete RObjectBrowser::mainBrowser ();
	delete RKCommandLog::getLog ();
	delete RKConsole::mainConsole ();
	delete RKHelpSearchWindow::mainHelpSearch ();
	delete RKGlobals::tracker ();
	delete RKGlobals::rInterface ();
	delete RControlWindow::getControl ();
	factory ()->removeClient (RKComponentMap::getMap ());
	delete RKComponentMap::getMap ();
}

void RKWardMainWindow::closeEvent (QCloseEvent *e) {
	RK_TRACE (APP);

	if (RKQuitAgent::quittingInProgress ()) {
		KParts::MainWindow::closeEvent (e);
		return;
	}

	e->ignore ();
	if (doQueryQuit ()) {
		emit (aboutToQuitRKWard());
		new RKQuitAgent (this);
	}
}

void RKWardMainWindow::doPostInit () {
	RK_TRACE (APP);

	// Check installation first
	if (RKCommonFunctions::getRKWardDataDir ().isEmpty ()) {
		KMessageBox::error (this, i18n ("<p>RKWard either could not find its resource files at all, or only an old version of those files. The most likely cause is that the last installation failed to place the files in the correct place. This can lead to all sorts of problems, from single missing features to complete failure to function.</p><p><b>You should quit RKWard, now, and fix your installation</b>. For help with that, see <a href=\"http://rkward.kde.org/compiling\">http://rkward.kde.org/compiling</a>.</p>"), i18n ("Broken installation"), KMessageBox::Notify | KMessageBox::AllowLink);
	}

	QStringList open_urls = RKGlobals::startup_options.take ("initial_urls").toStringList ();
	bool warn_external = RKGlobals::startup_options.take ("warn_external").toBool ();
	QString evaluate_code = RKGlobals::startup_options.take ("evaluate").toString ();

	initPlugins ();
	gui_rebuild_locked = false;

	show ();
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

	QString cd_to = RKSettingsModuleGeneral::initialWorkingDirectory ();
	if (!cd_to.isEmpty ()) {
		RKGlobals::rInterface ()->issueCommand ("setwd (" + RObject::rQuote (cd_to) + ")\n", RCommand::App);
		QDir::setCurrent (cd_to);
	}

	if (!open_urls.isEmpty()) {
		// this is also done when there are no urls specified on the command line. But in that case _after_ loading any workspace, so
		// the help window will be on top
		if (RKSettingsModuleGeneral::showHelpOnStartup ()) toplevel_actions->showRKWardHelp ();

		openUrlsFromCommandLineOrDBus (warn_external, open_urls);
	} else {
		StartupDialog::StartupDialogResult result = StartupDialog::getStartupAction (this, fileOpenRecentWorkspace);
		if (!result.open_url.isEmpty ()) {
			openWorkspace (result.open_url);
		} else {
			if (result.result == StartupDialog::ChoseFile) {
				askOpenWorkspace (QUrl());
			} else if (result.result == StartupDialog::EmptyTable) {
				RKWorkplace::mainWorkplace ()->editNewDataFrame (i18n ("my.data"));
			}
		}

		if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
			RKWorkplace::mainWorkplace ()->restoreWorkplace (RKSettingsModuleGeneral::getSavedWorkplace (KSharedConfig::openConfig ().data ()).split ('\n'));
		}
		if (RKSettingsModuleGeneral::showHelpOnStartup ()) toplevel_actions->showRKWardHelp ();
	}
	setNoAskSave (false);

	// up to this point, no "real" save-worthy stuff can be pending in the backend. So mark this point as "clean".
	RCommand *command = new RCommand (QString (), RCommand::EmptyCommand | RCommand::Sync | RCommand::App);
	connect (command->notifier (), &RCommandNotifier::commandFinished, this, &RKWardMainWindow::setWorkspaceUnmodified);
	RKGlobals::rInterface ()->issueCommand (command);

	if (!evaluate_code.isEmpty ()) RKConsole::pipeUserCommand (evaluate_code);
	RKDBusAPI *dbus = new RKDBusAPI (this);
	connect (this, &RKWardMainWindow::aboutToQuitRKWard, dbus, &RKDBusAPI::deleteLater);
	// around on the bus in this case.

	updateCWD ();
	connect (RKGlobals::rInterface (), &RInterface::backendWorkdirChanged, this, &RKWardMainWindow::updateCWD);
	setCaption (QString ());	// our version of setCaption takes care of creating a correct caption, so we do not need to provide it here
}

void RKWardMainWindow::openUrlsFromCommandLineOrDBus (bool warn_external, QStringList _urls) {
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

	if (warn_external && any_dangerous_urls) {
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
	if (all_maps.isEmpty()) {
		KMessageBox::information (0, i18n ("Plugins are needed: you may manage these through \"Settings->Manage R package and plugins\".\n"), i18n ("No active plugin maps"));
		return;
	}

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
			RKSettingsModulePlugins::markPluginMapAsBroken (map);
			completely_broken_maps.append (map);
			completely_broken_maps_details.append (result.detailed_problems);
		} else if (!result.detailed_problems.isEmpty ()) {
			if (RKSettingsModulePlugins::markPluginMapAsQuirky (map)) {
				somewhat_broken_maps.append (map);
				somewhat_broken_maps_details.append (result.detailed_problems);
			}
		} else {
			RKSettingsModulePlugins::markPluginMapAsWorking (map);
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
		KMessageBox::detailedError (0, QString ("<p>%1</p><p>%2</p>").arg (i18n ("The following RKWard pluginmap files could not be loaded, and have been disabled. This could be because they are broken, not compatible with this version of RKWard, or not meant for direct loading (see the 'Details' for more information). They have been disabled.")).arg (maplist), completely_broken_maps_details.join ("\n"), i18n ("Failed to load some plugin maps"));
	}
	if (!somewhat_broken_maps.isEmpty ()) {
		QString maplist = "<ul><li>" + somewhat_broken_maps.join ("</li>\n<li>") + "</li></ul>";
		KMessageBox::detailedError (0, QString ("<p>%1</p><p>%2</p><p>%3</p>").arg (i18n ("Some errors were encountered while loading the following RKWard pluginmap files. This could be because individual plugins are broken or not compatible with this version of RKWard (see the 'Details' for more information). Other plugins were loaded, successfully, however.")).arg (maplist).arg (i18n ("Note: You will not be warned about these pluginmap files again, until you upgrade RKWard, or remove and re-add them in Settings->Configure RKWard->Plugins.")), somewhat_broken_maps_details.join ("\n"), i18n ("Failed to load some plugin maps"));
	}

	slotSetStatusReady ();
}

void RKWardMainWindow::startR () {
	RK_TRACE (APP);
	RK_ASSERT (!RKGlobals::rInterface ());

	// make sure our general purpose files directory exists
	bool ok = QDir ().mkpath (RKSettingsModuleGeneral::filesPath());
	RK_ASSERT (ok);

	RKGlobals::rinter = new RInterface ();
	new RObjectList ();

	RObjectBrowser::mainBrowser ()->unlock ();
}

void RKWardMainWindow::slotConfigure () {
	RK_TRACE (APP);
	RKSettings::configureSettings (RKSettings::NoPage, this);
}

void RKWardMainWindow::slotCancelAllCommands () {
	RK_TRACE (APP);
	RK_ASSERT (RKGlobals::rInterface ());
	RKGlobals::rInterface ()->cancelAll ();
}

void RKWardMainWindow::configureCarbonCopy () {
	RK_TRACE (APP);

	QDialog *dialog = new QDialog ();
	dialog->setWindowTitle (i18n ("Carbon Copy Settings"));
	QVBoxLayout *layout = new QVBoxLayout (dialog);
	RKCarbonCopySettings *settings = new RKCarbonCopySettings (dialog);
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

	RObjectBrowser::object_browser = new RObjectBrowser (0, true);
	RObjectBrowser::mainBrowser ()->setCaption (i18n ("Workspace"));
	RKToolWindowList::registerToolWindow (RObjectBrowser::mainBrowser (), "workspace", RKToolWindowList::Left, Qt::AltModifier + Qt::Key_1);

	RKCommandLog::rkcommand_log = new RKCommandLog (0, true);
	RKToolWindowList::registerToolWindow (RKCommandLog::rkcommand_log, "commandlog", RKToolWindowList::Bottom, Qt::AltModifier + Qt::Key_3);

	startR ();

	RKFileBrowser::main_browser = new RKFileBrowser (0, true);
	RKFileBrowser::main_browser->setCaption (i18n ("Files"));
	RKToolWindowList::registerToolWindow (RKFileBrowser::main_browser, "filebrowser", RKToolWindowList::Left, Qt::AltModifier + Qt::Key_2);

	RControlWindow::control_window = new RControlWindow (0, true);
	RControlWindow::getControl ()->setCaption (i18n ("Pending Jobs"));
	RKToolWindowList::registerToolWindow (RControlWindow::getControl (), "pendingjobs", RKToolWindowList::Nowhere, Qt::AltModifier + Qt::Key_4);

	RKConsole *console = new RKConsole (0, true);
	RKConsole::setMainConsole (console);
	RKToolWindowList::registerToolWindow (console, "console", RKToolWindowList::Bottom, Qt::AltModifier + Qt::Key_5);

	RKHelpSearchWindow *help_search = new RKHelpSearchWindow (0, true);
	RKHelpSearchWindow::main_help_search = help_search;
	RKToolWindowList::registerToolWindow (help_search, "helpsearch", RKToolWindowList::Bottom, Qt::AltModifier + Qt::Key_6);

	RKCallstackViewer::_instance = new RKCallstackViewer (0, true);
	RKCallstackViewer::instance ()->setCaption (i18n ("Debugger Frames"));
	RKToolWindowList::registerToolWindow (RKCallstackViewer::instance (), "debugframes", RKToolWindowList::Right, Qt::AltModifier + Qt::Key_8);

	// HACK: Creating this _after_ the callstackviewer is important, so the debug console will end up the active window when entering a debug context
	RKDebugConsole::_instance = new RKDebugConsole (0, true);
	RKDebugConsole::instance ()->setCaption (i18n ("Debugger Console"));
	RKToolWindowList::registerToolWindow (RKDebugConsole::instance (), "debugconsole", RKToolWindowList::Nowhere, Qt::AltModifier + Qt::Key_7);

	RKDebugMessageWindow::_instance = new RKDebugMessageWindow (0, true);
	RKDebugMessageWindow::instance ()->setCaption (i18n ("RKWard Debug Messages"));
	RKToolWindowList::registerToolWindow (RKDebugMessageWindow::instance (), "rkdebugmessages", RKToolWindowList::Nowhere, 0);

	RKWorkplace::mainWorkplace ()->placeToolWindows ();
}

void RKWardMainWindow::initActions() {  
	RK_TRACE (APP);
	QAction *action;

	// TODO: is there a way to insert actions between standard actions without having to give all standard actions custom ids?
	new_data_frame = actionCollection ()->addAction ("new_data_frame", this, SLOT (slotNewDataFrame()));
	new_data_frame->setText (i18n ("Dataset"));
	new_data_frame->setIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowDataFrameEditor));
	new_data_frame->setStatusTip (i18n ("Creates new empty dataset and opens it for editing"));

	new_command_editor = actionCollection ()->addAction (KStandardAction::New, "new_command_editor", this, SLOT(slotNewCommandEditor()));
	new_command_editor->setText (i18n ("Script File"));
	new_command_editor->setIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowCommandEditor));

	fileOpen = actionCollection ()->addAction (KStandardAction::Open, "file_openy", this, SLOT(slotOpenCommandEditor()));
	fileOpen->setText (i18n ("Open R Script File..."));

	fileOpenRecent = static_cast<KRecentFilesAction*> (actionCollection ()->addAction (KStandardAction::OpenRecent, "file_open_recenty", this, SLOT(slotOpenCommandEditor(QUrl))));
	fileOpenRecent->setText (i18n ("Open Recent R Script File"));

#if 0
	// TODO: Fix import dialog and re-enable it: https://mail.kde.org/pipermail/rkward-devel/2015-June/004156.html
#ifdef Q_OS_WIN
	// TODO: find the cause and fix it! http://sourceforge.net/p/rkward/bugs/54/
#	ifdef __GNUC__
#		warning TODO: import data dialog is disabled on windows due to bug in kdelibs
#	endif
#else
	action = actionCollection ()->addAction ("import_data", this, SLOT (importData()));
	action->setText (i18n ("Import Data"));
	action->setStatusTip (i18n ("Import data from a variety of file formats"));
#endif
#endif

	fileOpenWorkspace = actionCollection ()->addAction (KStandardAction::Open, "file_openx", this, SLOT(slotFileOpenWorkspace()));
	fileOpenWorkspace->setText (i18n ("Open Workspace..."));
	actionCollection ()->setDefaultShortcut (fileOpenWorkspace, Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_O);
	fileOpenWorkspace->setStatusTip (i18n ("Opens an existing document"));

	fileOpenRecentWorkspace = static_cast<KRecentFilesAction*> (actionCollection ()->addAction (KStandardAction::OpenRecent, "file_open_recentx", this, SLOT(askOpenWorkspace(QUrl))));
	fileOpenRecentWorkspace->setText (i18n ("Open Recent Workspace"));
	fileOpenRecentWorkspace->setStatusTip (i18n ("Opens a recently used file"));

	fileSaveWorkspace = actionCollection ()->addAction (KStandardAction::Save, "file_savex", this, SLOT(slotFileSaveWorkspace()));
	fileSaveWorkspace->setText (i18n ("Save Workspace"));
	actionCollection ()->setDefaultShortcut (fileSaveWorkspace, Qt::ControlModifier + Qt::AltModifier + Qt::Key_S);
	fileSaveWorkspace->setStatusTip (i18n ("Saves the actual document"));

	fileSaveWorkspaceAs = actionCollection ()->addAction (KStandardAction::SaveAs, "file_save_asx", this, SLOT(slotFileSaveWorkspaceAs()));
	actionCollection ()->setDefaultShortcut (fileSaveWorkspaceAs, Qt::ControlModifier + Qt::AltModifier + Qt::ShiftModifier + Qt::Key_S);
	fileSaveWorkspaceAs->setText (i18n ("Save Workspace As"));
	fileSaveWorkspaceAs->setStatusTip (i18n ("Saves the actual document as..."));

	fileQuit = actionCollection ()->addAction (KStandardAction::Quit, "file_quitx", this, SLOT(close()));
	fileQuit->setStatusTip (i18n ("Quits the application"));

	interrupt_all_commands = actionCollection ()->addAction ("cancel_all_commands", this, SLOT (slotCancelAllCommands()));
	interrupt_all_commands->setText (i18n ("Interrupt all commands"));
	actionCollection ()->setDefaultShortcut (interrupt_all_commands, Qt::ShiftModifier + Qt::Key_Escape);
	interrupt_all_commands->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionInterrupt));
	interrupt_all_commands->setEnabled (false);		// enabled from within setRStatus()

	action = actionCollection ()->addAction ("carbon_copy", this, SLOT (configureCarbonCopy()));
	action->setText (i18n ("CC commands to output..."));

	// These two currently do the same thing
	action = actionCollection ()->addAction ("load_unload_libs", this, SLOT (slotFileLoadLibs()));
	action->setText (i18n ("Manage R packages and plugins..."));
	action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionConfigurePackages));

	action = actionCollection ()->addAction ("configure_packages", this, SLOT (slotFileLoadLibs()));
	action->setText (i18n ("Manage R packages and plugins..."));
	action->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionConfigurePackages));

	setStandardToolBarMenuEnabled (true);
	createStandardStatusBarAction ();

	close_all_editors = actionCollection ()->addAction ("close_all_editors", this, SLOT (slotCloseAllEditors()));
	close_all_editors->setText (i18n ("Close All Data"));
	close_all_editors->setStatusTip (i18n ("Closes all open data editors"));

	action = actionCollection ()->addAction (KStandardAction::Close, "window_close", this, SLOT (slotCloseWindow()));

	window_close_all = actionCollection ()->addAction ("window_close_all", this, SLOT (slotCloseAllWindows()));
	window_close_all->setText (i18n ("Close All"));

	window_detach = actionCollection ()->addAction ("window_detach", this, SLOT (slotDetachWindow()));
	window_detach->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionDetachWindow));
	window_detach->setText (i18n ("Detach"));

	configure = actionCollection ()->addAction (KStandardAction::Preferences, "options_configure", this, SLOT (slotConfigure()));

	edit_menu_dummy = actionCollection ()->addAction ("edit_menu_dummy", this);
	edit_menu_dummy->setText (i18n ("[No actions available for current view]"));
	edit_menu_dummy->setEnabled (false);
	view_menu_dummy = actionCollection ()->addAction ("view_menu_dummy", this);
	view_menu_dummy->setText (edit_menu_dummy->text ());
	view_menu_dummy->setEnabled (false);

	// collections for the toolbar:
	open_any_action = new KActionMenu (QIcon::fromTheme("document-open-folder"), i18n ("Open..."), this);
	open_any_action->setDelayed (false);
	actionCollection ()->addAction ("open_any", open_any_action);

	open_any_action->addAction (fileOpenWorkspace);
	open_any_action->addAction (fileOpenRecentWorkspace);
	open_any_action->addSeparator ();
	open_any_action->addAction (fileOpen);
	open_any_action->addAction (fileOpenRecent);
	open_any_action->addSeparator ();
	//open_any_action->addAction (proxy_import); -> later

	KActionMenu* new_any_action = new KActionMenu (QIcon::fromTheme("document-new"), i18n ("Create..."), this);
	new_any_action->setDelayed (false);
	actionCollection ()->addAction ("new_any", new_any_action);

	new_any_action->addAction (new_data_frame);
	new_any_action->addAction (new_command_editor);

	save_any_action = new KActionMenu (QIcon::fromTheme("document-save"), i18n ("Save..."), this);
	save_any_action->setDelayed (false);
	actionCollection ()->addAction ("save_any", save_any_action);

	save_any_action->addAction (fileSaveWorkspace);
	save_any_action->addAction (fileSaveWorkspaceAs);
	save_any_action->addSeparator ();
// TODO: A way to add R-script-save actions, dynamically, would be nice
	save_actions_plug_point = save_any_action->addSeparator ();
	//save_any_action->addAction (proxy_export); -> later
}

/*
// debug code: prints out all current actions
void printActionsRecursive (QAction* action, const QString &prefix) {
	if (action->menu ()) {
		foreach (QAction *a, action->menu ()->actions ()) printActionsRecursive (a, prefix + action->text () + "->");
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

void RKWardMainWindow::partChanged (KParts::Part *part) {
	RK_TRACE (APP);

	if (gui_rebuild_locked) return;
	createGUI (part);

	if (!guiFactory ()) {
		RK_ASSERT (false);
		return;
	}

	updateEmptyMenuIndicator (edit_menu_dummy, dynamic_cast<QMenu*>(guiFactory ()->container ("edit", this)));
	updateEmptyMenuIndicator (view_menu_dummy, dynamic_cast<QMenu*>(guiFactory ()->container ("view", this)));

	// plug save file actions into the toolbar collections
	RK_ASSERT (save_any_action);
	for (int i = 0; i < plugged_save_actions.size (); ++i) {
		QAction* a = plugged_save_actions[i].data ();
		if (a) save_any_action->removeAction (a);
	}
	plugged_save_actions.clear ();

	RKMDIWindow *w = RKWorkplace::mainWorkplace ()->activeWindow (RKMDIWindow::Attached);
	if (w && (w->isType (RKMDIWindow::CommandEditorWindow))) {
		QAction *a = static_cast<RKCommandEditorWindow*>(w)->fileSaveAction ();
		if (a) plugged_save_actions.append (a);
		a = static_cast<RKCommandEditorWindow*>(w)->fileSaveAsAction ();
		if (a) plugged_save_actions.append (a);
	}
	for (int i = 0; i < plugged_save_actions.size (); ++i) {
		save_any_action->insertAction (save_actions_plug_point, plugged_save_actions[i]);
	}
/*
	// debug code: prints out all current actions
	foreach (QAction *action, menuBar ()->actions ()) printActionsRecursive (action, QString ());
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

	statusbar_ready = new QLabel (i18n ("Ready."), statusBar ());
	statusBar ()->addWidget (statusbar_ready);
	statusbar_cwd = new KSqueezedTextLabel (statusBar ());
	statusbar_cwd->setAlignment (Qt::AlignRight);
	statusbar_cwd->setToolTip (i18n ("Current working directory"));
	statusBar ()->addWidget (statusbar_cwd, 10);
	updateCWD ();

	QWidget *box = new QWidget (statusBar ());
	QHBoxLayout *boxl = new QHBoxLayout (box);
	boxl->setSpacing (0);
	statusbar_r_status = new QLabel ("&nbsp;<b>R</b>&nbsp;", box);
	statusbar_r_status->setFixedHeight (statusBar ()->fontMetrics ().height () + 2);
	boxl->addWidget (statusbar_r_status);

	QToolButton* dummy = new QToolButton (box);
	dummy->setDefaultAction (interrupt_all_commands);
	dummy->setFixedHeight (statusbar_r_status->height ());
	dummy->setAutoRaise (true);
	boxl->addWidget (dummy);

	statusBar ()->addPermanentWidget (box, 0);
	setRStatus (Starting);
}

void RKWardMainWindow::openWorkspace (const QUrl &url) {
	RK_TRACE (APP);
	if (url.isEmpty ()) return;

	new RKLoadAgent (url, merge_loads);
}

void RKWardMainWindow::saveOptions () {
	RK_TRACE (APP);
	KSharedConfig::Ptr config = KSharedConfig::openConfig ();

	KConfigGroup cg = config->group ("main window options");
	saveMainWindowSettings (cg);

	cg = config->group ("General Options");
// TODO: WORKAROUND. See corresponding line in readOptions ()
	cg.writeEntry("Geometry", size ());

	fileOpenRecentWorkspace->saveEntries (config->group ("Recent Files"));
	fileOpenRecent->saveEntries (config->group ("Recent Command Files"));

	RKSettings::saveSettings (config.data ());

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
		size = QApplication::desktop ()->availableGeometry ().size ();
	}
	resize (size);

	RKSettings::loadSettings (config.data ());

	// initialize the recent file list
	fileOpenRecentWorkspace->loadEntries (config->group ("Recent Files"));
	fileOpenRecent->setMaxItems (RKSettingsModuleCommandEditor::maxNumRecentFiles ());
	fileOpenRecent->loadEntries (config->group ("Recent Command Files"));
}


bool RKWardMainWindow::doQueryQuit () {
	RK_TRACE (APP);

	slotSetStatusBarText (i18n ("Exiting..."));
	saveOptions ();
	if (RKSettingsModuleGeneral::workplaceSaveMode () == RKSettingsModuleGeneral::SaveWorkplaceWithSession) {
		RKSettingsModuleGeneral::setSavedWorkplace (RKWorkplace::mainWorkplace ()->makeWorkplaceDescription ().join ("\n"), KSharedConfig::openConfig ().data ());
	}

//	if (!RObjectList::getGlobalEnv ()->isEmpty ()) {
	int res;
	res = KMessageBox::questionYesNoCancel (this, i18n ("Quitting RKWard: Do you want to save the workspace?"), i18n ("Save Workspace?"), KStandardGuiItem::save (), KStandardGuiItem::discard (), KGuiItem (i18n ("Do Not Quit")));
	if (res == KMessageBox::Yes) {
		new RKSaveAgent (RKWorkplace::mainWorkplace ()->workspaceURL (), false, RKSaveAgent::DoNothing);
	} else if (res == KMessageBox::Cancel) {
		slotSetStatusReady ();
		return false;
	}
//	}

	RKWorkplace::RKWorkplaceObjectList map = RKWorkplace::mainWorkplace ()->getObjectList ();
	for (RKWorkplace::RKWorkplaceObjectList::const_iterator it = map.constBegin (); it != map.constEnd (); ++it) {
		lockGUIRebuild (true);
		if (!(*it)->close (true)) {
			if (!(*it)->isType (RKMDIWindow::X11Window)) {	// X11 windows have a delayed close
				// If a child refuses to close, we return false.
				slotSetStatusReady ();
				lockGUIRebuild (false);
				return false;
			}
		}
		gui_rebuild_locked = false; // like lockGUIRebuild (false), but does not trigger an immediate rebuild, as we are about to leave, anyway.
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

	if (!no_ask_save && ((!RObjectList::getGlobalEnv ()->isEmpty () && workspace_modified) || !RKGlobals::rInterface ()->backendIsIdle ())) {
		int res;
		res = KMessageBox::questionYesNoCancel (this, i18n ("Do you want to save the current workspace?"), i18n ("Save Workspace?"));
		if (res == KMessageBox::Yes) {
			new RKSaveAgent (RKWorkplace::mainWorkplace ()->workspaceURL (), false, RKSaveAgent::Load, url);
		} else if (res != KMessageBox::No) { // Cancel
			return;
		}
	}

	slotCloseAllEditors ();

	slotSetStatusBarText(i18n("Opening workspace..."));
	QUrl lurl = url;
	if (lurl.isEmpty ()) {
		lurl = QFileDialog::getOpenFileUrl (this, i18n("Select workspace to open..."), RKSettingsModuleGeneral::lastUsedUrlFor ("workspaces"), i18n ("R Workspace Files [%1](%1);;All files [*](*)", RKSettingsModuleGeneral::workspaceFilenameFilter ()));
	}
	if (!lurl.isEmpty ()) {
		RKSettingsModuleGeneral::updateLastUsedUrl ("workspaces", lurl.adjusted (QUrl::RemoveFilename));
		openWorkspace (lurl);
	}
	slotSetStatusReady ();
}

void RKWardMainWindow::slotFileOpenWorkspace () {
	RK_TRACE (APP);
	askOpenWorkspace (QUrl ());
}

void RKWardMainWindow::slotFileLoadLibs () {
	RK_TRACE (APP);
	RKLoadLibsDialog *dial = new RKLoadLibsDialog (this, 0);
	dial->show ();
}

void RKWardMainWindow::slotFileSaveWorkspace () {
	RK_TRACE (APP);
	new RKSaveAgent (RKWorkplace::mainWorkplace ()->workspaceURL ());
}

void RKWardMainWindow::slotFileSaveWorkspaceAs () {
	RK_TRACE (APP);
	new RKSaveAgent (RKWorkplace::mainWorkplace ()->workspaceURL (), true);
}

void RKWardMainWindow::addWorkspaceUrl (const QUrl &url) {
	RK_TRACE (APP);

	if (!url.isEmpty ()) fileOpenRecentWorkspace->addUrl (url);
	setCaption (QString ());	// trigger update of caption
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

void RKWardMainWindow::setRStatus (RStatus status) {
	RK_TRACE (APP);

	QColor status_color;
	if (status == Busy) {
		status_color = QColor (255, 0, 0);
		statusbar_r_status->setToolTip (i18n ("The <b>R</b> engine is busy."));
		interrupt_all_commands->setEnabled (true);
	} else if (status == Idle) {
		status_color = QColor (0, 255, 0);
		statusbar_r_status->setToolTip (i18n ("The <b>R</b> engine is idle."));
		interrupt_all_commands->setEnabled (false);
	} else {
		status_color = QColor (255, 255, 0);
		statusbar_r_status->setToolTip (i18n ("The <b>R</b> engine is being initialized."));
	}
	QPalette palette = statusbar_r_status->palette ();
	palette.setBrush (statusbar_r_status->backgroundRole(), QBrush (status_color));
	statusbar_r_status->setAutoFillBackground (true);
	statusbar_r_status->setPalette (palette);
}

void RKWardMainWindow::importData () {
	RK_TRACE (APP);

	new RKImportDialog ("import", this);
}

void RKWardMainWindow::slotNewCommandEditor () {
	RK_TRACE (APP);

	slotOpenCommandEditor (QUrl ());
}

void RKWardMainWindow::addScriptUrl (const QUrl &url) {
	RK_TRACE (APP);

	if (!url.isEmpty ()) fileOpenRecent->addUrl (url);
}

void RKWardMainWindow::slotOpenCommandEditor (const QUrl &url, const QString &encoding) {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openScriptEditor (url, encoding, url.isEmpty() || RKSettingsModuleCommandEditor::matchesScriptFileFilter (url.fileName()));
}

void RKWardMainWindow::slotOpenCommandEditor () {
	RK_TRACE (APP);
	KEncodingFileDialog::Result res;

	res = KEncodingFileDialog::getOpenUrlsAndEncoding (QString (), RKSettingsModuleGeneral::lastUsedUrlFor ("rscripts"), QString ("%1|R Script Files (%1)\n*|All Files (*)").arg (RKSettingsModuleCommandEditor::scriptFileFilter ()), this, i18n ("Open script file(s)"));
	for (int i = 0; i < res.URLs.size (); ++i) {
		if (i == 0) RKSettingsModuleGeneral::updateLastUsedUrl ("rscripts", res.URLs[i].adjusted (QUrl::RemoveFilename));
		slotOpenCommandEditor (res.URLs[i], res.encoding);
	}
};

void RKWardMainWindow::setCaption (const QString &) {
	RK_TRACE (APP);

	QString wcaption = RKWorkplace::mainWorkplace ()->workspaceURL ().fileName ();
	if (wcaption.isEmpty ()) wcaption = RKWorkplace::mainWorkplace ()->workspaceURL ().toDisplayString ();
	if (wcaption.isEmpty ()) wcaption = i18n ("[Unnamed Workspace]");
	RKMDIWindow *window = RKWorkplace::mainWorkplace ()->view ()->activePage ();
	if (window) wcaption.append (" - " + window->fullCaption ());
	KParts::MainWindow::setCaption (wcaption);
}

