/*
rkpreviewbox - This file is part of the RKWard project. Created: Wed Jan 24 2007
SPDX-FileCopyrightText: 2007-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKPREVIEWBOX_H
#define RKPREVIEWBOX_H

#include "rkcomponent.h"

#include "rkcomponentproperties.h"

class QGroupBox;
class QDomElement;
class QLabel;
class QTimer;
class RKPreviewManager;

/**
This RKComponent provides a (togglable) automatic graphical preview. WARNING: This component violates some standards of "good component behavior", esp. by assuming several things about the nature of the parent component. So please do not take this as an example for basing other components on.

@author Thomas Friedrichsmeier
*/
class RKPreviewBox : public RKComponent {
	Q_OBJECT
public: 
	RKPreviewBox (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKPreviewBox ();
	int type () override { return ComponentPreviewBox; };
	RKComponentPropertyBool *state;
	QVariant value (const QString &modifier=QString ()) override;
public Q_SLOTS:
	void changedStateFromUi ();
	void changedState (RKComponentPropertyBase *);
	void changedCode (RKComponentPropertyBase *);
	void tryPreviewNow ();
private:
	bool updating;		// prevent recursion
	bool preview_active;
	void tryPreview ();
	void killPreview (bool cleanup = false);
	RKPreviewManager *manager;
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
	QGroupBox *toggle_preview_box;
	RKComponentPropertyCode *code_property;
	QString idprop;
	QString placement_command;
	QString placement_end;
};

#endif
