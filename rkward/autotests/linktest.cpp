/*
core_test - This file is part of RKWard (https://rkward.kde.org). Created: Thu Jun 09 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QObject>
#include <QTest>

#include "../windows/rkhtmlwindow.h"
#include "../rkward.h"

void RKDebug (int, int, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}

class LinkTest: public QObject {
	Q_OBJECT
private slots:
	void initTestCase() {
		qDebug("Initializing test case");
	}

	void dummyTest() {
		new RKWardMainWindow();
		new RKHTMLWindow(nullptr, RKHTMLWindow::HTMLHelpWindow);
	}

	void cleanupTestCase() {
	}
};

QTEST_MAIN(LinkTest)

#include "linktest.moc"
