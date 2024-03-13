/*
rkvalueselector - This file is part of RKWard (https://rkward.kde.org). Created: Weg May 8 2013
SPDX-FileCopyrightText: 2013-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkvalueselector.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QStringListModel>

#include "../misc/xmlhelper.h"

#include "../debug.h"

RKValueSelector::RKValueSelector (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	updating = false;
	XMLHelper *xml = parent_component->xmlHelper ();
	standalone = element.tagName () == "select";

	addChild ("selected", selected = new RKComponentPropertyStringList (this, false));
	connect (selected, &RKComponentPropertyBase::valueChanged, this, &RKValueSelector::selectionPropertyChanged);
	selected->setInternal (!standalone);
	addChild ("available", available = new RKComponentPropertyStringList (this, false));
	connect (available, &RKComponentPropertyBase::valueChanged, this, &RKValueSelector::availablePropertyChanged);
	available->setInternal (true);
	addChild ("labels", labels = new RKComponentPropertyStringList (this, false));
	connect (labels, &RKComponentPropertyBase::valueChanged, this, &RKValueSelector::labelsPropertyChanged);
	labels->setInternal (true);

	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);

	label_string = xml->i18nStringAttribute (element, "label", QString (), DL_INFO);
	if (!label_string.isNull ()) {
		QLabel *label = new QLabel (label_string, this);
		vbox->addWidget (label);
	}

	list_view = new QTreeView (this);
	list_view->setHeaderHidden (true);
	if (standalone && xml->getBoolAttribute (element, "single", false, DL_INFO)) {
		list_view->setSelectionMode (QAbstractItemView::SingleSelection);
	} else {
		list_view->setSelectionMode (QAbstractItemView::ExtendedSelection);
	}
	list_view->setRootIsDecorated (false);
	model = new QStringListModel (this);
	list_view->setModel (model);
	connect (list_view->selectionModel (), &QItemSelectionModel::selectionChanged, this, &RKValueSelector::listSelectionChanged);

	vbox->addWidget (list_view);

	XMLChildList options = xml->getChildElements (element, "option", DL_INFO);
	if (!options.isEmpty ()) {
		QStringList values_list;
		QStringList labels_list;
		QStringList selected_list;

		for (int i = 0; i < options.size (); ++i) {
			const QDomElement &child = options[i];
			QString v = xml->getStringAttribute (child, "value", QString (), DL_WARNING);
			QString l = xml->i18nStringAttribute (child, "label", v, DL_INFO);
			if (xml->getBoolAttribute (child, "checked", false, DL_INFO)) selected_list.append (v);
			labels_list.append (l);
			values_list.append (v);
		}
		available->setValueList (values_list);
		labels->setValueList (labels_list);
		selected->setValueList (selected_list);
	}
}

RKValueSelector::~RKValueSelector () {
	RK_TRACE (PLUGIN);
}

static QStringList mergeLists (const QStringList &labels, const QStringList &ids) {
	if (labels.size () < ids.size ()) {
		return labels + (ids.mid (labels.size ()));
	} else if (labels.size () > ids.size ()) {
		return (labels.mid (0, ids.size ()));
	}
	return labels;
}

void RKValueSelector::labelsPropertyChanged () {
	RK_TRACE (PLUGIN);

	model->setStringList (mergeLists (labels->values (), available->values ()));
	selectionPropertyChanged ();   // To update selected items
}

void RKValueSelector::availablePropertyChanged () {
	RK_TRACE (PLUGIN);

	const QStringList &vals = available->values ();
	for (int i = vals.size () - 1; i >= 1; --i) {
		if (vals.lastIndexOf (vals[i], i - 1) >= 0) {
			RK_DEBUG (PLUGIN, DL_WARNING, "Duplicate value index in value selector: %s", qPrintable (vals[i]));
		}
	}
	model->setStringList (mergeLists (labels->values (), available->values ()));

	if (!purged_selected_indexes.isEmpty ()) {
		// This is to handle the case that the "selected" property was updated externally, *before* the "available" property got the corresponding change.
		// In this case, try to re-apply any selected strings that could not be matched, before
		purged_selected_indexes.append (selected->values ());
		selected->setValueList (purged_selected_indexes);   // side effect updating selected items
		purged_selected_indexes.clear ();
	} else {
		selectionPropertyChanged ();   // To update selected items
		purged_selected_indexes.clear ();
	}
}

void RKValueSelector::listSelectionChanged () {
	if (updating) return;
	RK_TRACE (PLUGIN);

	purged_selected_indexes.clear ();
	QStringList sel_list;
	QModelIndexList selected_rows = list_view->selectionModel ()->selectedRows ();
	for (int i = 0; i < selected_rows.size (); ++i) {
		sel_list.append (available->valueAt (selected_rows[i].row ()));
	}
	updating = true;
	selected->setValueList (sel_list);
	updating = false;

	changed ();
}

void RKValueSelector::selectionPropertyChanged () {
	if (updating) return;
	RK_TRACE (PLUGIN);

	updating = true;
	QSet<int> selected_rows;
	for (int i = 0; i < selected->listLength (); ++i) {
		const QString &val = selected->valueAt (i);
		int index = available->values ().indexOf (val);
		if (index < 0) {
			if (!purged_selected_indexes.contains (val)) purged_selected_indexes.append (val);
			selected->removeAt (i);
			--i;
			continue;
		}
		selected_rows.insert (index);
	}

	list_view->selectionModel ()->clearSelection ();
	for (const int row : std::as_const(selected_rows)) {
		list_view->selectionModel ()->select (model->index (row), QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}
	updating = false;

	changed ();
}

QVariant RKValueSelector::value (const QString& modifier) {
	RK_TRACE (PLUGIN);

	if (modifier == "labeled") {
		QStringList selected_labels;
		for (int i = 0; i < selected->listLength (); ++i) {
			int index = available->values ().indexOf (selected->valueAt (i));
			if (index < 0) {
				RK_ASSERT (index >= 0);
			} else selected_labels.append (labels->valueAt (index));
		}
		return QVariant (selected_labels);
	}
	return selected->value (modifier);
}

QStringList RKValueSelector::getUiLabelPair () const {
	RK_TRACE (PLUGIN);

	QStringList ret (label_string);
	ret.append (const_cast<RKValueSelector *> (this)->value ("labeled").toStringList ().join ("; "));
	return ret;
}

