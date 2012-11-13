/***************************************************************************
                          rkoptionset  -  description
                             -------------------
    begin                : Mon Oct 31 2011
    copyright            : (C) 2011, 2012 by Thomas Friedrichsmeier
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
#include <QTreeWidget>
#include <QHeaderView>
#include <QPushButton>

#include <klocale.h>
#include <kvbox.h>

#include "rkstandardcomponent.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardicons.h"
#include "../misc/xmlhelper.h"

#include "../debug.h"

RKOptionSet::RKOptionSet (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	XMLHelper *xml = XMLHelper::getStaticHelper ();
	updating_from_contents = updating_from_storage = false;
	last_known_status = Processing;

	min_rows = xml->getIntAttribute (element, "min_rows", 0, DL_INFO);
	min_rows_if_any = xml->getIntAttribute (element, "min_rows_if_any", 1, DL_INFO);
	max_rows = xml->getIntAttribute (element, "max", INT_MAX, DL_INFO);

	// create some meta properties
	active_row = -1;
	current_row = new RKComponentPropertyInt (this, false, -1);
	current_row->setInternal (true);
	addChild ("current_row", current_row);		// NOTE: read-write
	connect (current_row, SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (currentRowPropertyChanged(RKComponentPropertyBase*)));
	row_count = new RKComponentPropertyInt (this, false, 0);
	row_count->setInternal (true);
	addChild ("row_count", row_count);		// NOTE: read-only

	// first build the contents, as we will need to refer to the elements inside, later
	QVBoxLayout *layout = new QVBoxLayout (this);
	KVBox *contents_box = new KVBox (this);
	layout->addWidget (contents_box);

	display = 0;	// will be created from the builder, on demand -> createDisplay ()
	contents_container = new RKComponent (this, contents_box);
	RKComponentBuilder *builder = new RKComponentBuilder (contents_container, QDomElement ());
	builder->buildElement (xml->getChildElement (element, "content", DL_ERROR), contents_box, false);	// NOTE that parent widget != parent component, here, by intention. The point is that the display should not be disabled along with the contents
	builder->makeConnections ();
	addChild ("contents", contents_container);
	contents_container->fetchPropertyValuesRecursive (&default_row_state);
	contents_container->enablednessProperty ()->setBoolValue (false);	// no current row; Do this *after* fetching default values, however

	// create columns
	XMLChildList options = xml->getChildElements (element, "option", DL_WARNING);

	QStringList visible_column_labels ("#");	// Optionally hidden first row for index
	for (int i = 0; i < options.size (); ++i) {
		const QDomElement &e = options.at (i);
		QString id = xml->getStringAttribute (e, "id", QString (), DL_ERROR);
		QString label = xml->getStringAttribute (e, "label", QString (), DL_DEBUG);
		QString governor = xml->getStringAttribute (e, "connect", QString (), DL_INFO);
		bool external = xml->getBoolAttribute (e, "external", false, DL_INFO);

		while (child_map.contains (id) || (id.startsWith ("_row"))) {
			RK_DO (qDebug ("optionset already contains a property named %s. Renaming to _%s", qPrintable (id), qPrintable (id)), PLUGIN, DL_ERROR);
			id = "_" + id;
		}

		ColumnInfo col_inf;
		col_inf.column_name = id;
		col_inf.external = external;
		col_inf.governor = governor;
#warning TODO: Do we have to wait for the parent component to settle before (re-)fetching defaults?
#warning -------------- TODO ------------- Do not store defaults per column. Use only implicit defaults, instead.
		if (e.hasAttribute ("default")) col_inf.default_value = xml->getStringAttribute (e, "default", QString (), DL_ERROR);
		else if (!governor.isEmpty ()) col_inf.default_value = contents_container->fetchStringValue (governor);

		RKComponentPropertyStringList *column_property = new RKComponentPropertyStringList (this, false);
		addChild (id, column_property);
		connect (column_property, SIGNAL (valueChanged(RKComponentPropertyBase *)), this, SLOT (columnPropertyChanged(RKComponentPropertyBase *)));

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

	keycolumn = 0;
	QString keycol = xml->getStringAttribute (element, "keycolumn", QString (), DL_DEBUG);
	if (!keycol.isEmpty ()) {
		keycolumn = static_cast<RKComponentPropertyStringList*> (child_map.value (keycol));
		if (!column_map.contains (keycolumn)) {
			RK_DO (qDebug ("optionset does not contain a column named %s. Falling back to manual insertion mode", qPrintable (keycol)), PLUGIN, DL_ERROR);
			keycolumn = 0;
		} else if (!column_map[keycolumn].external) {
			RK_DO (qDebug ("keycolumn (%s) is not marked as external. Falling back to manual insertion mode", qPrintable (keycol)), PLUGIN, DL_ERROR);
			keycolumn = 0;
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
						RK_DO (qDebug ("Cannot connect external column '%s' in optionset to property with modifier (%s).", qPrintable (ci.column_name), qPrintable (ci.governor)), PLUGIN, DL_ERROR);
						continue;
					} else if (gov_prop->isInternal ()) {
						RK_DO (qDebug ("Cannot connect external column '%s' in optionset to property (%s), which is marked 'internal'.", qPrintable (ci.column_name), qPrintable (ci.governor)), PLUGIN, DL_ERROR);
						continue;
					}
				}
				columns_to_update.insertMulti (gov_prop, it.key ());
				connect (gov_prop, SIGNAL (valueChanged(RKComponentPropertyBase *)), this, SLOT (governingPropertyChanged(RKComponentPropertyBase *)));
			} else {
				RK_DO (qDebug ("did not find governing property %s for column %s of optionset", qPrintable (ci.governor), qPrintable (ci.column_name)), PLUGIN, DL_ERROR);
			}
		}
	}

	if (display) {		// may or may not have been created
		model->column_labels = visible_column_labels;
		display->setItemsExpandable (false);
		display->setRootIsDecorated (false);
		if (display_show_index) display->resizeColumnToContents (0);
		else display->setColumnHidden (0, true);
		display->setModel (model);
		display->setSelectionBehavior (QAbstractItemView::SelectRows);
		display->setSelectionMode (QAbstractItemView::SingleSelection);
		connect (display->selectionModel (), SIGNAL (selectionChanged(QItemSelection,QItemSelection)), this, SLOT (currentRowChanged()));

		if (keycolumn) display_buttons->setVisible (false);
		else {
			connect (add_button, SIGNAL (clicked()), this, SLOT (addRow()));
			connect (remove_button, SIGNAL (clicked()), this, SLOT (removeRow()));
		}
	}

	n_invalid_rows = n_unfinished_rows = 0;
	updateVisuals ();
}

RKOptionSet::~RKOptionSet () {
	RK_TRACE (PLUGIN);
}

RKComponent *RKOptionSet::createDisplay (bool show_index, QWidget *parent) {
	RK_TRACE (PLUGIN);

	RKComponent* dummy = new RKComponent (this, parent);
	QVBoxLayout *layout = new QVBoxLayout (dummy);
	layout->setContentsMargins (0, 0, 0, 0);
	KHBox *box = new KHBox (dummy);
	layout->addWidget (box);

	if (display) {
		RK_DO (qDebug ("cannot create more than one optiondisplay per optionset"), PLUGIN, DL_ERROR);
	} else {
		display = new QTreeView (box);
		display_show_index = show_index;
		model = new RKOptionSetDisplayModel (this);
	}

	display_buttons = new KHBox (dummy);
	layout->addWidget (display_buttons);
	add_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionInsertRow), QString (), display_buttons);
	RKCommonFunctions::setTips (i18n ("Add a row / element"), add_button);
	remove_button = new QPushButton (RKStandardIcons::getIcon (RKStandardIcons::ActionDeleteRow), QString (), display_buttons);
	RKCommonFunctions::setTips (i18n ("Remove a row / element"), remove_button);

	return (dummy);
}

void RKOptionSet::addRow () {
	RK_TRACE (PLUGIN);

	int row = active_row + 1;	// append feels more natural than insert, here
	int nrows = rowCount ();
	if (row <= 0) row = nrows;
	RK_ASSERT (!keycolumn);

	if (display) model->beginInsertRows (QModelIndex (), row, row);
	// adjust values
	QMap<RKComponentPropertyStringList *, ColumnInfo>::iterator it = column_map.begin ();
	for (; it != column_map.end (); ++it) {
		RKComponentPropertyStringList* col = it.key ();
		ColumnInfo &column = it.value ();
		QStringList values = col->values ();
		values.insert (row, getDefaultValue (column, row));
		col->setValues (values);
	}

	// adjust status info
	RowInfo ri (default_row_state);
	ri.valid = false;
	ri.finished = false;
	rows.insert (row, ri);
	++n_unfinished_rows;
	++n_invalid_rows;

	row_count->setIntValue (nrows + 1);
	storeRowSerialization (active_row);
	current_row->setIntValue (active_row = row);
	setContentsForRow (row);
	if (display) model->endInsertRows ();

	changed ();
}

void RKOptionSet::removeRow () {
	RK_TRACE (PLUGIN);

	int row = active_row;
	int nrows = rowCount ();
	if (row < 0) {
		RK_ASSERT (false);
		return;
	}
	RK_ASSERT (!keycolumn);

	if (display) model->beginRemoveRows (QModelIndex (), row, row);
	// adjust values
	QMap<RKComponentPropertyStringList *, ColumnInfo>::iterator it = column_map.begin ();
	for (; it != column_map.end (); ++it) {
		RKComponentPropertyStringList* col = it.key ();
		QStringList values = col->values ();
		values.removeAt (row);
		col->setValues (values);
	}

	// adjust status info
	if (!rows[row].valid) --n_invalid_rows;
	if (!rows[row].finished) --n_unfinished_rows;
	rows.removeAt (row);

	--row;
	if ((row < 0) && (nrows > 1)) row = 0;
	row_count->setIntValue (nrows - 1);
	current_row->setIntValue (active_row = row);
	setContentsForRow (row);
	if (display) model->endRemoveRows ();

	changed ();
}

QString getDefaultValue (const RKOptionSet::ColumnInfo& ci, int row) {
	// let's not trace this simple helper fun
	Q_UNUSED (row);
#warning TODO: should also allow scripted defaults
	return ci.default_value;
}

void RKOptionSet::setRowState (int row, bool finished, bool valid) {
	RK_ASSERT (row < rows.size ());
	if (rows[row].finished != finished) {
		rows[row].finished = finished;
		finished ? --n_unfinished_rows : ++n_unfinished_rows;
	}
	if (rows[row].valid != valid) {
		rows[row].valid = valid;
		valid ? --n_invalid_rows : ++n_invalid_rows;
	}
}

void RKOptionSet::changed () {
	int row = active_row;

	if (row > 0) {
		ComponentStatus cs = contents_container->recursiveStatus ();
		setRowState (row, cs != Processing, cs == Satisfied);
	}

	ComponentStatus s = recursiveStatus ();
	if (s != last_known_status) {
		last_known_status = s;
		updateVisuals ();
	}

	RKComponent::changed ();
}

// This function is called when a property of the current row of the optionset changes
void RKOptionSet::governingPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	int row = active_row;
	if (row < 0) return;
	if (updating_from_storage) return;
	updating_from_contents = true;

	QList<RKComponentPropertyStringList *> cols = columns_to_update.values (property);
	for (int i = 0; i < cols.size (); ++i) {
		RKComponentPropertyStringList *target = cols.at (i);
		ColumnInfo &inf = column_map[target];
		QString value = property->value (inf.governor_modifier);
		target->setValueAt (row, value);

		if (model && (inf.display_index >= 0)) {
			model->dataChanged (model->index (inf.display_index, row), model->index (inf.display_index, row));
		}
	}

	updating_from_contents = false;
}

// This function is called, when a column of the set is changed, typically from external logic
void RKOptionSet::columnPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (updating_from_contents) return;

	RKComponentPropertyStringList *target = static_cast<RKComponentPropertyStringList *> (property);
	RK_ASSERT (column_map.contains (target));
	ColumnInfo& ci = column_map[target];
	if (!ci.external) {
		RK_ASSERT (false);
		return;
	}

	if (target == keycolumn) handleKeycolumnUpdate ();
	else if (model) model->dataChanged (model->index (ci.display_index, 0), model->index (ci.display_index, model->rowCount ()));
}

void RKOptionSet::handleKeycolumnUpdate () {
	RK_TRACE (PLUGIN);

	int activate_row = activate_row;
	QStringList new_keys = keycolumn->values ();
	QMap<int, int> position_changes;

	int pos;
	for (pos = 0; pos < new_keys.size (); ++pos) {
		QString key = new_keys[pos];
		if (old_keys.value (pos) != key) {	// NOTE: old_keys could be shorter than new_keys!
			int old_pos = old_keys.indexOf (key);	// NOTE: -1 for key no longer present
			position_changes.insert (pos, old_pos);
		}
	}

	if (position_changes.isEmpty () && (old_keys.size () == new_keys.size ())) {
		return;	// no change
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
			QMap<int, int>::const_iterator pit = position_changes.find (pos);
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
		col->setValues (new_values);
	}

	// update status info
	QList<RowInfo> new_row_info = rows;
	for (int i = (new_keys.size () - new_row_info.size ()); i > 0; --i) new_row_info.append (RowInfo (default_row_state));
	for (int pos = 0; pos < new_keys.size (); ++pos) {
		QMap<int, int>::const_iterator pit = position_changes.find (pos);
		if (pit != position_changes.constEnd ()) {	// some change
			int old_pos = pit.value ();
			if (old_pos < 0) {	// a new key
				new_row_info.insert (pos, RowInfo (default_row_state));
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
	activate_row = qMin (nrows - 1, activate_row);
	current_row->setIntValue (active_row = activate_row);
	if (model) model->triggerReset ();
	changed ();
}

void RKOptionSet::applyContentsFromExternalColumn (RKComponentPropertyStringList* column, int row) {
	RK_TRACE (PLUGIN);

	const ColumnInfo &ci = column_map[column];
	if (!ci.external) return;

	QString dummy;
	RKComponentBase *governor = contents_container->lookupComponent (ci.governor, &dummy);
	if (governor && governor->isProperty ()) {
		RK_ASSERT (dummy.isEmpty ());

		QString value;
		if (row >= 0) value = column->valueAt (row);
		else value = getDefaultValue (ci, row);

		static_cast<RKComponentPropertyBase*> (governor)->setValue (value);
	} else {
		RK_DO (qDebug ("Lookup error while trying to restore row %d of optionset: %s. Remainder: %s", row, qPrintable (ci.governor), qPrintable (dummy)), PLUGIN, DL_WARNING);
		RK_ASSERT (false);
	}
}

void RKOptionSet::setContentsForRow (int row) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (rows.size () > row);
	if (row >= 0) {
		contents_container->setPropertyValues (&(rows[row].full_row_serialization), false);
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

void RKOptionSet::updateVisuals () {
	RK_TRACE (PLUGIN);

	if (!display) return;

	QPalette palette = display->header ()->palette ();
	if (isInactive ()) {
		palette.setColor (QPalette::Window, QColor (200, 200, 200));
	} else if (!isSatisfied ()) {
		palette.setColor (QPalette::Window, QColor (255, 0, 0));
	} else {
		palette.setColor (QPalette::Window, QColor (255, 255, 255));
	}
	display->header ()->setPalette (palette);
}

void RKOptionSet::storeRowSerialization (int row) {
	RK_TRACE (PLUGIN);

	if (row < 0) return;	// No row was active
	RK_ASSERT (rows.size () > row);
	rows[row].full_row_serialization.clear ();
	contents_container->fetchPropertyValuesRecursive (&(rows[row].full_row_serialization));
}

int getCurrentRowFromDisplay (QTreeView* display) {
	QModelIndexList l = display->selectionModel ()->selectedRows ();
	if (l.isEmpty ()) return -1;
	return (l[0].row ());
}

void setCurrentRowInDisplay (QTreeView* display, int row) {
	if (row < 0) display->selectionModel ()->clearSelection ();
	else {
		display->selectionModel ()->select (display->model ()->index (row, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
}

void RKOptionSet::currentRowChanged () {
	RK_TRACE (PLUGIN);

	RK_ASSERT (display);
	int r = getCurrentRowFromDisplay (display);
	if (active_row != r) current_row->setIntValue (r);
	// --> currentRowPropertyChanged ()
}

void RKOptionSet::currentRowPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (property == current_row);
	int row = current_row->intValue ();
	if (row != active_row) {	// May or may not be the case. True, e.g. if a row was removed
		storeRowSerialization (active_row);
		active_row = row;
		setContentsForRow (active_row);
	}

	if (display) setCurrentRowInDisplay (display, row);	// Doing this unconditionally helps fixing up selection problems
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
	connect (&reset_timer, SIGNAL (timeout()), this, SLOT (doResetNow()));
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
		if (column == 0) return QVariant (QString::number (row + 1));
		RKComponentPropertyStringList *p = set->visible_columns.value (column - 1);
		if (p) {
			return QVariant (p->valueAt (row));
		} else {
			RK_ASSERT (false);
		}
	}

	return QVariant ();
}

void RKOptionSetDisplayModel::doResetNow () {
	RK_TRACE (PLUGIN);
	emit (layoutChanged ());
}

QVariant RKOptionSetDisplayModel::headerData (int section, Qt::Orientation orientation, int role) const {
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			return (column_labels.value (section));
		}
	}
	return QVariant ();
}

void RKOptionSetDisplayModel::triggerReset() {
	RK_TRACE (PLUGIN);
	if (!reset_timer.isActive ()) {
		emit (layoutAboutToBeChanged ());
		reset_timer.start ();
	}
}


#include "rkoptionset.moc"
