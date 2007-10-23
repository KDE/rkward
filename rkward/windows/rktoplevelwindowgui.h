/***************************************************************************
                          rktoplevelwindowgui  -  description
                             -------------------
    begin                : Tue Apr 24 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#ifndef RKTOPLEVELWINDOWGUI_H
#define RKTOPLEVELWINDOWGUI_H

#include <kxmlguiclient.h>

#include <qobject.h>

class QWdiget;
class RKMDIWindow;

/** represents the common portions of the GUI for top level windows: The help menu, and the windows menu */
class RKTopLevelWindowGUI : public QObject, public KXMLGUIClient {
	Q_OBJECT
public:
	RKTopLevelWindowGUI (QWidget *for_window);
	~RKTopLevelWindowGUI ();
public slots:
	// windows menu
	/** Raise the help search window */
	void showHelpSearch ();
	/** Toggle the help search window */
	void toggleHelpSearch ();
	/** Toggle the console window */
	void toggleConsole ();
	/** Toggle the command log window */
	void toggleCommandLog ();
	/** Toggle the pending jobs window */
	void togglePendingJobs ();
	/** Toggle the workspace browser window */
	void toggleWorkspace ();
	/** Toggle the filesystem browser window */
	void toggleFilebrowser ();
	/** Activate the current (non tools) window in the workspace */
	void activateDocumentView ();
	/** ensure output window is shown. */
	void slotOutputShow ();

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
private:
	QWidget *for_window;
	void toggleToolView (RKMDIWindow *tool_window);
};

#endif
