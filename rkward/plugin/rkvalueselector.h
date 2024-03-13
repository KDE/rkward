/*
rkvalueselector - This file is part of the RKWard project. Created: Weg May 8 2013
SPDX-FileCopyrightText: 2013-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	int type () override { return ComponentValueSelector; };
	QVariant value (const QString &modifier=QString ()) override;
	QStringList getUiLabelPair () const override;
private Q_SLOTS:
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
	QString label_string;
};

#endif
