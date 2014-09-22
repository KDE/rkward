/***************************************************************************
                          rkinput  -  description
                             -------------------
    begin                : Sat Mar 10 2005
    copyright            : (C) 2005, 2006, 2007, 2012, 2014 by Thomas Friedrichsmeier
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

#include <QTextEdit>
#include <qlineedit.h>
#include <qlabel.h>
#include <QVBoxLayout>

#include <klocale.h>

#include "../misc/xmlhelper.h"
#include "../rkglobals.h"
#include "../debug.h"

RKInput::RKInput (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	textedit = 0;
	lineedit = 0;

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create and add property
	addChild ("text", text = new RKComponentPropertyBase (this, false));
	connect (text, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (textChanged (RKComponentPropertyBase *)));

	setRequired (xml->getBoolAttribute (element, "required", false, DL_INFO));
	connect (requirednessProperty (), SIGNAL (valueChanged (RKComponentPropertyBase*)), this, SLOT (requirednessChanged (RKComponentPropertyBase*)));

	// do all the layouting
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	QString label_string = xml->getStringAttribute (element, "label", i18n ("Enter text"), DL_INFO);
	if (!label_string.isEmpty ()) {
		QLabel *label = new QLabel (label_string, this);
		vbox->addWidget (label);
	}

	int size = xml->getMultiChoiceAttribute (element, "size", "small;medium;large", 1, DL_INFO);
	if (size == 2) {
		textedit = new QTextEdit (this);
		QFontMetrics fm = QFontMetrics (textedit->currentFont ());
		int lheight = fm.lineSpacing ();
		int margin = fm.descent () + 2;
		textedit->setMinimumSize (250, lheight * 4 + margin);

		vbox->addWidget (textedit);
		connect (textedit, SIGNAL (textChanged ()), SLOT (textChanged ()));
	} else {
		lineedit = new QLineEdit (this);
		vbox->addWidget (lineedit);
		connect (lineedit, SIGNAL (textChanged (const QString&)), SLOT (textChanged (const QString&)));
	}

	vbox->addStretch (1);		// to keep the label attached

	// initialize
	updating = false;
	// DO NOT replace "" with QString::null, here! it is important, that this is actually an empty string, not a null string.
	text->setValue (xml->getStringAttribute (element, "initial", "", DL_INFO));
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

	QWidget *widget = lineedit;
	if (!widget) widget = textedit;
	RK_ASSERT (widget);

	QPalette palette = widget->palette ();
	if (isEnabled ()) {
		if (isSatisfied ()) {
			palette.setColor (widget->backgroundRole (), QColor (255, 255, 255));
		} else {
			palette.setColor (widget->backgroundRole (), QColor (255, 0, 0));
		}
	} else {
		palette.setColor (widget->backgroundRole (), QColor (200, 200, 200));
	}
	widget->setPalette (palette);
}

void RKInput::requirednessChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	updateColor ();
}

void RKInput::textChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;

	RK_ASSERT (textedit || lineedit);
	if (textedit) textedit->setText (fetchStringValue (text));
	else lineedit->setText (fetchStringValue (text));

	updateColor ();

	updating = false;
	changed ();
}

void RKInput::textChanged (const QString &new_text) {
	RK_TRACE (PLUGIN);

	updating = true;

	text->setValue (new_text);
	updateColor ();

	updating = false;
	changed ();
}

void RKInput::textChanged () {
	RK_TRACE (PLUGIN);

	RK_ASSERT (textedit);
	textChanged (textedit->toPlainText ());
}

bool RKInput::isValid () {
	RK_TRACE (PLUGIN);

	return (!(fetchStringValue (text).isEmpty ()));
}

#include "rkinput.moc"
