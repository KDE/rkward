/*
rkcommandlog - This file is part of the RKWard project. Created: Sun Nov 3 2002
SPDX-FileCopyrightText: 2002-2023 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKCOMMANDLOG_H
#define RKCOMMANDLOG_H

#include <qstring.h>
#include <qtextedit.h>

#include "rkmdiwindow.h"
#include "../settings/rksettings.h"
#include "../rbackend/rcommand.h"

class RKCommandLogView;
class RKCommandLogPart;

/**
	\brief This widget shows all executed commands and their result
@author Thomas Friedrichsmeier
*/

class RKCommandLog : public RKMDIWindow {
	Q_OBJECT
public: 
/** Adds input to the log_view-window (i.e. commands issued) */
	void addInput (RCommand *command);
/** Adds output to the log_view-window (i.e. replies received) */
	void newOutput (RCommand *command, ROutput *output_fragment);
/** Adds output not originating from R. Note: Currently used from kate plugins, only, see katepluginintegration.cpp */
	void addOtherMessage(const QString &message, const QIcon &icon, ROutput::ROutputType severity);

	static RKCommandLog *getLog () { return rkcommand_log; };

	RKCommandLogView *getView () { return log_view; };
protected:
friend class RCommand;
/** Command has finished. If the command has failed, it may be necessary to print some more information */
	void rCommandDone (RCommand *command);
	RKCommandLog(QWidget *parent, bool tool_window, const char *name=nullptr);
	~RKCommandLog();
public Q_SLOTS:
/** configures the log_view-window */
	void configureLog ();
/** clears the log_view-window */
	void clearLog ();
	void runSelection ();
	void settingsChanged (RKSettings::SettingsPage page);
private:
	void addInputNoCheck (RCommand *command);
	void addOutputNoCheck (RCommand *command, ROutput *output);
	void checkRaiseWindow (RCommand *command);
/** internal helper function, called whenever a line/lines have been added. Check whether log is longer than maximum setting. Scroll to the bottom */
	void linesAdded ();
/** Used to keep track, which commands "input" has already been shown */
	QList<RCommand*> command_input_shown;
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
	explicit RKCommandLogView (RKCommandLog *parent);
	~RKCommandLogView ();
public Q_SLOTS:
	void selectAll ();
Q_SIGNALS:
	void popupMenuRequest (const QPoint &pos);
protected:
	void contextMenuEvent (QContextMenuEvent *event) override;
};

#include <kparts/part.h>

class QAction;

/** Provides a part interface for the RKCommandLog */
class RKCommandLogPart : public KParts::Part {
	Q_OBJECT
public:
	explicit RKCommandLogPart (RKCommandLog *for_log);
	~RKCommandLogPart ();

	void initActions ();
public Q_SLOTS:
	void doPopupMenu (const QPoint &pos);
private:
	RKCommandLog *log;

	QAction *run_selection;
	QAction *copy;
};

#endif
