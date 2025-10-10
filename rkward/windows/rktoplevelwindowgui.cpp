/*
rktoplevelwindowgui - This file is part of RKWard (https://rkward.kde.org). Created: Tue Apr 24 2007
SPDX-FileCopyrightText: 2007-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rktoplevelwindowgui.h"

#include <KAboutData>
#include <KActionMenu>
#include <KColorSchemeManager>
#include <KColorSchemeMenu>
#include <KHelpMenu>
#include <KLocalizedString>
#include <KMessageBox>
#include <kaboutapplicationdialog.h>
#include <kactioncollection.h>
#include <kcolorscheme_version.h>
#include <kshortcutsdialog.h>
#include <kxmlgui_version.h>
#include <kxmlguifactory.h>

#include <QDomDocument>
#include <QDomElement>
#include <QMenu>
#include <QWhatsThis>

#include "../dialogs/rkerrordialog.h"
#include "../misc/rkoutputdirectory.h"
#include "../misc/rkstandardicons.h"
#include "../plugin/rkcomponentmap.h"
#include "../rbackend/rkrinterface.h"
#include "../rkconsole.h"
#include "../rkward.h"
#include "../windows/rcontrolwindow.h"
#include "../windows/rkcommandlog.h"
#include "../windows/rkfilebrowser.h"
#include "../windows/rkhelpsearchwindow.h"
#include "../windows/rkhtmlwindow.h"
#include "../windows/rkmdiwindow.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkworkplaceview.h"
#include "../windows/robjectbrowser.h"

#include "../debug.h"

RKTopLevelWindowGUI::RKTopLevelWindowGUI(KXmlGuiWindow *for_window) : QObject(for_window), KXMLGUIClient(), help_menu_dummy(nullptr) {
	RK_TRACE(APP);

	RKTopLevelWindowGUI::for_window = for_window;

	setXMLFile(QStringLiteral("rktoplevelwindowgui.rc"));

	// help menu
	QAction *help_invoke_r_help = actionCollection()->addAction(QStringLiteral("invoke_r_help"), this, &RKTopLevelWindowGUI::invokeRHelp);
	help_invoke_r_help->setText(i18n("Help on R"));
	QAction *show_help_search = actionCollection()->addAction(QStringLiteral("show_help_search"), this, &RKTopLevelWindowGUI::showHelpSearch);
	show_help_search->setText(i18n("Search R Help"));
	QAction *show_rkward_help = actionCollection()->addAction(KStandardAction::HelpContents, QStringLiteral("rkward_help"), this, &RKTopLevelWindowGUI::showRKWardHelp);
	show_rkward_help->setText(i18n("RKWard Dashboard and Help"));

	actionCollection()->addAction(KStandardAction::AboutApp, QStringLiteral("about_app"), this, &RKTopLevelWindowGUI::showAboutApplication);
	actionCollection()->addAction(KStandardAction::WhatsThis, QStringLiteral("whats_this"), this, &RKTopLevelWindowGUI::startWhatsThis);
	actionCollection()->addAction(KStandardAction::ReportBug, QStringLiteral("report_bug"), this, &RKTopLevelWindowGUI::reportRKWardBug);
	actionCollection()->addAction(KStandardAction::SwitchApplicationLanguage, QStringLiteral("switch_application_language"), this, &RKTopLevelWindowGUI::showSwitchApplicationLanguage);

	help_invoke_r_help->setWhatsThis(i18n("Shows the R help index"));
	show_help_search->setWhatsThis(i18n("Shows/raises the R Help Search window"));
	show_rkward_help->setWhatsThis(i18n("Show the RKWard dashboard with links to important settings and documentation"));

	// window menu
	// NOTE: enabling / disabling the prev/next actions is not a good idea. It will cause the script windows to "accept" their shortcuts, when disabled
	prev_action = actionCollection()->addAction(QStringLiteral("prev_window"), this, &RKTopLevelWindowGUI::previousWindow);
	prev_action->setText(i18n("Previous Window"));
	prev_action->setIcon(QIcon(u":/rkward/icons/window_back.png"_s));
	actionCollection()->setDefaultShortcut(prev_action, Qt::ControlModifier | Qt::Key_Tab);
	next_action = actionCollection()->addAction(QStringLiteral("next_window"), this, &RKTopLevelWindowGUI::nextWindow);
	next_action->setText(i18n("Next Window"));
	next_action->setIcon(QIcon(u":rkward/icons/window_forward.png"_s));
	actionCollection()->setDefaultShortcut(next_action, Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Tab);

	QAction *action = actionCollection()->addAction(QStringLiteral("window_activate_docview"), this, &RKTopLevelWindowGUI::activateDocumentView);
	action->setText(i18n("Activate Document view"));
	actionCollection()->setDefaultShortcut(action, Qt::AltModifier | Qt::Key_0);

	action = actionCollection()->addAction(QStringLiteral("output_show"));
	action->setText(i18n("Show Output"));
	action->setIcon(RKStandardIcons::getIcon(RKStandardIcons::WindowOutput));
	action->setMenu(output_windows_menu = new QMenu());
	connect(output_windows_menu, &QMenu::aboutToShow, this, &RKTopLevelWindowGUI::populateOutputWindowsMenu);
	connect(output_windows_menu, &QMenu::triggered, this, &RKTopLevelWindowGUI::slotOutputShow);

	// settings
	KStandardAction::keyBindings(this, &RKTopLevelWindowGUI::configureShortcuts, actionCollection());
	KStandardAction::configureToolbars(this, &RKTopLevelWindowGUI::configureToolbars, actionCollection());
	// Color scheme action. NOTE: selection is non-permanent for KF5 <= 5.87.0, auto-saved afterwards. Apparently, auto-save cannot be implemented for earlier versions within a few lines of code
#if KCOLORSCHEME_VERSION >= QT_VERSION_CHECK(6, 6, 0) // NOTE: When retiring this version check, obsolete the "manager"-member along with it
	KColorSchemeManager *manager = KColorSchemeManager::instance();
#else
	KColorSchemeManager *manager = new KColorSchemeManager(this);
#endif
	actionCollection()->addAction(QStringLiteral("colorscheme_menu"), KColorSchemeMenu::createMenu(manager, this));
	// our "status bar" is inlined, and always visible. Action below would only hide and show a useless proxy
	// KF6 TODO: Still needed at all?
	QAction *a = for_window->action(QStringLiteral("options_show_statusbar"));
	if (a) a->setVisible(false);
}

RKTopLevelWindowGUI::~RKTopLevelWindowGUI() {
	RK_TRACE(APP);
	delete help_menu_dummy;
	delete output_windows_menu;
}

void RKTopLevelWindowGUI::initToolWindowActions() {
	RK_TRACE(APP);

	// Tool window actions
	QString action_tag(QStringLiteral("Action"));
	QString name_attr(QStringLiteral("name"));
	QDomDocument doc = xmlguiBuildDocument();
	if (doc.documentElement().isNull()) doc = domDocument();
	QDomElement menu = doc.elementsByTagName(QStringLiteral("Menu")).at(1).toElement(); // NOTE: this is known to be the "Windows"-Menu
	QDomElement ref = menu.firstChildElement(action_tag);
	while (!ref.isNull() && ref.attribute(name_attr) != QLatin1String("window_show_PLACEHOLDER")) {
		ref = ref.nextSiblingElement(action_tag);
	}
	QAction *action;
	const auto windows = RKToolWindowList::registeredToolWindows();
	for (const RKToolWindowList::ToolWindowRepresentation &rep : windows) {
		QString id = QLatin1String("window_show_") + rep.id;
		QString wid = rep.id;
		action = actionCollection()->addAction(id, this, [this, wid]() { toggleToolView(wid); });
		action->setText(i18n("Show/Hide %1", rep.window->shortCaption()));
		action->setIcon(rep.window->windowIcon());
		actionCollection()->setDefaultShortcut(action, rep.default_shortcut);
		QDomElement e = doc.createElement(action_tag);
		e.setAttribute(name_attr, id);
		menu.insertBefore(e, ref);
	}
	setXMLGUIBuildDocument(doc);
}

void RKTopLevelWindowGUI::configureShortcuts() {
	RK_TRACE(APP);

	KMessageBox::information(for_window, i18n("For technical reasons, the following dialog allows you to configure the keyboard shortcuts only for those parts of RKWard that are currently active.\n\nTherefore, if you want to configure keyboard shortcuts e.g. for use inside the script editor, you need to open a script editor window, and activate it."), i18n("Note"), QStringLiteral("configure_shortcuts_kparts"));

	KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, qobject_cast<QWidget *>(parent()));
	const auto clients = factory()->clients();
	for (KXMLGUIClient *client : clients) {
		if (client && !client->xmlFile().isEmpty()) dlg.addCollection(client->actionCollection());
	}
	dlg.addCollection(RKComponentMap::getMap()->actionCollection(), i18n("RKWard Plugins"));
	dlg.configure(true);
}

void RKTopLevelWindowGUI::configureToolbars() {
	RK_TRACE(APP);

	KMessageBox::information(for_window, i18n("For technical reasons, the following dialog allows you to configure the toolbar buttons only for those parts of RKWard that are currently active.\n\nTherefore, if you want to configure tool buttons e.g. for use inside the script editor, you need to open a script editor window, and activate it."), i18n("Note"), QStringLiteral("configure_toolbars_kparts"));

	for_window->configureToolbars();
}

void RKTopLevelWindowGUI::invokeRHelp() {
	RK_TRACE(APP);

	RInterface::issueCommand(QStringLiteral("help.start ()"), RCommand::App);
	RKWardMainWindow::getMain()->topLevelWidget()->raise();
}

void RKTopLevelWindowGUI::startWhatsThis() {
	RK_TRACE(APP);

	QWhatsThis::enterWhatsThisMode();
}

void RKTopLevelWindowGUI::reportRKWardBug() {
	RKErrorDialog::reportBug(for_window);
}

void RKTopLevelWindowGUI::showAboutApplication() {
	RK_TRACE(APP);

	KAboutApplicationDialog about(KAboutData::applicationData());
	about.exec();
}

void RKTopLevelWindowGUI::showSwitchApplicationLanguage() {
	RK_TRACE(APP);

	// Uggh. No direct or static access to KSwitchLanguageDialog...
	if (!help_menu_dummy) {
#if KXMLGUI_VERSION >= QT_VERSION_CHECK(6, 9, 0)
		help_menu_dummy = new KHelpMenu(for_window);
		help_menu_dummy->setShowWhatsThis(false);
#else
		help_menu_dummy = new KHelpMenu(for_window, QString(), false);
#endif
	}
	help_menu_dummy->switchApplicationLanguage();
}

void RKTopLevelWindowGUI::toggleToolView(RKMDIWindow *tool_window) {
	RK_TRACE(APP);
	RK_ASSERT(tool_window);

	if (tool_window->isActive()) {
		tool_window->close(RKMDIWindow::NoAskSaveModified);
		activateDocumentView();
	} else {
		tool_window->activate(true);
	}
}

void RKTopLevelWindowGUI::toggleToolView(const QString &id) {
	RK_TRACE(APP);

	RKMDIWindow *win = RKToolWindowList::findToolWindowById(id);
	RK_ASSERT(win);
	toggleToolView(win);
}

void RKTopLevelWindowGUI::showHelpSearch() {
	RK_TRACE(APP);

	RKHelpSearchWindow::mainHelpSearch()->activate();
}

void RKTopLevelWindowGUI::showRKWardHelp() {
	RK_TRACE(APP);

	RKWorkplace::mainWorkplace()->openHelpWindow(QUrl(QStringLiteral("rkward://page/rkward_welcome")), true);
}

void RKTopLevelWindowGUI::activateDocumentView() {
	RK_TRACE(APP);

	RKMDIWindow *window = RKWorkplace::mainWorkplace()->view()->activePage();
	if (window) window->activate();
}

void RKTopLevelWindowGUI::slotOutputShow(QAction *action) {
	RK_TRACE(APP);

	QString id = action->data().toString();
	RKOutputDirectory *dir = RKOutputDirectory::findOutputById(id);
	if (!dir) return; // Could happen in corner case, if dir got deleted, while menu is shown
	dir->view(true);
}

void RKTopLevelWindowGUI::populateOutputWindowsMenu() {
	RK_TRACE(APP);

	output_windows_menu->clear();
	auto outputs = RKOutputDirectory::allOutputs();
	for (int i = 0; i < outputs.size(); ++i) {
		auto dir = outputs[i];
		QString title = dir->caption();
		if (dir->isActive()) {
			title.append(u' ');
			title.append(i18n("[Active]"));
		}
		QAction *action = output_windows_menu->addAction(title);
		action->setData(dir->getId());
	}
}

void RKTopLevelWindowGUI::nextWindow() {
	RK_TRACE(APP);

	// well, this is sort of cumbersome, but the switcher widget gets keyboard focus, and so we need to register the window switching actions with it.
	RKWorkplace::getHistory()->next(prev_action, next_action);
}

void RKTopLevelWindowGUI::previousWindow() {
	RK_TRACE(APP);

	RKWorkplace::getHistory()->prev(prev_action, next_action);
}
