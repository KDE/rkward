/*
rkpdfwindow - This file is part of the RKWard project. Created: Tue Jun 18 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKPDFWINDOW_H
#define RKPDFWINDOW_H

#include "rkmdiwindow.h"

#include <KParts/ReadOnlyPart>

class RKPDFWindow : public RKMDIWindow {
	Q_OBJECT
public:
	explicit RKPDFWindow(QWidget *parent);
	~RKPDFWindow() override;
	void openURL(const QUrl &url);
	QUrl url() const;
	static KParts::ReadOnlyPart* getOkularPart(const QVariantList &args=QVariantList());
private:
	bool valid;
};

#endif
