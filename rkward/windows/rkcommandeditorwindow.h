/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef RKCOMMANDEDITORWINDOW_H
#define RKCOMMANDEDITORWINDOW_H

#include <qwidget.h>
#include <qstring.h>


#include <kate/view.h>
#include <kate/document.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kparts/part.h>


#include <kurl.h>

class RKCommandEditor;
class KAction;
class KToggleAction;

/**
This class provides an editor window for R-commands. Basically this class is responsible for adding a GUI to RKCommandEditor.

@author Thomas Friedrichsmeier
*/
class RKCommandEditorWindow : public KParts::MainWindow {
Q_OBJECT
public:
    RKCommandEditorWindow (QWidget *parent = 0);

    ~RKCommandEditorWindow();
public slots:
	void newWindow ();

	void closeWindow ();

	void run ();
	void runSelection ();
	void runFromCursor ();
	void runToCursor ();

	void newFile ();
	void openFile ();
private:
	RKCommandEditor *editor;
	KTextEditor::View *m_view;
	KTextEditor::Document *m_doc;
	void setRHighlighting ();

	QString caption;
	
protected:
	void closeEvent (QCloseEvent *e);
};

#endif
