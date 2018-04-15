/***************************************************************************
                          rkpreviewbox  -  description
                             -------------------
    begin                : Wed Jan 24 2007
    copyright            : (C) 2007-2016 by Thomas Friedrichsmeier
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

#ifndef RKPREVIEWBOX_H
#define RKPREVIEWBOX_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

#include "../rbackend/rcommandreceiver.h"

class QCheckBox;
class QDomElement;
class QLabel;
class QTimer;

/**
This RKComponent provides a (togglable) automatic graphical preview. WARNING: This component violates some standards of "good component behavior", esp. by assuming several things about the nature of the parent component. So please do not take this as an example for basing other components on.

@author Thomas Friedrichsmeier
*/
class RKPreviewBox : public RKComponent, public RCommandReceiver {
	Q_OBJECT
public: 
	RKPreviewBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKPreviewBox ();
	int type () { return ComponentPreviewBox; };
	RKComponentPropertyBool *state;
	QVariant value (const QString &modifier=QString ()) override;
public slots:
	void changedStateFromUi ();
	void changedState (RKComponentPropertyBase *);
	void changedCode (RKComponentPropertyBase *);
	void tryPreviewNow ();
protected:
	void rCommandDone (RCommand *);
private:
	bool updating;		// prevent recursion
	bool preview_active;
	bool prior_preview_done;
	bool new_preview_pending;
	void tryPreview ();
	void killPreview (bool cleanup = false);
	void updateStatusLabel ();
	void setStatusMessage (const QString& status);
	enum PreviewMode {
		PlotPreview,
		DataPreview,
		OutputPreview,
		CustomPreview
	} preview_mode;
	enum PreviewPlacement {
		DefaultPreview,
		AttachedPreview,
		DetachedPreview,
		DockedPreview
	} placement;
	QTimer *update_timer;
	QCheckBox *toggle_preview_box;
	QLabel *status_label;
	RKComponentPropertyCode *code_property;
	QString idprop;
	QString placement_command;
	QString placement_end;
};

#endif
