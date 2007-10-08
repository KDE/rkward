/***************************************************************************
                          rkformula  -  description
                             -------------------
    begin                : Thu Aug 12 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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

class Q3ListView;
class QPushButton;
class Q3ButtonGroup;
class QWidget;
class QSpinBox;
class Q3ListViewItem;
class QDomElement;

/**
@author Thomas Friedrichsmeier
*/
class RKFormula : public RKComponent {
  Q_OBJECT
public:
	RKFormula (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKFormula ();

	QString value (const QString &modifier) { return model->value (modifier); };
	bool isSatisfied ();

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
	Q3ButtonGroup *type_selector;
	
	QWidget *custom_model_widget;
	Q3ListView *model_view;
	Q3ListView *predictors_view;
	QPushButton *add_button;
	QPushButton *remove_button;
	QSpinBox *level_box;
	
	void checkCustomModel ();
	void makeModelString ();
	QString mangleName (RObject *var);
	
	typedef QMap<Q3ListViewItem*, RObject*> ItemMap;
	ItemMap item_map;
	
	struct Interaction {
		int level;
		RObjectPtr* vars;
	};
	
	typedef QMap<Q3ListViewItem*, Interaction> InteractionMap;
	InteractionMap interaction_map;
	
	typedef QMap<int, Q3ListViewItem*> LevelMap;
	LevelMap level_map;
	
	/** recursively cross the given source variables on level level. Returns the resulting terms in an array. The number
	of interactions generated is stored in count */
	Interaction *makeInteractions (int level, const RObjectPtr *source_vars, int source_count, int *count);
};

#endif
