/***************************************************************************
                          multistringselector  -  description
                             -------------------
    begin                : Fir Sep 10 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

class QListView;
class QPushButton;

/** This convenience widget allows to select one or more strings (e.g. filenames) and sort them in any order. The function to acutally select new strings to add to the selection is not implemented in this class for more flexibility. Rather, connect to the getNewStrings () signal and assign the desired QString(s) in a custom slot.

@author Thomas Friedrichsmeier
*/
class MultiStringSelector : public QWidget {
Q_OBJECT
public:
	MultiStringSelector (const QString& label, QWidget* parent = 0);

	~MultiStringSelector ();

/** get list of current strings (in the correct order, of course) */
	QStringList getValues ();
/** set list of strings. Any strings previously selected are discarded */
	void setValues (const QStringList& values);
public slots:
	void addButtonClicked ();
	void removeButtonClicked ();
	void upButtonClicked ();
	void downButtonClicked ();
	void listSelectionChanged ();
private:
	QListView* list_view;
	QPushButton* add_button;
	QPushButton* remove_button;
	QPushButton* up_button;
	QPushButton* down_button;
signals:
/** This signal is triggered, when the "Add"-button is pressed. Connect to this to your custom slot, and add strings to the (empty) string_list. If you don't touch the string_list or set it to empty, nothing will be added to the list. Of course it does not make much sense to connect multiple slots to this signal, as only the last one called would be effective. */
	void getNewStrings (QStringList *string_list);
/** emitted whenever there is a change in the user selection */
	void listChanged ();
};

#endif
