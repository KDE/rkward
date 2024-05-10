/*
rkoptionset - This file is part of RKWard (https://rkward.kde.org). Created: Mon Oct 31 2011
SPDX-FileCopyrightText: 2011-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkoptionset.h"

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QMimeData>

#include <KLocalizedString>

#include "rkstandardcomponent.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkaccordiontable.h"
#include "../misc/rkstandardicons.h"
#include "../misc/xmlhelper.h"

#include "../debug.h"

#define KEYCOLUMN_UNINITIALIZED_VALUE QString ("___#!RK!___Keycol_uninitialized")

RKOptionSet::RKOptionSet (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = parent_component->xmlHelper ();
	updating = false;
	last_known_status = Processing;
	n_invalid_rows = n_unfinished_rows = 0;

	min_rows = xml->getIntAttribute (element, "min_rows", 0, DL_INFO);
	min_rows_if_any = xml->getIntAttribute (element, "min_rows_if_any", 1, DL_INFO);
	max_rows = xml->getIntAttribute (element, "max_rows", INT_MAX, DL_INFO);

	// build UI framework
	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	accordion = new RKAccordionTable (this);
	layout->addWidget (accordion);

	connect (accordion, &RKAccordionTable::activatedRow, this, &RKOptionSet::currentRowChanged);
	connect (accordion, &RKAccordionTable::addRow, this, &RKOptionSet::addRow);
	connect (accordion, &RKAccordionTable::removeRow, this, &RKOptionSet::removeRow);

	updating_notice = new QLabel (i18n ("Updating status, please wait"), this);
	layout->addWidget (updating_notice);
	update_timer.setInterval (0);
	update_timer.setSingleShot (true);
	connect (&update_timer, &QTimer::timeout, this, &RKOptionSet::slotUpdateUnfinishedRows);

	// create some meta properties
	serialization_of_set = new RKComponentPropertyBase (this, false);
	addChild ("serialized", serialization_of_set);
	connect (serialization_of_set, &RKComponentPropertyBase::valueChanged, this, &RKOptionSet::serializationPropertyChanged);

	row_count = new RKComponentPropertyInt (this, false, 0);
	row_count->setInternal (true);
	addChild ("row_count", row_count);		// NOTE: read-only
	return_to_row = active_row = -1;
	current_row = new RKComponentPropertyInt (this, false, active_row);
	current_row->setInternal (true);
	addChild ("current_row", current_row);		// NOTE: read-write
	connect (current_row, &RKComponentPropertyBase::valueChanged, this, &RKOptionSet::currentRowPropertyChanged);

	// first build the contents, as we will need to refer to the elements inside, later
	model = new RKOptionSetDisplayModel (this);
	contents_container = new RKComponent (this, accordion->editorWidget ());
	accordion->editorWidget ()->layout ()->addWidget (contents_container);
	QDomElement content_element = xml->getChildElement (element, "content", DL_ERROR);
	RKComponentBuilder *builder = new RKComponentBuilder (contents_container, content_element);
	builder->buildElement (content_element, *xml, accordion->editorWidget (), false);	// NOTE that parent widget != parent component, here, by intention. The point is that the display should not be disabled along with the contents
	builder->parseLogic (xml->getChildElement (element, "logic", DL_INFO), *xml, false);
	builder->makeConnections ();
	addChild ("contents", contents_container);
	connect (standardComponent (), &RKStandardComponent::standardInitializationComplete, this, &RKOptionSet::fetchDefaults);

	// create columns
	XMLChildList options = xml->getChildElements (element, "optioncolumn", DL_WARNING);

	QStringList visible_column_labels;
	for (int i = 0; i < options.size (); ++i) {
		const QDomElement &e = options.at (i);
		QString id = xml->getStringAttribute (e, "id", QString (), DL_ERROR);
		QString label = xml->i18nStringAttribute (e, "label", QString (), DL_DEBUG);
		QString governor = xml->getStringAttribute (e, "connect", QString (), DL_INFO);
		bool external = xml->getBoolAttribute (e, "external", false, DL_INFO);

		while (child_map.contains (id)) {
			RK_DEBUG (PLUGIN, DL_ERROR, "optionset already contains a property named %s. Renaming to _%s", qPrintable (id), qPrintable (id));
			id = '_' + id;
		}

		ColumnInfo col_inf;
		col_inf.column_name = id;
		col_inf.external = external;
		col_inf.governor = governor;
		if (external && e.hasAttribute ("default")) col_inf.default_value = xml->getStringAttribute (e, "default", QString (), DL_ERROR);

		RKComponentPropertyStringList *column_property = new RKComponentPropertyStringList (this, false);
		column_property->setInternal (external);	// Yes, looks strange, indeed. External properties should simply not be serialized / restored...
		addChild (id, column_property);
		connect (column_property, &RKComponentPropertyBase::valueChanged, this, &RKOptionSet::columnPropertyChanged);

		if (!label.isEmpty ()) {
			col_inf.display_index = visible_column_labels.size ();
			col_inf.column_label = label;
			visible_column_labels.append (label);
			visible_columns.append (column_property);
		} else {
			col_inf.display_index = -1;
		}

		column_map.insert (column_property, col_inf);
	}

	keycolumn = nullptr;
	QString keycol = xml->getStringAttribute (element, "keycolumn", QString (), DL_DEBUG);
	if (!keycol.isEmpty ()) {
		keycolumn = static_cast<RKComponentPropertyStringList*> (child_map.value (keycol));
		if (!column_map.contains (keycolumn)) {
			RK_DEBUG (PLUGIN, DL_ERROR, "optionset does not contain an optioncolumn named %s. Falling back to manual insertion mode", qPrintable (keycol));
			keycolumn = nullptr;
		} else if (!column_map[keycolumn].external) {
			RK_DEBUG (PLUGIN, DL_ERROR, "keycolumn (%s) is not marked as external. Falling back to manual insertion mode", qPrintable (keycol));
			keycolumn = nullptr;
		} else {
			updating = true;
			keycolumn->setValue (KEYCOLUMN_UNINITIALIZED_VALUE);
			updating = false;
		}
	}

	QMap<RKComponentPropertyStringList *, ColumnInfo>::iterator it = column_map.begin ();
	for (; it != column_map.end (); ++it) {
		ColumnInfo &ci = it.value ();
		if (!ci.governor.isEmpty ()) {		// there *can* be columns without governor for driven or connected option sets
			// Establish connections between columns and their respective governors. Since the format differs, the connection is done indirectly, through this component.
			// So, here, we set up a map of properties to columns, and connect to the change signals.
			RKComponentBase *governor = contents_container->lookupComponent (ci.governor, &ci.governor_modifier);
			if (governor && governor->isProperty ()) {
				RKComponentPropertyBase *gov_prop = static_cast<RKComponentPropertyBase*> (governor);
				if (ci.external) {
					if (!ci.governor_modifier.isEmpty ()) {
						RK_DEBUG (PLUGIN, DL_ERROR, "Cannot connect external column '%s' in optionset to property with modifier (%s).", qPrintable (ci.column_name), qPrintable (ci.governor));
						continue;
					}
				}
				columns_to_update.insert(gov_prop, it.key());
				connect (gov_prop, &RKComponentPropertyBase::valueChanged, this, &RKOptionSet::governingPropertyChanged);
			} else {
				RK_DEBUG (PLUGIN, DL_ERROR, "did not find governing property %s for column %s of optionset", qPrintable (ci.governor), qPrintable (ci.column_name));
			}
		}
	}

	model->column_labels = visible_column_labels;
	accordion->setShowAddRemoveButtons (!keycolumn);
	accordion->setModel (model);
	updating_notice->hide();
	QSizePolicy pol(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	pol.setVerticalStretch(1);  // For some reason, this apparently defaults to 0 - no stretch, unless absolutely necessary - for this widget, although I have no idea, why.
	setSizePolicy(pol);
}

RKOptionSet::~RKOptionSet () {
	RK_TRACE (PLUGIN);
}

void RKOptionSet::fetchDefaults () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (default_row_state.isEmpty ());
	contents_container->fetchPropertyValuesRecursive (&default_row_state, false, QString (), true);
	if (min_rows && !keycolumn && (rowCount () <= 0)) addRow (rowCount ());
	contents_container->enablednessProperty ()->setBoolValue (rowCount () > 0);	// no current row; Do this *after* fetching default values, however. Otherwise most values will *not* be read, as the element is disabled
}

QString serializeList (const QStringList &list) {
	QString ret;
	for (int i = 0; i < list.size (); ++i) {
		if (i > 0) ret.append ('\t');
		ret.append (RKCommonFunctions::escape (list[i]));
	}
	return ret;
}

QStringList unserializeList  (const QString &serial) {
	QStringList ret = serial.split ('\t', Qt::KeepEmptyParts);
	for (int i = 0; i < ret.size (); ++i) {
		ret[i] = RKCommonFunctions::unescape (ret[i]);
	}
	return ret;
}

QString serializeMap (const RKComponent::PropertyValueMap &map) {
	QString ret;

	RKComponent::PropertyValueMap::const_iterator it;
	for (it = map.constBegin (); it != map.constEnd (); ++it) {
		if (!ret.isEmpty ()) ret.append ('\t');
		ret.append (RKCommonFunctions::escape (it.key () + '=' + it.value ()));
	}
	return ret;
}

RKComponent::PropertyValueMap unserializeMap (const QString &serial) {
	RKComponent::PropertyValueMap ret;
	QStringList l = serial.split ('\t', Qt::KeepEmptyParts);
	for (int i = 0; i < l.size (); ++i) {
		QString &line = l[i];
		int sep = line.indexOf ('=');
		ret.insert (RKCommonFunctions::unescape (line.left (sep)), RKCommonFunctions::unescape (line.mid (sep+1)));
	}
	return ret;
}

void RKOptionSet::fetchPropertyValuesRecursive (PropertyValueMap *list, bool include_top_level, const QString &prefix, bool include_inactive_elements) const {
	RK_TRACE (PLUGIN);
	RK_ASSERT (include_top_level);
	RK_ASSERT (!include_inactive_elements);

	QString serialization;

	if (keycolumn) {
		serialization.append ("keys=" + serializeList (old_keys));
	}

	for (int r = 0; r < rows.size (); ++r) {
		if (!serialization.isEmpty ()) serialization.append ("\n");
		if (r == active_row) {
			PropertyValueMap map;	// current row may have changes which have not yet been stored to the state map
			contents_container->fetchPropertyValuesRecursive (&map);
			serialization.append ("_row=" + serializeMap (map));
		} else {
			serialization.append ("_row=" + serializeMap (rows[r].full_row_map));
		}
	}

	list->insert (prefix + "serialized", serialization);
}

void RKOptionSet::serializationPropertyChanged (RKComponentPropertyBase* property) {
	if (updating) return;
	updating = true;
	if (model) Q_EMIT model->layoutAboutToBeChanged();

	RK_TRACE (PLUGIN);
	RK_ASSERT (property == serialization_of_set);
/* What happens when deserializing a plugin, with a driven optionset, and
 * the property connected to the keycolumn is restored *before* the optionset itself has been de-serialized?
 * 
 * We have to special-case this: If we go into setPropertyValues, and
 * the key column has already been touched, we have to
 * - apply property values from the serialization
 * - trigger handleKeycolumnUpdate(), delayed
 * 
 */
	bool update_key_col = false;
	if (keycolumn && (keycolumn->value () != KEYCOLUMN_UNINITIALIZED_VALUE)) {
		update_key_col = true;
	} else {
		RK_ASSERT (rows.isEmpty ());
	}

	QList<RowInfo> new_rows;
	int row = 0;
	QStringList items = fetchStringValue (property).split ('\n');
	bool keys_missing = (keycolumn != nullptr);
	for (int i = 0; i < items.size (); ++i) {
		const QString &item = items[i];
		int sep = item.indexOf ('=');
		if (sep < 0) {
			RK_DEBUG (PLUGIN, DL_WARNING, "Bad format while trying to de-serialize optionset, line %d", i);
			continue;
		}
		QString tag = item.left (sep);
		QString value = item.mid (sep + 1);

		if (tag == QLatin1String ("keys")) {
			if (!keys_missing) {
				RK_DEBUG (PLUGIN, DL_WARNING, "Unexpected specification of keys while trying to de-serialize optionset, line %d", i);
				continue;
			}
			old_keys = unserializeList (value);
			keys_missing = false;
		} else if (tag == QLatin1String ("_row")) {
			new_rows.append (RowInfo (unserializeMap (value)));
			++row;
		} else {
			RK_DEBUG (PLUGIN, DL_WARNING, "Unexpected tag %s while trying to de-serialize optionset, line %d", qPrintable (tag), i);
			continue;
		}
	}

	if (keycolumn) {
		RK_ASSERT (new_rows.size () == old_keys.size ());
		RK_ASSERT (!keys_missing);
	}

	// reset all non-external columns to default values
	QMap<RKComponentPropertyStringList*, ColumnInfo>::const_iterator it;
	for (it = column_map.constBegin (); it != column_map.constEnd (); ++it) {
		const ColumnInfo &col = it.value ();
		if (col.external) continue;
		QStringList def;
		for (int i = 0; i < row; ++i) {
			def.append (getDefaultValue (col, i));
		}
		it.key ()->setValueList (def);
	}

	rows = new_rows;
	n_unfinished_rows = n_invalid_rows = row;
	row_count->setIntValue (row);
	serialization_of_set->setValue (QString ());	// free some mem, and don't make users think, this can actually be queried in real-time
	updating = false;
	if (update_key_col) handleKeycolumnUpdate ();

	active_row = -1;
	current_row->setIntValue (qMin (0, row - 1));

	if (model) Q_EMIT model->layoutChanged();
	changed ();
}

void RKOptionSet::slotUpdateUnfinishedRows () {
	updateUnfinishedRows ();
}

void RKOptionSet::updateUnfinishedRows () {
	if (updating) return;
	if ((active_row >= 0) && (active_row < rows.size ())) {
		if (!rows[active_row].finished) {
			return;	// if the current row is unfinished: let's wait for this one, first.
		}
	}

	RK_TRACE (PLUGIN);

	if (!n_unfinished_rows) {	// done
		if (!updating_notice->isVisible()) return;
		current_row->setIntValue (return_to_row);
		accordion->show();
		updating_notice->hide();
		return;
	}

	if (!updating_notice->isVisible()) {
		updating_notice->show();
		accordion->hide();
		return_to_row = active_row;
	}
	for (int i = 0; i < rows.size (); ++i) {
		if (!rows[i].finished) {
			current_row->setIntValue (i);
			return;
		}
	}

	RK_ASSERT (false);	// This would mean, we did not find any unfinished row, even though we tested for n_unfinished_rows, above.
}

void RKOptionSet::addRow (int row) {
	RK_TRACE (PLUGIN);

	storeRowSerialization (active_row);

	int nrows = rowCount ();
	if (row < 0) row = nrows;
	RK_ASSERT (!keycolumn);

	model->beginInsertRows (QModelIndex (), row, row);
	// adjust values
	updating = true;
	QMap<RKComponentPropertyStringList *, ColumnInfo>::iterator it = column_map.begin ();
	for (; it != column_map.end (); ++it) {
		RKComponentPropertyStringList* col = it.key ();
		ColumnInfo &column = it.value ();
		QStringList values = col->values ();
		values.insert (row, getDefaultValue (column, row));
		col->setValueList (values);
	}
	updating = false;

	// adjust status info
	RowInfo ri (default_row_state);
	ri.valid = false;
	ri.finished = false;
	rows.insert (row, ri);
	++n_unfinished_rows;
	++n_invalid_rows;
	row_count->setIntValue (nrows + 1);
	current_row->setIntValue (active_row = row);
	setContentsForRow (active_row);
	model->endInsertRows ();

	current_row->setIntValue (row);  // Setting this _again_, as the view might have messed with it following endInsertRows ()

	changed ();
}

void RKOptionSet::removeRow (int row) {
	RK_TRACE (PLUGIN);

	int nrows = rowCount ();
	if (row < 0) {
		RK_ASSERT (false);
		return;
	}
	RK_ASSERT (!keycolumn);

	model->beginRemoveRows (QModelIndex (), row, row);
	updating = true;
	// adjust values
	QMap<RKComponentPropertyStringList *, ColumnInfo>::iterator it = column_map.begin ();
	for (; it != column_map.end (); ++it) {
		RKComponentPropertyStringList* col = it.key ();
		QStringList values = col->values ();
		values.removeAt (row);
		col->setValueList (values);
	}
	updating = false;

	// adjust status info
	if (!rows[row].valid) --n_invalid_rows;
	if (!rows[row].finished) --n_unfinished_rows;
	rows.removeAt (row);

	--row;
	if ((row < 0) && (nrows > 1)) row = 0;
	row_count->setIntValue (nrows - 1);
	current_row->setIntValue (active_row = row);
	setContentsForRow (row);
	model->endRemoveRows ();

	current_row->setIntValue (row);  // Setting this _again_, as the view might have messed with it following endRemoveRows ()

	changed ();
}

void RKOptionSet::moveRow (int old_index, int new_index) {
	RK_TRACE (PLUGIN);

	int nrows = rowCount ();
	if (old_index < 0 || old_index >= nrows) {
		RK_ASSERT (false);
		return;
	}

	if (new_index < 0 || new_index > nrows) {
		new_index = nrows;
	}

	storeRowSerialization (active_row);
	PropertyValueMap backup = rows[old_index].full_row_map;
	removeRow (old_index);
	if (new_index > old_index) new_index -= 1;
	addRow (new_index);
	rows[new_index].full_row_map = backup;
	setContentsForRow (new_index);

	changed ();
}

QString getDefaultValue (const RKOptionSet::ColumnInfo& ci, int row) {
	// let's not trace this simple helper fun
	Q_UNUSED (row);
// TODO: should also allow scripted defaults?
	return ci.default_value;
}

void RKOptionSet::setRowState (int row, bool finished, bool valid) {
	bool changed = false;
	RK_ASSERT (row < rows.size ());
	if (rows[row].finished != finished) {
		rows[row].finished = finished;
		finished ? --n_unfinished_rows : ++n_unfinished_rows;
		changed = true;
	}
	if (rows[row].valid != valid) {
		rows[row].valid = valid;
		valid ? --n_invalid_rows : ++n_invalid_rows;
		changed = true;
	}
	if (changed && model) Q_EMIT model->dataChanged(model->index(row, 0), model->index(row, model->columnCount() - 1));
}

void RKOptionSet::changed () {
	int row = active_row;

	if (row >= 0) {
		ComponentStatus cs = contents_container->recursiveStatus ();
		setRowState (row, cs != Processing, cs == Satisfied);
	}

	update_timer.start ();

	ComponentStatus s = recursiveStatus ();
	if (s != last_known_status) {
		last_known_status = s;
		if (model) Q_EMIT model->headerDataChanged(Qt::Horizontal, 0, model->columnCount() - 1);
	}

	RKComponent::changed ();
}

// This function is called when a property of the current row of the optionset changes
void RKOptionSet::governingPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	int row = active_row;
	if (row < 0) return;
	if (updating) return;
	updating = true;

	QList<RKComponentPropertyStringList *> cols = columns_to_update.values (property);
	for (int i = 0; i < cols.size (); ++i) {
		RKComponentPropertyStringList *target = cols.at (i);
		ColumnInfo &inf = column_map[target];
		QString value = fetchStringValue (property, inf.governor_modifier);
		target->setValueAt (row, value);

		if (model && (inf.display_index >= 0)) {
			Q_EMIT model->dataChanged(model->index(inf.display_index, row), model->index(inf.display_index, row));
		}
	}

	updating = false;
}

// This function is called, when a column of the set is changed, typically from external logic
void RKOptionSet::columnPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (updating) return;

	RKComponentPropertyStringList *target = static_cast<RKComponentPropertyStringList *> (property);
	RK_ASSERT (column_map.contains (target));
	ColumnInfo& ci = column_map[target];
	if (!ci.external) {
		RK_DEBUG (PLUGIN, DL_ERROR, "Column %s was touched externally, although it is not marked as external", qPrintable (ci.column_name));
		return;
	}

	if (target == keycolumn) handleKeycolumnUpdate ();
	else {
		if (model) Q_EMIT model->dataChanged(model->index(ci.display_index, 0), model->index(ci.display_index, model->rowCount()));
		applyContentsFromExternalColumn (target, active_row);
	}
}

void RKOptionSet::handleKeycolumnUpdate () {
	RK_TRACE (PLUGIN);

	int activate_row = active_row;
	QStringList new_keys = keycolumn->values ();
	QMap<int, int> position_changes;
	QSet<int> found_rows;

	for (int pos = 0; pos < new_keys.size (); ++pos) {
		QString key = new_keys[pos];
		if (old_keys.value (pos) != key) {	// NOTE: old_keys could be shorter than new_keys!
			int old_pos = old_keys.indexOf (key);	// NOTE: -1 for new keys
			if (old_pos == active_row) activate_row = pos;
			position_changes.insert (pos, old_pos);
			if (old_pos >= 0) found_rows.insert (old_pos);
		} else found_rows.insert (pos);
	}

	if (position_changes.isEmpty () && (old_keys.size () == new_keys.size ())) {
		return;	// no change
	}

	// get state of current row (which may subsequently be moved or even deleted
	storeRowSerialization (active_row);
	updating = true;

	// as a first step, take a backup of any rows that have been removed.
	for (int i = old_keys.size () - 1; i >= 0; --i) {
		if (!found_rows.contains (i)) {
			former_row_states.insert (old_keys[i], rows[i].full_row_map);
		}
	}

	// update all columns
	QMap<RKComponentPropertyStringList *, ColumnInfo>::iterator it = column_map.begin ();
	for (; it != column_map.end (); ++it) {
		RKComponentPropertyStringList* col = it.key ();
		ColumnInfo &column = it.value ();
		if (column.external) continue;

		// Ok, we'll have to adjust this column. We start by copying the old values, and padding to the
		// new length (if that is greater than the old).
		QStringList old_values = col->values ();
		QStringList new_values = old_values;
		for (int i = (new_keys.size () - new_values.size ()); i > 0; --i) new_values.append (QString ());

		// adjust all positions that have changed
		for (int pos = 0; pos < new_keys.size (); ++pos) {
			QMap<int, int>::const_iterator pit = position_changes.constFind (pos);
			if (pit != position_changes.constEnd ()) {	// some change
				int old_pos = pit.value ();
				if (old_pos < 0) {	// a new key
					new_values[pos] = getDefaultValue (column, pos);
					activate_row = pos;
				} else {	// old key changed position
					new_values[pos] = old_values[old_pos];
				} // NOTE: not visible: old key is gone without replacement
			}
		}

		// strip excess length (if any), and apply
		new_values = new_values.mid (0, new_keys.size ());
		col->setValueList (new_values);
	}

	// update status info
	QList<RowInfo> new_row_info = rows;
	for (int i = (new_keys.size () - new_row_info.size ()); i > 0; --i) new_row_info.append (RowInfo (default_row_state));
	for (int pos = 0; pos < new_keys.size (); ++pos) {
		QMap<int, int>::const_iterator pit = position_changes.constFind (pos);
		if (pit != position_changes.constEnd ()) {	// some change
			int old_pos = pit.value ();
			if (old_pos < 0) {	// a new key (but it might have been known, formerly)
				new_row_info[pos] = RowInfo (former_row_states.value (new_keys[pos], default_row_state));
			} else {	// old key changed position
				new_row_info[pos] = rows[old_pos];
			} // NOTE: not visible: old key is gone without replacement
		}
	}
	rows = new_row_info.mid (0, new_keys.size ());
	n_invalid_rows = n_unfinished_rows = 0;
	for (int i = 0; i < rows.size (); ++i) {
		if (!rows[i].finished) ++n_unfinished_rows;
		if (!rows[i].valid) ++n_invalid_rows;
	}

	old_keys = new_keys;

	int nrows = new_keys.size ();
	row_count->setIntValue (nrows);
	if (model) model->triggerReset ();
	updating = false;
	activate_row = qMin (nrows - 1, activate_row);
	setContentsForRow (active_row = activate_row);
	current_row->setIntValue (active_row);
	changed ();
}

void RKOptionSet::applyContentsFromExternalColumn (RKComponentPropertyStringList* column, int row) {
	RK_TRACE (PLUGIN);

	const ColumnInfo &ci = column_map[column];
	if (!ci.external) return;
	if (ci.governor.isEmpty ()) return;

	updating = true;
	QString dummy;
	RKComponentBase *governor = contents_container->lookupComponent (ci.governor, &dummy);
	if (governor && governor->isProperty ()) {
		RK_ASSERT (dummy.isEmpty ());

		QString value;
		if (row >= 0) value = column->valueAt (row);
		else value = getDefaultValue (ci, row);

		static_cast<RKComponentPropertyBase*> (governor)->setValue (value);
	} else {
		RK_DEBUG (PLUGIN, DL_WARNING, "Lookup error while trying to restore row %d of optionset: %s. Remainder: %s", row, qPrintable (ci.governor), qPrintable (dummy));
		RK_ASSERT (false);
	}
	updating = false;
}

void RKOptionSet::setContentsForRow (int row) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (rows.size () > row);
	if (row >= 0) {
		const PropertyValueMap *map = &(rows[row].full_row_map);
		// If some elements are disabled, these will *not* be contained in the serialization of a row (unless that is still at the default).
		// They *are* contained in the default_row_state, however, and thus we will apply any properties for which we do not have a value
		// from the default_row_state, instead.
		for (PropertyValueMap::const_iterator it = default_row_state.constBegin (); it != default_row_state.constEnd (); ++it) {
			if (!map->contains (it.key ())) {
				RKComponentPropertyBase *prop = contents_container->lookupProperty(it.key(), nullptr, true);
				if (prop) {		// found a property
					RK_ASSERT (!prop->isInternal ());
					prop->setValue (it.value ());
				}
			}
		}
		contents_container->setPropertyValues (map, false);
	} else {
		contents_container->setPropertyValues (&default_row_state, false);
	}
	QMap<RKComponentPropertyStringList *, ColumnInfo>::const_iterator it = column_map.constBegin ();
	for (; it != column_map.constEnd (); ++it) {
		RKComponentPropertyStringList* col = it.key ();
		applyContentsFromExternalColumn (col, row);
	}
	contents_container->enablednessProperty ()->setBoolValue (row >= 0);
}

void RKOptionSet::storeRowSerialization (int row) {
	RK_TRACE (PLUGIN);

	if (row < 0) return;	// No row was active
	RK_ASSERT (rows.size () > row);
	rows[row].full_row_map.clear ();
	contents_container->fetchPropertyValuesRecursive (&(rows[row].full_row_map));
}

void RKOptionSet::updateCurrentRowInDisplay () {
	if (!(accordion && model)) return;	// can happen during initialization

	if (active_row < 0) accordion->collapseAll ();
	else accordion->activateRow (active_row);
}

void RKOptionSet::currentRowChanged (int row) {
	RK_TRACE (PLUGIN);

	if (active_row != row) current_row->setIntValue (row);
	// --> currentRowPropertyChanged ()
}

void RKOptionSet::currentRowPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (property == current_row);
	int row = qMin (row_count->intValue () - 1, current_row->intValue ());
	if (row != active_row) {	// May or may not be the case. True, e.g. if a row was removed
		storeRowSerialization (active_row);
		active_row = row;
		setContentsForRow (active_row);
	}

	updateCurrentRowInDisplay ();	// Doing this, even if the current row _seeems_ unchanged, helps fixing up selection problems
}


/** reimplemented from RKComponent */
RKComponent::ComponentStatus RKOptionSet::recursiveStatus () {
	RK_TRACE (PLUGIN);

	ComponentStatus s = RKComponent::recursiveStatus ();
	if (s == Dead) return s;
	if (n_unfinished_rows > 0) return Processing;
	return s;
}

bool RKOptionSet::isValid () {
	if (n_invalid_rows > n_unfinished_rows) return false;
	int count = row_count->intValue ();
	if (count < min_rows) return false;
	if ((count > 0) && (count < min_rows_if_any)) return false;
	if (count > max_rows) return false;
	return true;
}




RKOptionSetDisplayModel::RKOptionSetDisplayModel (RKOptionSet* parent) : QAbstractTableModel (parent) {
	RK_TRACE (PLUGIN);
	set = parent;
	connect (&reset_timer, &QTimer::timeout, this, &RKOptionSetDisplayModel::doResetNow);
	reset_timer.setInterval (0);
	reset_timer.setSingleShot (true);
}

RKOptionSetDisplayModel::~RKOptionSetDisplayModel () {
	RK_TRACE (PLUGIN);
}

int RKOptionSetDisplayModel::columnCount (const QModelIndex& parent) const {
	if (parent.isValid ()) return 0;
	return column_labels.size ();
}

int RKOptionSetDisplayModel::rowCount (const QModelIndex& parent) const {
	if (parent.isValid ()) return 0;
	return set->rowCount ();
}

QVariant RKOptionSetDisplayModel::data (const QModelIndex& index, int role) const {
	if (!index.isValid ()) return QVariant ();
	int row = index.row ();
	int column = index.column ();

	if (role == Qt::DisplayRole) {
		RKComponentPropertyStringList *p = set->visible_columns.value (column);
		if (p) {
			return QVariant (p->valueAt (row));
		} else {
			RK_ASSERT (false);
		}
	} else if (role == Qt::BackgroundRole) {
		const RKOptionSet::RowInfo &ri = set->rows[row];
		if (!ri.finished) return QColor (Qt::yellow);
		if (!ri.valid) return QColor (Qt::red);
	} else if ((role == Qt::ToolTipRole) || role == (Qt::StatusTipRole)) {
		const RKOptionSet::RowInfo &ri = set->rows[row];
		if (!ri.finished) return i18n ("This row has not yet been processed.");
		if (!ri.valid) return i18n ("This row contains invalid settings.");
	}

	return QVariant ();
}

void RKOptionSetDisplayModel::doResetNow () {
	RK_TRACE (PLUGIN);
	Q_EMIT layoutChanged();
	set->updateCurrentRowInDisplay ();
}

QVariant RKOptionSetDisplayModel::headerData (int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) return (column_labels.value (section));
		if (role == Qt::BackgroundRole) {
			if (set->n_unfinished_rows > 0) return QColor (Qt::yellow);
			if (!set->isValid ()) return QColor (Qt::red);
		}
		if ((role == Qt::ToolTipRole) || role == (Qt::StatusTipRole)) {
			if (set->n_unfinished_rows > 0) return i18n ("Please wait while settings are being processed");
			if (!set->isValid ()) {
				QStringList probs;
				if (set->n_invalid_rows > set->n_unfinished_rows) probs.append (i18n ("One or more rows contain invalid settings."));
				if ((set->rowCount () > 0) && (set->rowCount () < set->min_rows_if_any)) probs.append (i18n ("At least %1 rows have to be defined (if any)", set->min_rows_if_any));
				if (set->rowCount () < set->min_rows) probs.append (i18n ("At least %1 rows have to be defined", set->min_rows));
				if (set->rowCount () > set->max_rows) probs.append (i18n ("At most %1 rows may be defined", set->max_rows));
				return (QString ("<p>%1</p><ul><li>%2</li></ul>").arg(i18n("This element is not valid for the following reason(s):"), probs.join("</li>\n<li>")));
			}
		}
	}
	return QVariant ();
}

void RKOptionSetDisplayModel::triggerReset() {
	RK_TRACE (PLUGIN);
	if (!reset_timer.isActive ()) {
		Q_EMIT layoutAboutToBeChanged();
		reset_timer.start ();
	}
}

QString optionsetdisplaymodel_mt ("application/x-rkaccordiontableitem");
QStringList RKOptionSetDisplayModel::mimeTypes () const {
	return QStringList (optionsetdisplaymodel_mt);
}

QMimeData* RKOptionSetDisplayModel::mimeData (const QModelIndexList& indexes) const {
	RK_ASSERT (indexes.length () >= 1);
	QMimeData *ret = new QMimeData ();
	ret->setData (optionsetdisplaymodel_mt, QByteArray (QString::number (indexes.first ().row ()).toLatin1 ()));
	return (ret);
}

bool RKOptionSetDisplayModel::dropMimeData (const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
	Q_UNUSED (column);
	if (action == Qt::IgnoreAction) return true;
	if (action == Qt::MoveAction) {
		if (parent.isValid ()) return false;
		int srow = QString::fromLatin1 (data->data (optionsetdisplaymodel_mt)).toInt ();
		set->moveRow (srow, row);
	}
	return false;
}

Qt::ItemFlags RKOptionSetDisplayModel::flags (const QModelIndex& index) const {
	return QAbstractTableModel::flags (index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions RKOptionSetDisplayModel::supportedDropActions () const {
    return Qt::MoveAction;
}

Qt::DropActions RKOptionSetDisplayModel::supportedDragActions() const {
	return Qt::MoveAction;
}
