/*
rkcodenavigation - This file is part of the RKWard project. Created: Fri May 23 2025
SPDX-FileCopyrightText: 2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKCODENAVIGATION_H
#define RKCODENAVIGATION_H

#include <KTextEditor/View>

class QAction;
class QActionGroup;
class QMenu;
class RKCodeNavigationInternal;

class RKCodeNavigation : public QObject {
  public:
	RKCodeNavigation(KTextEditor::View *view, QWidget *parent);

	/** get list of (named; suitable for use with KActionCollection) actions for code navigation. */
	QList<QAction *> actions() const { return _actions; };
  private:
	QAction *addAction(QMenu *menu, const QString &name, const QString &label, const QChar command);

	KTextEditor::View *view;
	QList<QAction *> _actions;
	RKCodeNavigationInternal *internal;
	QActionGroup *rmdactions;
};

#endif
