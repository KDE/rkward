/***************************************************************************
                          rkhelpsearchwindow  -  description
                             -------------------
    begin                : Fri Feb 25 2005
    copyright            : (C) 2005, 2006, 2007, 2010 by Thomas Friedrichsmeier
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

#ifndef RKHELPSEARCHWINDOW_H
#define RKHELPSEARCHWINDOW_H

#include <qwidget.h>

#include "../rbackend/rcommandreceiver.h"
#include "rkmdiwindow.h"

class QComboBox;
class QCheckBox;
class QPushButton;
class QListView;
class QListViewItem;

class RCommandChain;

/** Provides a UI interface for help-search.

@author Pierre Ecochard */
class RKHelpSearchWindow : public RKMDIWindow, public RCommandReceiver {
	Q_OBJECT
public:
	RKHelpSearchWindow (QWidget *parent, bool tool_window, const char *name=0);
	~RKHelpSearchWindow ();
	void rCommandDone (RCommand *command);
/** small convenience function to get context help for RKCommandEditorWindow and RKConsole.
@param context_line The current line
@param cursor_pos cursor position in the current line
Will figure out the word under the cursor, and provide help on that (if there is such a word, and such help exists) */
	void getContextHelp (const QString &context_line, int cursor_pos);
	void getFunctionHelp (const QString &function_name, const QString &package=QString::null);
	static RKHelpSearchWindow *mainHelpSearch () { return main_help_search; };
public slots:
	void slotFindButtonClicked();
	void slotResultsListDblClicked (QListViewItem *item, const QPoint &, int);
protected:
/** reimplemnented from QWidget to make the input focus default to the input field */
	void focusInEvent (QFocusEvent *e);
private:
	QComboBox* field;
	QComboBox* fieldsList;
	QComboBox* packagesList;
	QCheckBox* caseSensitiveCheckBox;
	QCheckBox* fuzzyCheckBox;
	QPushButton* findButton;
	QListView* resultsList;
friend class RKWardMainWindow;
	static RKHelpSearchWindow *main_help_search;
};

#endif
