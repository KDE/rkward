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
class QListView;
class QListViewItem;

class RKVarSelector;
class RKVariable;
class RContainerObject;

#define VARSLOT_WIDGET 1

/** An RKVarSlot takes one or more variable(s) from an RKVarSelector.
  *@author Thomas Friedrichsmeier
  */

class RKVarSlot : public RKPluginWidget {
	Q_OBJECT
public: 
	RKVarSlot(const QDomElement &element, QWidget *parent, RKPlugin *plugin);
	~RKVarSlot();
	int getNumVars () { return num_vars; };
	int getNumCont () { return num_cont; };
	QValueList<RKVariable*> getVariables ();
  	int type() {return VARSLOT_WIDGET ;};
  	void  setEnabled(bool);
  
public slots:
/** Called when the select-button is pressed */
	void selectPressed ();
	void listSelectionChanged ();
/// find out whether all items are still present, remove items which are no longer present and update text for all others
	void objectListChanged ();
  	void slotActive();
  	void slotActive(bool);
private:
	QLineEdit *line_edit;
	QListView *list;
	QPushButton *select;
	RKVarSelector *source;
	QString source_id;
	QString depend;
	QString varchier;
	int min_vars;
	int num_vars;
	int num_cont;
	bool multi;
	bool required;
	bool selection;
	typedef QMap<QListViewItem*, RKVariable*> ItemMap; 
	ItemMap item_map;
	typedef QMap<QListViewItem*, RContainerObject*> ContMap; 
	ContMap cont_map;
	QString classes ; 
	void updateState ();
	bool belongToClasses (const QString &nom) ;
	bool varOrCont ;
protected:
	bool isSatisfied ();
	QString value (const QString &modifier);
	QString complaints ();
	void initialize ();
};

#endif
