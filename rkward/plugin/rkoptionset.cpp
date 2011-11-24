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

#include <klocale.h>
#include <kvbox.h>

#include "rkstandardcomponent.h"
#include "../misc/xmlhelper.h"

#include "../debug.h"

RKOptionSet::RKOptionSet (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget, RKStandardComponentBuilder *builder) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = XMLHelper::getStaticHelper ();

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
	QQStringList column_labels;
	QMap<QString, QString> columns_to_governors;
	for (int i = 0; i < options.size (); ++i) {
		QString id = xml->getStringAttribute (options.at (i), "id", QString (), DL_ERROR);
		QString label = xml->getStringAttribute (options.at (i), "label", QString (), DL_DEBUG);
		QString governor = xml->getStringAttribute (options.at (i), "governor", QString (), DL_WARNING);

		while (columns_to_governors.contains (id) || child_map.contains (id)) {
			RK_DO (qDebug ("optionset already contains a property named %s. Renaming to _%s", qPrintable (id), qPrintable (id)), PLUGIN, DL_ERROR);
			id = "_" + id;
		}
		columns_to_governors.insert (id, governor);
		if (!label.isEmpty ()) {
			column_labels.append (label);
			column_to_display_index_map.insert (id, visible_columns++);
		}
	}

	keycolumn = xml->getStringAttribute (element, "keycolumn", QString (), DL_DEBUG);
	if (!columns_to_governors.contains (keycolumn)) {
		RK_DO (qDebug ("optionset does not contain a column named %s. Falling back to manual insertion mode", qPrintable (keycolumn)), PLUGIN, DL_ERROR);
		keycolumn.clear ();
	}

	QMap<QString, QString>::const_iterator it = columns_to_governors.constBegin ();
	for (; it != columns_to_governors.constEnd (); ++it) {
		// add columns
		RKComponentPropertyStringList *column_property = new RKComponentPropertyStringList (this, false);
		addChild (it.key (), column_property);
		connect (column_property, SIGNAL (valueChanged(RKComponentPropertyBase *)), this, SLOT (columnPropertyChanged(RKComponentPropertyBase *)));

		if (!it ().value ().isEmpty ()) {		// there *can* be columns without governor. This allows to connect two option-sets, e.g. on different pages of a tabbook.
			// Establish connections between columns and their respective governors. Since the format differs, the connection is done indirectly, through this component.
			// So, here, we set up a map of properties to columns, and connect to the change signals.
			QString modifier;
			RKComponentBase *governor = container->lookupComponent (it.value (), &modifier);
			if (governor && governor->isProperty ()) {
				RKComponentPropertyBase *gov_prop = static_cast<RKComponentPropertyBase*> (governor);
				ModifierAndColumn mac;
				mac.modifier = modifier;
				mac.column_name = it.key ();
				property_to_column_map.insertMulti (gov_prop, mac);
				connect (gov_prop, SIGNAL (valueChanged(RKComponentPropertyBase *)), this, SLOT (governingPropertyChanged(RKComponentPropertyBase *)));
			} else {
				RK_DO (qDebug ("did not find governing property %s for column %s of optionset", qPrintable (it.value ()), qPrintable (it.key ())), PLUGIN, DL_ERROR);
			}
		}
	}
}

