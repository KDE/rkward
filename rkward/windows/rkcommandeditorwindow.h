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

#include <kurl.h>

class RKCommandEditor;
class KAction;
class KToggleAction;

/**
This class provides an editor window for R-commands. Basically this class is responsible for adding a GUI to RKCommandEditor.

@author Thomas Friedrichsmeier
*/
class RKCommandEditorWindow : public QWidget {
Q_OBJECT
public:
    RKCommandEditorWindow (QWidget *parent = 0);

    ~RKCommandEditorWindow();
public slots:
	void newWindow ();
	void save ();
	void saveAs ();
	void print ();
	void close ();
	void load ();
	void run ();
	void runSelection ();
	void runFromCursor ();
	void runToCursor ();
	void configure ();
	void wordWrap ();
	void lineNumbers ();
private:
	RKCommandEditor *editor;
	bool checkSave ();
	void trySave (const KURL &url);
	QString caption;
	
	KAction *file_new;
	KAction *file_open;
	KAction *file_save;
	KAction *file_save_as;
	KAction *file_print;
	KAction *file_close;
	
	KAction *run_all;
	
	KToggleAction *word_wrap;
	KToggleAction *line_numbers;
protected:
	void closeEvent (QCloseEvent *e);
};

#endif
