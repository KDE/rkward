/***************************************************************************
                          rkxmlguipreviewarea  -  description
                             -------------------
    begin                : Wed Feb 03 2016
    copyright            : (C) 2016 by Thomas Friedrichsmeier
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
	explicit RKXMLGUIPreviewArea (QWidget* parent);
	~RKXMLGUIPreviewArea ();

	QWidget *menuButton () const;
protected:
	/** build / destroy menu, when child is added removed. Note that we are in the fortunate situation that RKMDIWindow-children only ever get to the
	 *  preview area via reparenting, i.e. contrary to usual QEvent::ChildAdded semnatics, they are always fully constructed, when added. */
	void childEvent (QChildEvent *event) override;
protected slots:
	void prepareMenu ();
private:
	QToolButton *menu_button;
	QMenu *menu;
	QPointer<KParts::Part> current;
};

#endif
