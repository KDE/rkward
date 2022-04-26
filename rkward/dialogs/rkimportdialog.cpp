/*
rkimportdialog - This file is part of RKWard (https://rkward.kde.org). Created: Tue Jan 30 2007
SPDX-FileCopyrightText: 2007-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkimportdialog.h"

#include <KLocalizedString>

#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>

#include "../plugin/rkcomponentmap.h"
#include "../plugin/rkcomponentcontext.h"
#include "../misc/rkcommonfunctions.h"

#include "../debug.h"

RKImportDialog::RKImportDialog(const QString &context_id, QWidget *parent) : KAssistantDialog(parent) {
	RK_TRACE (DIALOGS);

	context = RKComponentMap::getContext(context_id);
	if (context) {
		component_ids = context->components();
	}

	for (auto it = component_ids.constBegin(); it != component_ids.constEnd(); ++it) {
		RKComponentHandle *handle = RKComponentMap::getComponentHandle(*it);
		if (!handle) {
			RK_ASSERT (false);
			continue;
		}

		QString filter = handle->getAttributeValue("format");
		QString label = handle->getAttributeLabel("format");

		QString elabel = label;
		elabel.replace ('(', "[");
		elabel.replace (')', "]");
		filters.append (elabel + " [" + filter + "] (" + filter + ')');
	}

	QWidget *page1 = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(page1);
	layout->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("For certain formats, RKWard provides specialized import dialogs, and those generally provide the most options. Is the file you wish to import in one of the following formats?")));
	QGroupBox *box = new QGroupBox();
	layout->addWidget(box);
	select_format_group = new QButtonGroup(this);
	QVBoxLayout *sublayout = new QVBoxLayout(box);
	for (int i = 0; i < filters.size(); ++i) {
		auto *button = new QRadioButton(filters[i]);
		sublayout->addWidget(button);
		select_format_group->addButton(button);
	}
	auto *button = new QRadioButton("None of the above / try another method");
	sublayout->addWidget(button);
	select_format_group->addButton(button);
	button->setChecked(true);
	select_format = addPage(page1, i18n("Select format"));
}

RKImportDialog::~RKImportDialog() {
	RK_TRACE(DIALOGS);
}

/*
void RKImportDialog::accept () {
	RK_TRACE (DIALOGS);

	int index = filters.indexOf (selectedNameFilter ());
	QString cid = component_ids.value (index);
	RKComponentHandle *handle = RKComponentMap::getComponentHandle (cid);
	RKContextHandler *chandler = context->makeContextHandler (this, false);

	if (!(handle && chandler)) {
		RK_ASSERT (false);
	} else {
		RKComponentPropertyBase *filename = new RKComponentPropertyBase (chandler, false);
		filename->setValue (selectedFiles ().value (0));
		chandler->addChild ("filename", filename);

		chandler->invokeComponent (handle);
	}
}
*/
