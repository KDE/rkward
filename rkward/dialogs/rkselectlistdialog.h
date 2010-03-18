/***************************************************************************
                          rkselectlistdialog  -  description
                             -------------------
    begin                : Thu Mar 18 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#ifndef RKSELECTLISTDIALOG_H
#define RKSELECTLISTDIALOG_H

#include <kdialog.h>

class QListWidget;
class QWidget;

/** This class represent a dialog asking for a choice among several optiosn. It is used, when the backend calls select.list() / menu().

@author Thomas Friedrichsmeier
*/
class RKSelectListDialog : public KDialog {
	Q_OBJECT
public:
	/** Construct and run modal RKSelectListDialog.
	@param parent QWidget to center the modal dialog on (0 for application)
	@param caption Dialog title
	@param choices The list of available choices
	@param preselected The preselected choices (QStringList ()) for no preselects
	@param multiple Whether multiple items may be selected
	@returns The selected option(s). An empty list, if cancel was pressed. */
	static QStringList doSelect (QWidget *parent, const QString &caption, const QStringList& choices, const QStringList& preselected, bool multiple);
protected:
	/** ctor. Use the doSelect() instead. */
	RKSelectListDialog (QWidget *parent, const QString &caption, const QStringList& choices, const QStringList& preselected, bool multiple);
	/** destructor */
	~RKSelectListDialog ();
private slots:
	void updateState ();
private:
	QListWidget *input;
};

#endif
