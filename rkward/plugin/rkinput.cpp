/*
rkinput - This file is part of RKWard (https://rkward.kde.org). Created: Sat Mar 10 2005
SPDX-FileCopyrightText: 2005-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkinput.h"

#include <QTextEdit>
#include <qlineedit.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QEvent>

#include <KLocalizedString>

#include "../misc/xmlhelper.h"

#include "../debug.h"

RKInput::RKInput (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	textedit = nullptr;
	lineedit = nullptr;

	// get xml-helper
	XMLHelper *xml = parent_component->xmlHelper ();

	// create and add property
	addChild ("text", text = new RKComponentPropertyBase (this, false));
	connect (text, &RKComponentPropertyBase::valueChanged, this, &RKInput::textChanged);

	setRequired (xml->getBoolAttribute (element, "required", false, DL_INFO));
	connect (requirednessProperty (), &RKComponentPropertyBase::valueChanged, this, &RKInput::requirednessChanged);

	// do all the layouting
	QVBoxLayout *vbox = new QVBoxLayout (this);
	vbox->setContentsMargins (0, 0, 0, 0);
	label_string = xml->i18nStringAttribute (element, "label", i18n ("Enter text"), DL_INFO);
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
		connect (textedit, &QTextEdit::textChanged, this, &RKInput::textChangedFromUi);
	} else {
		lineedit = new QLineEdit (this);
		vbox->addWidget (lineedit);
		connect (lineedit, &QLineEdit::textChanged, this, &RKInput::textChangedFromUi);
	}

	vbox->addStretch (1);		// to keep the label attached

	// initialize
	updating = false;
	// DO NOT replace "" with QString (), here! it is important, that this is actually an empty string, not a null string.
	text->setValue (xml->getStringAttribute (element, "initial", "", DL_INFO));
}

RKInput::~RKInput () {
	RK_TRACE (PLUGIN);
}

void RKInput::changeEvent (QEvent *event) {
	RK_TRACE (PLUGIN);

	if (event->type () == QEvent::EnabledChange) updateColor ();
	RKComponent::changeEvent (event);
}

void RKInput::updateColor () {
	RK_TRACE (PLUGIN);

	QWidget *widget = lineedit;
	if (!widget) widget = textedit;
	RK_ASSERT (widget);

	if (isEnabled ()) {
		if (isSatisfied ()) {
			widget->setStyleSheet("");
		} else {
			widget->setStyleSheet("background: red; color: black");
		}
	} else {
		widget->setStyleSheet("background: rgb(200, 200, 200); color: black");
	}
}

void RKInput::requirednessChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	updateColor ();
}

void RKInput::textChanged () {
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

void RKInput::textChangedFromUi () {
	RK_TRACE (PLUGIN);

	updating = true;

	if (lineedit) text->setValue (lineedit->text ());
	else {
		RK_ASSERT (textedit);
		text->setValue (textedit->toPlainText ());
	}
	updateColor ();

	updating = false;
	changed ();
}

bool RKInput::isValid () {
	RK_TRACE (PLUGIN);

	return (!(fetchStringValue (text).isEmpty ()));
}

QStringList RKInput::getUiLabelPair () const {
	RK_TRACE (PLUGIN);

	QStringList ret (label_string);
	ret.append (text->value ().toString ());
	return ret;
}

