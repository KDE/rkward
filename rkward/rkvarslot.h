/***************************************************************************
                          rkvarslot.h  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#ifndef RKVARSLOT_H
#define RKVARSLOT_H

#include <rkpluginwidget.h>

#include <qmap.h>

class QLineEdit;
class QPushButton;
class QVarSelector;
class QListView;
class QListViewItem;

/** An RKVarSlot takes one or more variable(s) from an RKVarSelector.
  *@author Thomas Friedrichsmeier
  */

class RKVarSlot : public RKPluginWidget {
	Q_OBJECT
public: 
	RKVarSlot(const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout);
	~RKVarSlot();
public slots:
/** Called when the select-button is pressed */
	void selectPressed ();
	void listSelectionChanged ();
private:
	QLineEdit *line_edit;
	QListView *list;
	QPushButton *select;
	QString source_id;
	int min_vars;
	int num_vars;
	bool multi;
	bool required;
	bool selection;
	typedef QMap<QListViewItem*, int> ItemMap; 
	ItemMap item_map;
	void updateState ();
protected:
	bool isSatisfied ();
	QString value (const QString &modifier);
	QString complaints ();
};

#endif
