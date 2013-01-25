/***************************************************************************
                          multistringselector  -  description
                             -------------------
    begin                : Fri Sep 10 2005
    copyright            : (C) 2005, 2013 by Thomas Friedrichsmeier
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

#ifndef MULTISTRINGSELECTOR_H
#define MULTISTRINGSELECTOR_H

#include <qwidget.h>
#include <qstringlist.h>
#include <qstring.h>

class RKMultiStringSelectorV2;
class QAbstractItemModel;
class QPushButton;
class QStringListModel;
class QTreeView;

/** A more modern variant of the MultiStringSelector. This operates on a (flat!) QAbstractItemModel.
 * The model (or a buddy of the model) must connect to insertNewStrings(), and swapRows() for handling Add, Up, and Down buttons.
 * Further, the model must provide an implementation of removeRows() to make the remove button work.

@author Thomas Friedrichsmeier
*/
class RKMultiStringSelectorV2 : public QWidget {
	Q_OBJECT
public:
	explicit RKMultiStringSelectorV2 (const QString& label, QWidget* parent = 0);
	virtual ~RKMultiStringSelectorV2 ();
	void setModel (QAbstractItemModel *model, int main_column=-1);
public slots:
	void buttonClicked ();
	void updateButtons ();
	void anyModelDataChange ();
protected:
	QTreeView* tree_view;
	QPushButton* add_button;
	QPushButton* remove_button;
	QPushButton* up_button;
	QPushButton* down_button;
signals:
	void insertNewStrings (int above_row);
	void swapRows (int rowa, int rowb);
/** emitted whenever there is a change in the data selection */
	void listChanged ();
};

/** This convenience widget allows to select one or more strings (e.g. filenames) and sort them in any order. The function to acutally select new strings to add to the selection is not implemented in this class for more flexibility. Rather, connect to the getNewStrings () signal and assign the desired QString(s) in a custom slot.

@author Thomas Friedrichsmeier
*/
class MultiStringSelector : public RKMultiStringSelectorV2 {
Q_OBJECT
public:
	explicit MultiStringSelector (const QString& label, QWidget* parent = 0);
	~MultiStringSelector ();

/** get list of current strings (in the correct order, of course) */
	QStringList getValues ();
/** set list of strings. Any strings previously selected are discarded */
	void setValues (const QStringList& values);
private:
	QStringListModel *model;
private slots:
	void insertNewStringsImpl (int above_row);
	void swapRowsImpl (int rowa, int rowb);
signals:
/** This signal is triggered, when the "Add"-button is pressed. Connect to this to your custom slot, and add strings to the (empty) string_list. If you don't touch the string_list or set it to empty, nothing will be added to the list. */
	void getNewStrings (QStringList *string_list);
};

#endif
