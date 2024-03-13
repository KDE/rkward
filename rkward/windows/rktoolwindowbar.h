/*
rktoolwindowbar - This file is part of the RKWard project. Created: Fri Oct 12 2007
SPDX-FileCopyrightText: 2007-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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

Some more would need to be copied for full functionality (session saving / restoring), but for now, I focused on the bare essentials */
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
	void captionChanged(RKMDIWindow* window);
private Q_SLOTS:
	void tabClicked (int id);
	void changeAttachment ();
	void moveToolWindow (int target);
	void addRemoveToolWindow (QAction* action);
	void splitterMoved (int, int);
protected:
	/** handle RMB clicks on individual buttons */
	bool eventFilter (QObject *obj, QEvent *ev) override;
	/** handle RMB clicks on the bar itself */
	void contextMenuEvent (QContextMenuEvent *event) override;
private:
friend class RKWorkplace;
	void reclaimDetached (RKMDIWindow *window);
	void closeOthers (RKMDIWindow *window);
	void windowDestroyed (QObject *window);

	int getSplitterSize () const;
	void setSplitterSize (int new_size);

	QMap<RKMDIWindow*, int> widget_to_id;
	RKMDIWindow* idToWidget (int id) const;

	QSplitter* splitter;
	QWidget* container;

	int last_known_size;
	int id_of_popup;
};

#endif
