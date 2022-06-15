/*
core_test - This file is part of RKWard (https://rkward.kde.org). Created: Thu Jun 09 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QObject>
#include <QTest>
#include <QWebEngineView>

/*#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/Document>

#include "../windows/rkhtmlwindow.h"
#include "../rkward.h" */

class LinkTest: public QObject {
	Q_OBJECT
private slots:
	void initTestCase() {
		qDebug("Initializing test case");
	}

	void dummyTest() {
		//new RKWardMainWindow();
		//new RKHTMLWindow(nullptr, RKHTMLWindow::HTMLHelpWindow);
		/*auto editor = KTextEditor::Editor::instance();
		auto doc = editor->createDocument(this);
		auto view = doc->createView(nullptr);*/
		new QWebEngineView();

	}

	void cleanupTestCase() {
	}
};

QTEST_MAIN(LinkTest)

#include "linktest.moc"
