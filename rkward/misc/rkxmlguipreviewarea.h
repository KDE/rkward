/***************************************************************************
                          rkxmlguipreviewarea  -  description
                             -------------------
    begin                : Wed Feb 03 2016
    copyright            : (C) 2016-2018 by Thomas Friedrichsmeier
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

#ifndef RKXMLGUIPREVIEWAREA_H
#define RKXMLGUIPREVIEWAREA_H

#include <kxmlguiwindow.h>
#include <kparts/part.h>

#include <QPointer>

class QMenu;
class QToolButton;

class RKXMLGUIPreviewArea : public KXmlGuiWindow {
	Q_OBJECT
public:
	RKXMLGUIPreviewArea (const QString &label, QWidget* parent);
	~RKXMLGUIPreviewArea ();
	/** (initializes, and) returns a wrapper widget that contains this widget along with a caption (see setLabel()), menu button, and close button. */
	QWidget *wrapperWidget ();
	QString label () const { return _label; };
protected:
	/** build / destroy menu, when child is added removed. Note that we are in the fortunate situation that RKMDIWindow-children only ever get to the
	 *  preview area via reparenting, i.e. contrary to usual QEvent::ChildAdded semnatics, they are always fully constructed, when added. */
	void childEvent (QChildEvent *event) override;
protected slots:
	void prepareMenu ();
signals:
	void previewClosed (RKXMLGUIPreviewArea *preview);
private:
	QWidget *wrapper_widget;
	QString _label;
	QMenu *menu;
	QPointer<KParts::Part> current;
};

#endif
