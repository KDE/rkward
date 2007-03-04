/***************************************************************************
                          rkpreviewbox  -  description
                             -------------------
    begin                : Wed Jan 24 2007
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
#include "rkpreviewbox.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtimer.h>

#include <klocale.h>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../misc/xmlhelper.h"
#include "../debug.h"

RKPreviewBox::RKPreviewBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	preview_active = false;
	last_plot_done = true;
	new_plot_pending = false;

	// get xml-helper
	XMLHelper *xml = XMLHelper::getStaticHelper ();

	// create and add property
	addChild ("state", state = new RKComponentPropertyBool (this, true, preview_active, "active", "inactive"));
	connect (state, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (changedState (RKComponentPropertyBase *)));

	// create checkbox
	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	toggle_preview_box = new QCheckBox (xml->getStringAttribute (element, "label", i18n ("Preview"), DL_INFO), this);
	vbox->addWidget (toggle_preview_box);
	toggle_preview_box->setChecked (preview_active);
	connect (toggle_preview_box, SIGNAL (stateChanged (int)), this, SLOT (changedState (int)));

	// status lable
	status_label = new QLabel (QString::null, this);
	vbox->addWidget (status_label);

	// find and connect to code property of the parent
	QString dummy;
	RKComponentBase *cp = parentComponent ()->lookupComponent ("code", &dummy);
	if (cp && dummy.isNull () && (cp->type () == PropertyCode)) {
		code_property = static_cast<RKComponentPropertyCode *> (cp);
		connect (code_property, SIGNAL (valueChanged (RKComponentPropertyBase *)), this, SLOT (changedCode (RKComponentPropertyBase *)));
	} else {
		RK_DO (qDebug ("Could not find code property in preview box (remainder: %s)", dummy.latin1()), PLUGIN, DL_WARNING);
		code_property = 0;
	}

	// initialize
	update_timer = new QTimer (this);
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (tryPreviewNow ()));
	updating = false;
	changedState (0);
}

RKPreviewBox::~RKPreviewBox () {
	RK_TRACE (PLUGIN);

	killPreview ();
}

void RKPreviewBox::changedState (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	if (updating) return;
	updating = true;
	toggle_preview_box->setChecked (state->boolValue ());
	updating = false;

	tryPreview ();
	changed ();
}

void RKPreviewBox::changedCode (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	tryPreview ();
}

void RKPreviewBox::changedState (int) {
	RK_TRACE (PLUGIN);

	state->setBoolValue (toggle_preview_box->isChecked ());
}

void RKPreviewBox::tryPreview () {
	RK_TRACE (PLUGIN);

	if (toggle_preview_box->isChecked ()) update_timer->start (10, true);
	else killPreview ();

	updateStatusLabel ();
}

void RKPreviewBox::tryPreviewNow () {
	RK_TRACE (PLUGIN);

	if (!code_property) return;
	if (!parentComponent ()->isSatisfied ()) {
		return;
	}
	if (!parentComponent ()->isReady ()) {
		tryPreview ();
		return;
	}

	if (!last_plot_done) {		// if the last plot is not done, yet, wait before starting the next.
		new_plot_pending = true;
		updateStatusLabel ();
		return;
	}

	preview_active = true;
	QString dummy;
	RKGlobals::rInterface ()->issueCommand (dummy.sprintf (".rk.startPreviewDevice (\"%p\")", this), RCommand::Plugin | RCommand::Sync);
	RKGlobals::rInterface ()->issueCommand ("local({\n" + code_property->preview () + "})\n", RCommand::Plugin | RCommand::Sync, QString::null, this);

	last_plot_done = false;
	new_plot_pending = false;

	updateStatusLabel ();
}

void RKPreviewBox::killPreview () {
	RK_TRACE (PLUGIN);

	if (!preview_active) return;
	preview_active = false;
	QString command;
	RKGlobals::rInterface ()->issueCommand (command.sprintf (".rk.killPreviewDevice (\"%p\")", this), RCommand::Plugin | RCommand::Sync);

	last_plot_done = true;
	new_plot_pending = false;
}

void RKPreviewBox::rCommandDone (RCommand *) {
	RK_TRACE (PLUGIN);

	last_plot_done = true;
	if (new_plot_pending) tryPreview ();

	updateStatusLabel ();
}

void RKPreviewBox::updateStatusLabel () {
	RK_TRACE (PLUGIN);

	if (!toggle_preview_box->isChecked ()) {
		status_label->setText (i18n ("Preview disabled"));
	} else {
		if (parentComponent ()->isSatisfied () && parentComponent ()->isReady ()) {
			if (last_plot_done && (!new_plot_pending)) {
				status_label->setText (i18n ("Preview up to date"));
			} else {
				status_label->setText (i18n ("Preview updating"));
			}
		} else {
			status_label->setText (i18n ("Preview not (yet) possible"));
		}
	}
}

#include "rkpreviewbox.moc"
