/***************************************************************************
                          rkoptionset  -  description
                             -------------------
    begin                : Mon Oct 31 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#include "rkoptionset.h"

#include <QVBoxLayout>
#include <QTableWidget>

#include <klocale.h>
#include <kvbox.h>

#include "rkstandardcomponent.h"
#include "../misc/xmlhelper.h"

#include "../debug.h"

RKOptionSet::RKOptionSet (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget, RKStandardComponentBuilder *builder) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = XMLHelper::getStaticHelper ();
	updating_from_contents = changing_row = false;

	connect (standardComponent (), SIGNAL (componentChanged(RKComponent*)), this, SLOT (componentChangeComplete(RKComponent*)));

	// create some meta properties
	current_row = new RKComponentPropertyInt (this, false, -1);
	row_count->setInternal (true);
	addChild ("current_row", current_row);
	connect (current_row, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (currentRowPropertyChanged(RKComponentPropertyBase*)));
	row_count = new RKComponentPropertyInt (this, false, 0);
	row_count->setInternal (true);
	addChild ("row_count", row_count);

	// first build the contents, as we will need to refer to the elements inside, later
	QVBoxLayout *layout = new QVBoxLayout (this);
	KVBox *contents_box = new KVBox (this);
	layout->addWidget (contents_box);

	display = 0;	// will be created from the builder, on demand -> createDisplay ()
	container = new RKComponent (this, contents_box);
	RKComponentBuilder *builder = new RKComponentBuilder (container, QDomElement ());
	builder->buildElement (xml->getChildElement (element, "content", DL_ERROR), contents_box, false);
	// take a snapshot of the default state
	container->fetchPropertyValuesRecursive (&defaults);

	// create columns
	XMLChildList options = xml->getChildElements (element, "option", DL_WARNING);

	int visible_columns = 0;
	for (int i = 0; i < options.size (); ++i) {
		const QDomElement &e = options.at (i);
		QString id = xml->getStringAttribute (e, "id", QString (), DL_ERROR);
		QString label = xml->getStringAttribute (e, "label", QString (), DL_DEBUG);
		QString governor = xml->getStringAttribute (e, "connect", QString (), DL_WARNING);
		bool restorable = xml->getBoolAttribute (e, "restorable", true, DL_INFO);

		while (columns_to_governors.contains (id) || child_map.contains (id)) {
			RK_DO (qDebug ("optionset already contains a property named %s. Renaming to _%s", qPrintable (id), qPrintable (id)), PLUGIN, DL_ERROR);
			id = "_" + id;
		}

		ColumnInfo col_inf;
		col_inf.column_name = id;
		col_inf.restorable = restorable;
		col_inf.governor = governor;
		if (!label.isEmpty ()) {
			col_inf.display_index = visible_columns++;
			col_inf.column_label = label;
		} else {
			col_inf.display_index = -1;
		}

		RKComponentPropertyStringList *column_property = new RKComponentPropertyStringList (this, false);
		addChild (id, column_property);
		connect (column_property, SIGNAL (valueChanged(RKComponentPropertyBase *)), this, SLOT (columnPropertyChanged(RKComponentPropertyBase *)));
		column_map.insert (column_property, col_inf);
	}

	keycolumn = 0;
	QString keycol = xml->getStringAttribute (element, "keycolumn", QString (), DL_DEBUG);
	if (!keycol.isEmpty ()) {
		if (!columns_to_governors.contains (keycol)) {
			RK_DO (qDebug ("optionset does not contain a column named %s. Falling back to manual insertion mode", qPrintable (keycol)), PLUGIN, DL_ERROR);
		} else {
			keycolumn = static_cast<RKComponentPropertyStringList*> (child_map[keycol]);
		}
	}

	QMap<RKComponentPropertyStringList *, ColumnInfo>::const_iterator it = column_map.constBegin ();
	for (; it != column_map.constEnd (); ++it) {
		ColumnInfo &ci = it.value ();
		if (!ci.governor.isEmpty ()) {		// there *can* be columns without governor. This allows to connect two option-sets, e.g. on different pages of a tabbook, manually
			// Establish connections between columns and their respective governors. Since the format differs, the connection is done indirectly, through this component.
			// So, here, we set up a map of properties to columns, and connect to the change signals.
			RKComponentBase *governor = container->lookupComponent (ci.governor, &ci.governor_modifier);
			if (governor && governor->isProperty ()) {
				RKComponentPropertyBase *gov_prop = static_cast<RKComponentPropertyBase*> (governor);
				if (ci.restorable) {
					if (!modifier.isEmpty ()) {
						RK_DO (qDebug ("Cannot connect restorable column '%s' in optionset to property with modifier (%s). Add an auxiliary non-restorable column, instead.", qPrintable (ci.column_name), qPrintable (ci.governor)), PLUGIN, DL_ERROR);
					} else if (gov_prop->isInternal ()) {
						RK_DO (qDebug ("Cannot connect restorable column '%s' in optionset to property (%s), which is marked 'internal'. Add an auxiliary non-restorable column, instead.", qPrintable (ci.column_name), qPrintable (ci.governor)), PLUGIN, DL_ERROR);
					}
					continue;
				}
				columns_to_update.insertMulti (gov_prop, it.key ());
				connect (gov_prop, SIGNAL (valueChanged(RKComponentPropertyBase *)), this, SLOT (governingPropertyChanged(RKComponentPropertyBase *)));
			} else {
				RK_DO (qDebug ("did not find governing property %s for column %s of optionset", qPrintable (ci.governor), qPrintable (ci.column_name)), PLUGIN, DL_ERROR);
			}
		}
	}

	if (display) {		// may or may not have been created
		QVector<QString> headers (visible_columns, QString ());
		QMap<RKComponentPropertyStringList *, ColumnInfo>::const_iterator it = column_map.constBegin ();
		for (; it != column_map.constEnd (); ++it) {
			const ColumnInfo &inf = it.value ();
			if (inf.display_index >= 0) {
				RK_ASSERT (headers.size () >= inf.display_index);
				headers[inf.display_index] = inf.column_label;
			}
		}
		display->setColumnCount (headers.size ());
		display->setHorizontalHeaderLabels (QStringList (headers.toList ()));
		display->verticalHeader ()->setVisible (display_show_index);
	}
}

RKOptionSet::~RKOptionSet () {
	RK_TRACE (PLUGIN);
}

QWidget *RKOptionSet::createDisplay (bool show_index, QWidget *parent) {
	RK_TRACE (PLUGIN);

	if (display) {
		RK_DO (qDebug ("cannot create more than one optiondisplay per optionset"), PLUGIN, DL_ERROR);
	} else {
		display = new QTableWidget (parent);
		display_show_index = show_index;
	}
	return (display);
}

void RKOptionSet::governingPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (changing_row) return;
	updating_from_contents = true;

	int row = current_row->intValue ();
	if (row < 0) {
		RK_ASSERT (false);
		return;
	}

	QList<RKComponentPropertyStringList *> cols = columns_to_update.values (property);
	for (int i = 0; i < cols.size (); ++i) {
		RKComponentPropertyStringList *target = cols.at (i);
		const ColumnInfo &inf = column_map[target];
		QString value = property->value (inf.governor_modifier);
		target->setValueAt (row, value);

		if (display && (inf.display_index >= 0)) {
			display->setItem (row, inf.display_index, new QTableWidgetItem (value));
		}
	}

	updating_from_contents = false;
}

void RKOptionSet::columnPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (updating_from_contents) return;

	const ColumnInfo &inf = column_map[target];
	if (inf.display_index >= 0) {
		QString value = property->value (inf.display_modifier);
		display->setItem (row, inf.display_index, new QTableWidgetItem (value));
	}

#error TODO
}

void RKOptionSet::currentRowPropertyChanged (RKComponentPropertyBase *property);
