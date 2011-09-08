/***************************************************************************
                          rkhelpsearchwindow  -  description
                             -------------------
    begin                : Fri Feb 25 2005
    copyright            : (C) 2005, 2006, 2007, 2009, 2010, 2011 by Thomas Friedrichsmeier
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
#include <QAbstractTableModel>

#include "../rbackend/rcommandreceiver.h"
#include "rkmdiwindow.h"

class QFocusEvent;
class QComboBox;
class QCheckBox;
class QPushButton;
class RKHelpSearchResultsModel;
class QTreeView;
class QSortFilterProxyModel;
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
	void getFunctionHelp (const QString &function_name, const QString &package=QString(), const QString &type=QString ());
	static RKHelpSearchWindow *mainHelpSearch () { return main_help_search; };
public slots:
	void slotFindButtonClicked();
	void resultDoubleClicked (const QModelIndex& index);
	void updateInstalledPackages ();
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
	QTreeView* results_view;
	RKHelpSearchResultsModel* results;
	QSortFilterProxyModel* proxy_model;

friend class RKWardMainWindow;
	static RKHelpSearchWindow *main_help_search;
};

/** An item model meant for use by RKHelpSearchWindow. Since it is fairly specialized, it is unlikely to be of any use in any other context. NOTE: This class is pretty useless, really, we should just switch to a QTree/TableWidget with predefined model, whenever we need to make the next big change to the RKHelpSearchWindow.
@author Thomas Friedrichsmeier */
class RKHelpSearchResultsModel : public QAbstractTableModel {
public:
	RKHelpSearchResultsModel (QObject *parent);
	~RKHelpSearchResultsModel ();

/** Set the results. The model will assume ownership of the results */
	void setResults (const QStringList &new_results);

	int rowCount (const QModelIndex& parent=QModelIndex()) const;
	int columnCount (const QModelIndex& parent=QModelIndex()) const;
	QVariant data (const QModelIndex& index, int role=Qt::DisplayRole) const;
	QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QString resultsType (int row);
private:
	QStringList topics;
	QStringList titles;
	QStringList packages;
	QStringList types;
	int result_count;
};

#endif
