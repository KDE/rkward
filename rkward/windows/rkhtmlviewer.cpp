/*
rkhtmlviewerwidget - This file is part of the RKWard project. Created: Sat Feb 07 2026
SPDX-FileCopyrightText: 2026 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkhtmlviewer.h"

#include "../settings/rksettingsmodulegeneral.h"
#include "rkhtmlwindow.h"
#include "rkqwebenginewidget.h"
#include "rkqwebview.h"

#include "../debug.h"

RKHTMLViewer::RKHTMLViewer(RKHTMLWindow *parent) : QObject(parent), window(parent) {
	RK_TRACE(APP);
}

RKHTMLViewer *RKHTMLViewer::getNew(RKHTMLWindow *parent) {
	RK_TRACE(APP);
	const auto engine = RKSettingsModuleGeneral::htmlEngine(); // cppcheck-suppress unreadVariable
#if RK_WITH_QWEBENGINE
	if (engine == RKSettingsModuleGeneral::QWebEngineRenderingEngine) {
		return new RKQWebEngineWidget(parent);
	}
#endif
#if RK_WITH_QWEBVIEW
	if (engine == RKSettingsModuleGeneral::QWebViewRenderingEngine) {
		return new RKQWebView(parent);
	}
#endif
	// default to QWebEngine on Linux (where it mostly works), but to QWebView
	// everywhere else (if possible)
#if defined(Q_OS_LINUX) && RK_WITH_QWEBENGINE
	return new RKQWebEngineWidget(parent);
#endif
#if RK_WITH_QWEBVIEW
	return new RKQWebView(parent);
#else
	return new RKQWebEngineWidget(parent);
#endif
}
