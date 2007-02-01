/***************************************************************************
                          rkinput  -  description
                             -------------------
    begin                : Sat Mar 10 2005
    copyright            : (C) 2005, 2006, 2007 by Thomas Friedrichsmeier
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

#include "rkinput.h"

#include <qlayout.h>
#include <qtextedit.h>
#include <qlabel.h>

#include <klocale.h>

#include "../misc/xmlhelper.h"
#include "../rkglobals.h"
#include "../debug.h"

RKInput::RKInput (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create and add property
	addChild ("text", text = new RKComponentPropertyBase (this, false));
	connect (text, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (textChanged ( RKComponentPropertyBase *)));

	setRequired (xml->getBoolAttribute (element, "required", false, DL_INFO));
	connect (requirednessProperty (), SIGNAL (valueChanged (RKComponentPropertyBase*)), this, SLOT (requirednessChanged (RKComponentPropertyBase*)));

	// do all the layouting
	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	QLabel *label = new QLabel (xml->getStringAttribute (element, "label", i18n ("Enter text"), DL_INFO), this);
	vbox->addWidget (label);

	textedit = new QTextEdit (this);
	int size = xml->getMultiChoiceAttribute (element, "size", "small;medium;large", 1, DL_INFO);
	int lheight = textedit->fontMetrics ().lineSpacing ();
	int margin = textedit->height () - textedit->visibleHeight () + textedit->fontMetrics ().descent () + 2;
	if (size == 0) {
		textedit->setFixedSize (100, lheight + margin);
		textedit->setHScrollBarMode (QScrollView::AlwaysOff);
		textedit->setVScrollBarMode (QScrollView::AlwaysOff);
	} else if (size == 1) {
		textedit->setFixedSize (250, lheight + margin);
		textedit->setHScrollBarMode (QScrollView::AlwaysOff);
		textedit->setVScrollBarMode (QScrollView::AlwaysOff);
	} else if (size == 2) {
		textedit->setMinimumSize (250, lheight * 4 + margin);
	}
	vbox->addWidget (textedit);
	connect (textedit, SIGNAL (textChanged ()), SLOT (textChanged ()));

	vbox->addStretch (1);		// to keep the label attached

	// initialize
	updating = false;
	text->setValue (xml->getStringAttribute (element, "initial", QString::null, DL_INFO));
}

RKInput::~RKInput () {
	RK_TRACE (PLUGIN);
}

void RKInput::enabledChange (bool old) {
	RK_TRACE (PLUGIN);

	updateColor ();

	RKComponent::enabledChange (old);
}

void RKInput::updateColor () {
	RK_TRACE (PLUGIN);

	if (isEnabled ()) {
		if (isSatisfied ()) {
			textedit->setPaletteBackgroundColor (QColor (255, 255, 255));
		} else {
			textedit->setPaletteBackgroundColor (QColor (255, 0, 0));
		}
	} else {
		textedit->setPaletteBackgroundColor (QColor (200, 200, 200));
	}
}

void RKInput::requirednessChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	updateColor ();
}

void RKInput::textChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;

	textedit->setText (text->value ());
	updateColor ();

	updating = false;
	changed ();
}

void RKInput::textChanged () {
	RK_TRACE (PLUGIN);

	updating = true;

	text->setValue (textedit->text ());
	updateColor ();

	updating = false;
	changed ();
}

bool RKInput::isValid () {
	RK_TRACE (PLUGIN);

	return (!(text->value ().isEmpty ()));
}

#include "rkinput.moc"
