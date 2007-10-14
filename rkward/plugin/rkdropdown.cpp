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
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <klocale.h>

#include "../rkglobals.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKDropDown::RKDropDown (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKAbstractOptionSelector (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create layout
	Q3VBoxLayout *vbox = new Q3VBoxLayout (this, RKGlobals::spacingHint ());

	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Select one:"), DL_INFO), this);
	vbox->addWidget (label);

	// create ComboBox
	box = new QComboBox (false, this);

	addOptionsAndInit (element);

	vbox->addWidget (box);
	connect (box, SIGNAL (activated (int)), this, SLOT (comboItemActivated (int)));
}

RKDropDown::~RKDropDown(){
	RK_TRACE (PLUGIN);
}

void RKDropDown::comboItemActivated (int id) {
	RK_TRACE (PLUGIN);

//KDE4: TODO!!!
	// make sure only selectable items get selected

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

//	QModelIndex offset = box->model ()->createIndex (id, box->modelColumn ());

//KDE4 TODO: enable/disable item
//	item->setSelectable (enabled);
}

/*
////////////////// RKDropDownListItem ////////////////////////

#include <qpainter.h>

RKDropDownListItem::RKDropDownListItem (Q3ListBox *listbox, const QString &text) : Q3ListBoxText (listbox, text) {
	RK_TRACE (PLUGIN);
}

void RKDropDownListItem::paint (QPainter *painter) {
	// no trace!
	if (!isSelectable ()) {
		painter->setPen (QColor (150, 150, 150));
	}

	Q3ListBoxText::paint (painter);
}*/

#include "rkdropdown.moc"
