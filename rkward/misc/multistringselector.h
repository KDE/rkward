/*
multistringselector - This file is part of the RKWard project. Created: Fri Sep 10 2005
SPDX-FileCopyrightText: 2005-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	explicit RKMultiStringSelectorV2(const QString& label, QWidget* parent = nullptr);
	virtual ~RKMultiStringSelectorV2();
	void setModel (QAbstractItemModel *model, int main_column=-1);
	void setAlwaysAddAtBottom (bool always_add_at_bottom) { add_at_bottom = always_add_at_bottom; };
public Q_SLOTS:
	void buttonClicked ();
	void updateButtons ();
	void anyModelDataChange ();
protected:
	QTreeView* tree_view;
	QPushButton* add_button;
	QPushButton* remove_button;
	QPushButton* up_button;
	QPushButton* down_button;
	bool add_at_bottom;
Q_SIGNALS:
	void insertNewStrings (int above_row);
	void swapRows (int rowa, int rowb);
/** emitted whenever there is a change in the data selection */
	void listChanged ();
};

/** This convenience widget allows to select one or more strings (e.g. filenames) and sort them in any order. The function to actually select new strings to add to the selection is not implemented in this class for more flexibility. Rather, connect to the getNewStrings () signal and assign the desired QString(s) in a custom slot.

@author Thomas Friedrichsmeier
*/
class MultiStringSelector : public RKMultiStringSelectorV2 {
Q_OBJECT
public:
	explicit MultiStringSelector(const QString& label, QWidget* parent = nullptr);
	~MultiStringSelector();

/** get list of current strings (in the correct order, of course) */
	QStringList getValues ();
/** set list of strings. Any strings previously selected are discarded */
	void setValues (const QStringList& values);
private:
	QStringListModel *model;
private Q_SLOTS:
	void insertNewStringsImpl (int above_row);
	void swapRowsImpl (int rowa, int rowb);
Q_SIGNALS:
/** This signal is triggered, when the "Add"-button is pressed. Connect to this to your custom slot, and add strings to the (empty) string_list. If you don't touch the string_list or set it to empty, nothing will be added to the list. */
	void getNewStrings (QStringList *string_list);
};

#endif
