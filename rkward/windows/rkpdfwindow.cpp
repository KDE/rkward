/*
rkpdfwindow - This file is part of the RKWard project. Created: Tue Jun 18 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkpdfwindow.h"

#include <QVBoxLayout>
#include <QLabel>

#include <KLocalizedString>
#include <KParts/PartLoader>

#include "../misc/rkdummypart.h"

#include "../debug.h"

#define OKULAR_LIBRARY_NAME "kf6/parts/okularpart"

RKPDFWindow::RKPDFWindow(QWidget *parent) : RKMDIWindow(parent, RKMDIWindow::PDFWindow), valid(false) {
	RK_TRACE(APP);

	auto l = new QVBoxLayout(this);
	// Try loading okular part first, then fall back to any other pdf part
	auto pdfpart = getOkularPart();
	if (!pdfpart) {
		pdfpart = KParts::PartLoader::instantiatePartForMimeType<KParts::ReadOnlyPart>("application/pdf").plugin;
	}
	if (pdfpart) {
		setPart(pdfpart);
		valid = true;
	} else {
		setPart(new RKDummyPart(nullptr, new QLabel(i18n("No PDF viewer component found"))));
	}
	l->addWidget(getPart()->widget());
}

RKPDFWindow::~RKPDFWindow() {
	RK_TRACE(APP);
}

void RKPDFWindow::openURL(const QUrl &url) {
	RK_TRACE(APP);
	if (valid) static_cast<KParts::ReadOnlyPart*>(getPart())->openUrl(url);
	setWindowTitle(url.fileName());
}

QUrl RKPDFWindow::url() const {
	if (valid) return static_cast<KParts::ReadOnlyPart*>(getPart())->url();
	return QUrl();
}

KParts::ReadOnlyPart* RKPDFWindow::getOkularPart(const QVariantList &args) {
	RK_TRACE(APP);
	const KPluginMetaData okularPart(QStringLiteral(OKULAR_LIBRARY_NAME));
	auto result = KPluginFactory::instantiatePlugin<KParts::ReadOnlyPart>(okularPart, nullptr, args);
	return result.plugin; // may be nullptr
}
