/*
rkprintagent - This file is part of RKWard (https://rkward.kde.org). Created: Mon Aug 01 2011
SPDX-FileCopyrightText: 2011-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkprintagent.h"

#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QUrl>

#include <kservice.h>
#include <kmessagebox.h>
#include <kio_version.h>
#if KIO_VERSION >= QT_VERSION_CHECK(5, 77, 0)
#	include <KIO/OpenUrlJob>
#	include <KIO/JobUiDelegate>
#else
#	include <KRun>
#endif
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <kcoreaddons_version.h>

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

void RKPrintAgent::printPostscript (const QString &file, bool delete_file) {
	RK_TRACE (APP)

	KParts::ReadOnlyPart *provider = nullptr;
	KService::Ptr service = KService::serviceByDesktopPath("okular_part.desktop");
	if (!service) service = KService::serviceByDesktopPath("kpdf_part.desktop");
	if (service) {
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5,86,0)
		auto factory = KPluginLoader(service->library()).factory();
		if (factory) {
			provider = factory->create<KParts::ReadOnlyPart>(nullptr);
		}
#else
		provider = KPluginFactory::instantiatePlugin<KParts::ReadOnlyPart>(KPluginMetaData(QPluginLoader(service->library())), nullptr).plugin;
#endif
	} else {
		RK_DEBUG (APP, DL_WARNING, "No KDE service found for postscript printing");
	}

	QAction *printaction = 0;
	if (provider) {
		printaction = provider->action ("print");
		if (!printaction) printaction = provider->action ("file_print");
		if (!printaction) {
			QAction *a = new QAction (provider);
			bool ok = connect (a, SIGNAL (triggered()), provider, SLOT (slotPrint()));
			if (ok) printaction = a;
		}
		if (!(printaction && provider->openUrl (QUrl::fromLocalFile (file)))) {
			RK_DEBUG (APP, DL_WARNING, "No print action in postscript provider");
			delete provider;
			provider = 0;
		}
	}

	if (!provider) {
		RK_DEBUG (APP, DL_WARNING, "No valid postscript provider was found");
		KMessageBox::sorry (RKWardMainWindow::getMain (), i18n ("No service was found to provide a KDE print dialog for PostScript files. We will try to open a generic PostScript viewer (if any), instead.<br><br>Consider installing 'okular', or configure RKWard not to attempt to print using a KDE print dialog."), i18n ("Unable to open KDE print dialog"));
		// fallback: If we can't find a proper part, try to invoke a standalone PS reader, instead
#if KIO_VERSION < QT_VERSION_CHECK(5, 77, 0)
		KRun::runUrl (QUrl::fromLocalFile(file), "application/postscript", RKWardMainWindow::getMain(), KRun::RunFlags());
#else
		auto *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(file), "application/postscript");
		job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, RKWardMainWindow::getMain()));
		job->setDeleteTemporaryFile(delete_file);
		job->start();
#endif
		return;
	}

	RKPrintAgent *agent = new RKPrintAgent(file, provider, delete_file);

	// very hacky heuristic to try to find out, whether the print action is synchronous or asynchronous. If the latter, delete after half an hour. If the former delete after printing.
	QElapsedTimer ts;
	ts.start();
	printaction->trigger();
	if (ts.elapsed() < 5000) {
		QTimer::singleShot (1800000, agent, SLOT (deleteLater()));
	} else {
		agent->deleteLater ();
	}
}
