/***************************************************************************
                          rktoplevelwindowgui  -  description
                             -------------------
    begin                : Tue Apr 24 2007
    copyright            : (C) 2007-2022 by Thomas Friedrichsmeier
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

#ifndef RKTOPLEVELWINDOWGUI_H
#define RKTOPLEVELWINDOWGUI_H

#include <kxmlguiclient.h>

#include <qobject.h>

class KXmlGuiWindow;
class RKMDIWindow;
class QAction;
class KHelpMenu;
class QMenu;

/** represents the common portions of the GUI for top level windows: The help menu, and the windows menu */
class RKTopLevelWindowGUI : public QObject, public KXMLGUIClient {
	Q_OBJECT
public:
	explicit RKTopLevelWindowGUI (KXmlGuiWindow *for_window);
	~RKTopLevelWindowGUI ();
	void initToolWindowActions ();
public slots:
	// windows menu
	/** Raise the help search window */
	void showHelpSearch ();
	/** Activate the current (non tools) window in the workspace */
	void activateDocumentView ();
	/** Show an output window */
	void slotOutputShow(QAction *action);
	void populateOutputWindowsMenu();

	// help menu
	/** Show the starting page of RKWard help */
	void showRKWardHelp ();
	/** enter "what's this" mode */
	void startWhatsThis ();
	/** Invokes R help (help.start ()) */
	void invokeRHelp ();
	/** show instructions on reporting bugs in rkward */
	void reportRKWardBug ();
	/** not quite sure, why I have to reimplement this from KMainWindow */
	void showAboutApplication ();
	/** Language switching dialog */
	void showSwitchApplicationLanguage();

	// settings menu
	/** configure key bindings. Reimplemented to show notice before the actual dialog. */
	void configureShortcuts ();
	/** configure key bindings. Reimplemented to show notice before the actual dialog. */
	void configureToolbars ();
private slots:
	void toggleToolView ();
	void previousWindow ();
	void nextWindow ();
private:
	KXmlGuiWindow *for_window;
	QAction *prev_action;
	QAction *next_action;
	void toggleToolView (RKMDIWindow *tool_window);
	KHelpMenu *help_menu_dummy;
	QMenu* output_windows_menu;
};

#endif
