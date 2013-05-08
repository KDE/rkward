/***************************************************************************
                          rkvalueselector  -  description
                             -------------------
    begin                : Weg May 8 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier
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

#ifndef RKVALUESELECTOR_H
#define RKVALUESELECTOR_H

#include "rkcomponent.h"
#include "rkcomponentproperties.h"

class QTreeView;
class QStringListModel;
class QDomElement;

/** Like RKVarSelector, but provides selection among an arbitrary list of strings.

@author Thomas Friedrichsmeier
*/
class RKValueSelector : public RKComponent {
	Q_OBJECT
public: 
	RKValueSelector (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKValueSelector ();
	int type () { return ComponentValueSelector; };
	QVariant value (const QString &modifier=QString ());
private slots:
	void selectionPropertyChanged ();
	void listSelectionChanged ();
	void labelsPropertyChanged ();
	void availablePropertyChanged ();
private:
	QTreeView *list_view;
	QStringListModel *model;
	bool updating;
	bool standalone;
	RKComponentPropertyStringList *selected;
	RKComponentPropertyStringList *labels;
	RKComponentPropertyStringList *available;
	QStringList purged_selected_indexes;
};

#endif
