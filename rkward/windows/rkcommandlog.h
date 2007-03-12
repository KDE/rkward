/***************************************************************************
                          rkcommandlog  -  description
                             -------------------
    begin                : Sun Nov 3 2002
    copyright            : (C) 2002, 2004, 2005, 2006, 2007 by Thomas Friedrichsmeier
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

#ifndef RKCOMMANDLOG_H
#define RKCOMMANDLOG_H

#include <qstring.h>
#include <qtextedit.h>
#include <kmdichildview.h>

#include "rkmdiwindow.h"
#include "../rbackend/rcommandreceiver.h"

class RCommand;
class ROutput;
class QPushButton;
class QTextEdit;
class RKCommandEditor;
class QBoxLayout;
class RKCommandLogView;
class RKCommandLogPart;

/**
	\brief This widget shows all executed commands and their result
@author Thomas Friedrichsmeier
*/

class RKCommandLog : public RKMDIWindow, public RCommandReceiver {
	Q_OBJECT
public: 
/** Adds input to the log_view-window (i.e. commands issued) */
	void addInput (RCommand *command);
/** Adds output to the log_view-window (i.e. replies received) */
	void newOutput (RCommand *command, ROutput *output_fragment);

	static RKCommandLog *getLog () { return rkcommand_log; };

	RKCommandLogView *getView () { return log_view; };
protected:
/** Command has finished. If the command has failed, it may be necessary to print some more information */
	void rCommandDone (RCommand *command);
	RKCommandLog (QWidget *parent, bool tool_window, char *name=0);
	~RKCommandLog ();
public slots:
/** configures the log_view-window */
	void configureLog ();
/** clears the log_view-window */
	void clearLog ();
	void runSelection ();
private:
	void addInputNoCheck (RCommand *command);
	void addOutputNoCheck (RCommand *command, ROutput *output);
	void checkRaiseWindow (RCommand *command);
/** internal helper function, called whenever a line/lines have been added. Check whether log is longer than maximum setting. Scroll to the bottom */
	void linesAdded ();
/** Id of last command the input (i.e. the command itself) was shown for. Used to keep track of whether a command's input should be shown or not */
	int command_input_shown;
/** On a given command, the log_view should not be raised more than once */
	int last_raised_command;

	RKCommandLogView *log_view;
friend class RKWardMainWindow;
	static RKCommandLog *rkcommand_log;
};

/** Simply subclass of QTextEdit to override context menu handling */
class RKCommandLogView : public QTextEdit {
	Q_OBJECT
public:
	RKCommandLogView (RKCommandLog *parent);
	~RKCommandLogView ();
public slots:
	void selectAll ();
signals:
	void popupMenuRequest (const QPoint &pos);
protected:
	bool eventFilter (QObject *o, QEvent *e);
};

#include <kparts/part.h>

class KAction;

/** Provides a part interface for the RKCommandLog */
class RKCommandLogPart : public KParts::Part {
	Q_OBJECT
public:
	explicit RKCommandLogPart (RKCommandLog *for_log);
	~RKCommandLogPart ();
public slots:
	void doPopupMenu (const QPoint &pos);
private:
	RKCommandLog *log;

	KAction *run_selection;
	KAction *copy;
};

#endif
