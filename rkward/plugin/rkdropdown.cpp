/***************************************************************************
                          rkdropdown.h  -  description
                             -------------------
    begin                : Fri Jan 12 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include "rkdropdown.h"

#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlistbox.h>

#include <klocale.h>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKDropDown::RKDropDown (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKAbstractOptionSelector (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create layout
	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());

	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Select one:"), DL_INFO), this);
	vbox->addWidget (label);

	// create ComboBox
	box = new QComboBox (false, this);
	if (!(box->listBox ())) {
		// make sure the combo box uses a list box internally
		box->setListBox (new QListBox (this));
	}

	addOptionsAndInit (element);

	vbox->addWidget (box);
	connect (box, SIGNAL (activated (int)), this, SLOT (comboItemActivated (int)));
}

RKDropDown::~RKDropDown(){
	RK_TRACE (PLUGIN);
}

void RKDropDown::comboItemActivated (int id) {
	RK_TRACE (PLUGIN);

	QListBox *list = box->listBox ();
	RK_ASSERT (list);
	QListBoxItem *item = list->item (id);
	RK_ASSERT (item);
	if (!item->isSelectable ()) return;		// yes, apparently not selectable items can be "activated"

	itemSelected (id);
}

void RKDropDown::setItemInGUI (int id) {
	RK_TRACE (PLUGIN);

	box->setCurrentItem (id);
}

void RKDropDown::addOptionToGUI (const QString &label, int id) {
	RK_TRACE (PLUGIN);

	box->insertItem (label, id);
}

void RKDropDown::setItemEnabledInGUI (int id, bool enabled) {
	RK_TRACE (PLUGIN);

	QListBox *list = box->listBox ();
	RK_ASSERT (list);
	QListBoxItem *item = list->item (id);
	RK_ASSERT (item);

	if (item->rtti () != ID_RKDROPDOWNLISTITEM) {
		// this item won't show whether it is enabled or not. We need to replace it.
		item = new RKDropDownListItem (0, list->text (id));
		list->changeItem (item, id);
	}

	item->setSelectable (enabled);
}

////////////////// RKDropDownListItem ////////////////////////

#include <qpainter.h>

RKDropDownListItem::RKDropDownListItem (QListBox *listbox, const QString &text) : QListBoxText (listbox, text) {
	RK_TRACE (PLUGIN);
}

void RKDropDownListItem::paint (QPainter *painter) {
	// no trace!
	if (!isSelectable ()) {
		painter->setPen (QColor (150, 150, 150));
	}

	QListBoxText::paint (painter);
}

#include "rkdropdown.moc"
