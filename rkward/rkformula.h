/***************************************************************************
                          rkformula  -  description
                             -------------------
    begin                : Thu Aug 12 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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

#include <rkpluginwidget.h>

#include <qstring.h>
#include <qmap.h>
#include <rkvariable.h>

class QListView;
class QPushButton;
class QButtonGroup;
class QWidget;
class QSpinBox;
class QListViewItem;
class RKVarSlot;

/**
@author Thomas Friedrichsmeier
*/
class RKFormula : public RKPluginWidget {
  Q_OBJECT
public:
    RKFormula(const QDomElement &element, QWidget *parent, RKPlugin *plugin, QLayout *layout);

    ~RKFormula();
public slots:
	void typeChange (int id);
	void addButtonClicked ();
	void removeButtonClicked ();
	void factorsChanged ();
private:
	QString model_string;
	QString table_string;
	bool multitable;
	bool model_ok;
	typedef QMap<QString, RKVariable*> MangledNames;
	MangledNames mangled_names;

	enum ModelType { FullModel=0, MainEffects=1, Custom=2 };
	ModelType model_type;
	QButtonGroup *type_selector;
	
	QWidget *custom_model_widget;
	QListView *model_view;
	QListView *predictors_view;
	
	QPushButton *add_button;
	QPushButton *remove_button;
	QSpinBox *level_box;
	
	void checkCustomModel ();
	void makeModelString ();
	QString mangleName (RKVariable *var);
	
	typedef QMap<QListViewItem*, RKVariable*> ItemMap;
	ItemMap item_map;
	
	struct Interaction {
		int level;
		RKVarPtr* vars;
	};
	
	typedef QMap<QListViewItem*, Interaction> InteractionMap;
	InteractionMap interaction_map;
	
	typedef QMap<int, QListViewItem*> LevelMap;
	LevelMap level_map;
	
	/** recursively cross the given source variables on level level. Returns the resulting terms in an array. The number
	of interactions generated is stored in count */
	Interaction *makeInteractions (int level, const RKVarPtr *source_vars, int source_count, int *count);
	
	QString fixed_factors_id;
	QString dependent_id;
	RKVarSlot *fixed_factors;
	RKVarSlot *dependent;
protected:
	bool isSatisfied ();
	QString value (const QString &modifier);
	void initialize ();
};

#endif
