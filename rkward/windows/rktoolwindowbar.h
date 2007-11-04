/***************************************************************************
                          rktoolwindowbar  -  description
                             -------------------
    begin                : Fri Oct 12 2007
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

/* This code is based substantially on kate's katemdi! */

#ifndef RKTOOLWINDOWBAR_H
#define RKTOOLWINDOWBAR_H

#include <kmultitabbar.h>
#include <kconfiggroup.h>

#include <QMap>

class QSplitter;
class QObject;
class QEvent;
class QAction;
class KHBox;
class RKMDIWindow;

/** This class represents one of the bar which tool windows can dock into (top, left, bottom, right). It contains heavy copying from Kate's katemdi SideBar class. I wish this was available as a library, but it isn't, yet.

Some more would need to be copied for full functionality (session saving / restoring), but for now, I focussed on the bare essentials */
class RKToolWindowBar : public KMultiTabBar {
	Q_OBJECT
public:
	RKToolWindowBar (KMultiTabBar::KMultiTabBarPosition position, QWidget *parent);
	~RKToolWindowBar ();

	void setSplitter (QSplitter *splitter);
	void addWidget (RKMDIWindow *widget);
	void removeWidget (RKMDIWindow *widget);

	void showWidget (RKMDIWindow *widget);
	void hideWidget (RKMDIWindow *widget);

	void restoreSize (const KConfigGroup &cg);
	void saveSize (KConfigGroup &cg) const;
private slots:
	void tabClicked (int id);
	void buttonPopupActivate (QAction *a);
protected:
	bool eventFilter (QObject *obj, QEvent *ev);
private:
friend class RKWorkplace;
	void reclaimDetached (RKMDIWindow *window);

	int getSplitterSize () const;
	void setSplitterSize (int new_size);

	QMap<RKMDIWindow*, int> widget_to_id;
	RKMDIWindow* idToWidget (int id) const;

	QSplitter* splitter;
	KHBox* container;

	int initial_size;
	int id_of_popup;
};

#endif
