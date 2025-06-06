/*
rkrapimenu - This file is part of the RKWard project. Created: Wed Aug 07 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKRAPIMENU_H
#define RKRAPIMENU_H

#include <KXMLGUIClient>
#include <QVariantList>

/** KXMLGUIClient to represent the menu structure defined by rk.menu() from R API */
class RKRApiMenu : public KXMLGUIClient {
  public:
	RKRApiMenu();
	~RKRApiMenu() override;
	void updateFromR(const QVariantList &rep);
	void enableAction(const QStringList &path, bool enable, bool show);
	/** mostly for testing: Lookup action by path */
	QAction *actionByPath(const QStringList &path);

  private:
	void makeXML(QDomElement e, const QVariantList &l, const QString &path, QStringList *actionlist);
	void makeAction(QDomElement e, const QString &id, const QString &label, QStringList *actionlist);
	QVariantList rep;
	void commit();
};

#endif
