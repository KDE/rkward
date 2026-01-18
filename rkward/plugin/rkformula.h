/*
rkformula - This file is part of the RKWard project. Created: Thu Aug 12 2004
SPDX-FileCopyrightText: 2004-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKFORMULA_H
#define RKFORMULA_H

#include "rkcomponent.h"

#include "../core/robject.h"
#include <QHash>
#include <qstring.h>

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
	RKFormula(const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKFormula() override;

	QVariant value(const QString &modifier = QString()) override { return model->value(modifier); };
	QStringList getUiLabelPair() const override;
	bool isValid() override;

	/** RTTI */
	int type() override { return ComponentFormula; };
  public Q_SLOTS:
	void typeChange(int id);
	void addButtonClicked();
	void removeButtonClicked();
	void factorsChanged(RKComponentPropertyBase *);

  private:
	RKComponentPropertyRObjects *fixed_factors;
	RKComponentPropertyRObjects *dependent;
	RKComponentPropertyBase *model;
	RKComponentPropertyBase *table;
	RKComponentPropertyBase *labels;

	bool multitable;
	bool model_ok;
	typedef QMap<QString, const RObject *> MangledNames;
	MangledNames mangled_names;

	enum ModelType { FullModel = 0,
		             MainEffects = 1,
		             Custom = 2 };
	ModelType model_type;
	QButtonGroup *type_selector;

	QWidget *custom_model_widget;
	QTreeWidget *model_view;
	QTreeWidget *predictors_view;
	QPushButton *add_button;
	QPushButton *remove_button;
	QSpinBox *level_box;
	QString label_string;

	void checkCustomModel();
	void makeModelString();
	QString mangleName(const RObject *var);

	typedef QHash<QTreeWidgetItem *, RObject *> ItemMap;
	ItemMap predictors_map;

	struct Interaction {
		int level;
		RObject::ObjectList vars;
	};

	typedef QHash<QTreeWidgetItem *, Interaction> InteractionMap;
	InteractionMap interaction_map;

	/** recursively cross the given source variables on level level. Returns the resulting terms in an QList */
	QList<Interaction> makeInteractions(int level, RObject::ObjectList);
};

#endif
