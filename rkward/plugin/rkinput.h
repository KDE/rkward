/*
rkinput - This file is part of the RKWard project. Created: Sat Mar 10 2005
SPDX-FileCopyrightText: 2005-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKINPUT_H
#define RKINPUT_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

class QTextEdit;
class QLineEdit;
class QDomElement;

/** A component to enter plain text

@author Adrien d'Hardemare
*/
class RKInput : public RKComponent {
	Q_OBJECT
public:
	RKInput (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKInput ();

	RKComponentPropertyBase *text;
	QVariant value (const QString &modifier=QString ()) override { return (text->value (modifier)); };
	QStringList getUiLabelPair () const override;
	int type () override { return ComponentInput; };
	bool isValid () override;
public Q_SLOTS:
	void textChanged ();
	void textChangedFromUi ();
	void requirednessChanged (RKComponentPropertyBase *);
protected:
/** Grey out input when disabled */
	void changeEvent (QEvent *event) override;
private:
	void updateColor ();
	bool updating;
	QTextEdit *textedit;
	QLineEdit *lineedit;
	QString label_string;
};

#endif
