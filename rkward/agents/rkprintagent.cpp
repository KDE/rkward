/*
rkprintagent - This file is part of RKWard (https://rkward.kde.org). Created: Mon Aug 01 2011
SPDX-FileCopyrightText: 2011-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkprintagent.h"

#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QUrl>

#include <KMessageBox>
#include <KIO/OpenUrlJob>
#include <KIO/JobUiDelegate>
#include <KIO/JobUiDelegateFactory>
#include <KLocalizedString>
#include <KPluginFactory>
#include <kcoreaddons_version.h>

#include "../windows/rkpdfwindow.h"
#include "../rkward.h"

#include "../debug.h"

RKPrintAgent::RKPrintAgent(const QString &file, KParts::ReadOnlyPart *provider, bool delete_file) : QObject(), file(file), provider(provider), delete_file(delete_file) {
	RK_TRACE (APP);
	//provider->widget()->show(); // not very helpful as a preview, unfortunately
}

RKPrintAgent::~RKPrintAgent () {
	RK_TRACE (APP)

	if (delete_file) QFile (file).remove ();
	if (provider) provider->deleteLater ();
}

void fallbackToGeneric(const QString &file, bool delete_file) {
	KMessageBox::error(RKWardMainWindow::getMain(), i18n("No service was found to provide a KDE print dialog for PostScript files. We will try to open a generic PostScript viewer (if any), instead.<br><br>Consider installing 'okular', or configure RKWard not to attempt to print using a KDE print dialog."), i18n("Unable to open KDE print dialog"));

	// fallback: If we can't find a proper part, try to invoke a standalone PS reader, instead
	auto *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(file), QStringLiteral("application/postscript"));
	job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, RKWardMainWindow::getMain()));
	job->setDeleteTemporaryFile(delete_file);
	job->start();
}

void RKPrintAgent::printPostscript (const QString &file, bool delete_file) {
	RK_TRACE (APP)

	auto provider = RKPDFWindow::getOkularPart({"ViewerWidget"});
	if(!provider) {
		RK_DEBUG(APP, DL_WARNING, "No valid postscript provider was found");
		fallbackToGeneric(file, delete_file);
		return;
	}

	QAction *printaction = provider->action("print");
	if (!printaction) printaction = provider->action("file_print");
	if (!printaction) {
		QAction *a = new QAction(provider);
		bool ok = connect(a, SIGNAL(triggered()), provider, SLOT(slotPrint()));
		if (ok) printaction = a;
	}
	if (!(printaction && provider->openUrl(QUrl::fromLocalFile(file)))) {
		if (!printaction) {
			RK_DEBUG(APP, DL_WARNING, "No print action in postscript provider");
		} else {
			RK_DEBUG(APP, DL_WARNING, "Failure to open file %s in postscript provider", qPrintable(file));
		}
		delete provider;
		provider = nullptr;
		fallbackToGeneric(file, delete_file);
		return;
	}

	RKPrintAgent *agent = new RKPrintAgent(file, provider, delete_file);

	// very hacky heuristic to try to find out, whether the print action is synchronous or asynchronous. If the latter, delete after half an hour. If the former delete after printing.
	QElapsedTimer ts;
	ts.start();
	printaction->trigger();
	if (ts.elapsed() < 5000) {
		QTimer::singleShot(1800000, agent, &QObject::deleteLater);
	} else {
		agent->deleteLater ();
	}
}
