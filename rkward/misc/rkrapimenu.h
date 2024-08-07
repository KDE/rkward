/*
rkrapimenu - This file is part of the RKWard project. Created: Wed Aug 07 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKRAPIMENU_H
#define RKRAPIMENU_H

#include <QVariantList>
#include <KXMLGUIClient>

/** KXMLGUIClient to represent the menu structure defined by rk.menu() from R API */
class RKRApiMenu : public KXMLGUIClient {
public:
	RKRApiMenu();
	~RKRApiMenu() override;
	void updateFromR(const QVariantList &rep);
private:
	void makeXML(QDomDocument &doc, QDomElement e, const QVariantList &l, const QString &path, QStringList *actionlist);
};

#endif
