/*
rkhtmlviewerwidget - This file is part of the RKWard project. Created: Sat Feb 07 2026
SPDX-FileCopyrightText: 2026 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkhtmlviewer.h"

#include "rkqwebenginewidget.h"
#include "rkqwebview.h"

#include "../debug.h"

RKHTMLViewer::RKHTMLViewer(QObject *parent) : QObject(parent) {
	RK_TRACE(APP);
}

RKHTMLViewer *RKHTMLViewer::getNew(RKHTMLWindow *parent) {
	RK_TRACE(APP);
	//return new RKQWebEngineWidget(parent);
	return new RKQWebView(parent);
}

#include "rkhtmlviewer.moc"
