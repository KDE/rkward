/***************************************************************************
                          rkinput  -  description
                             -------------------
    begin                : Sat Mar 10 2005
    copyright            : (C) 2005-2018 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
	bool isValid ();
public slots:
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
