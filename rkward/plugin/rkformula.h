/***************************************************************************
                          rkformula  -  description
                             -------------------
    begin                : Thu Aug 12 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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
#ifndef RKFORMULA_H
#define RKFORMULA_H

#include "rkcomponent.h"

#include <qstring.h>
#include <qmap.h>
#include "../core/robject.h"

class QPushButton;
class QButtonGroup;
class QWidget;
class QSpinBox;
class QDomElement;
class QTreeWidget;
class QTreeWidgetItem;

/**
@author Thomas Friedrichsmeier
*/
class RKFormula : public RKComponent {
  Q_OBJECT
public:
	RKFormula (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKFormula ();

	QVariant value (const QString &modifier=QString ()) { return model->value (modifier); };
	bool isValid ();

/** RTTI */
	int type () { return ComponentFormula; };
public slots:
	void typeChange (int id);
	void addButtonClicked ();
	void removeButtonClicked ();
	void factorsChanged (RKComponentPropertyBase *);

private:
	RKComponentPropertyRObjects *fixed_factors;
	RKComponentPropertyRObjects *dependent;
	RKComponentPropertyBase *model;
	RKComponentPropertyBase *table;
	RKComponentPropertyBase *labels;

	bool multitable;
	bool model_ok;
	typedef QMap<QString, RObject*> MangledNames;
	MangledNames mangled_names;

	enum ModelType { FullModel=0, MainEffects=1, Custom=2 };
	ModelType model_type;
	QButtonGroup *type_selector;
	
	QWidget *custom_model_widget;
	QTreeWidget *model_view;
	QTreeWidget *predictors_view;
	QPushButton *add_button;
	QPushButton *remove_button;
	QSpinBox *level_box;
	
	void checkCustomModel ();
	void makeModelString ();
	QString mangleName (RObject *var);
	
	typedef QMap<QTreeWidgetItem*, RObject*> ItemMap;
	ItemMap predictors_map;
	
	struct Interaction {
		int level;
		RObject::ObjectList vars;
	};
	
	typedef QMap<QTreeWidgetItem*, Interaction> InteractionMap;
	InteractionMap interaction_map;
	
	/** recursively cross the given source variables on level level. Returns the resulting terms in an QList */
	QList<Interaction> makeInteractions (int level, RObject::ObjectList);
};

#endif
