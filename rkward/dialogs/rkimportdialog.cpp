/*
rkimportdialog - This file is part of RKWard (https://rkward.kde.org). Created: Tue Jan 30 2007
SPDX-FileCopyrightText: 2007-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkimportdialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>

#include <KLocalizedString>
#include <KMessageWidget>

#include "../plugin/rkcomponentmap.h"
#include "../plugin/rkcomponentcontext.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkspecialactions.h"

#include "../debug.h"

RKImportDialog::RKImportDialog(const QString &context_id, QWidget *parent) : KAssistantDialog(parent) {
	RK_TRACE (DIALOGS);

	setWindowTitle(i18n("Import Data Assistant"));
	rio_handle = RKComponentMap::getComponentHandle("rkward::import_generic_rio");
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

	QWidget *page = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(page);
	layout->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("For certain formats, RKWard provides specialized import dialogs, and those generally provide the most options. Is the file you wish to import in one of the following formats?")));
	if (filters.isEmpty()) {
		auto w = new KMessageWidget(i18n("RKWard comes with several import dialogs, but none seem to be loaded, at present. Check your settings."));
		w->setMessageType(KMessageWidget::Warning);
		w->setWordWrap(true);
		layout->addWidget(w);
	}
	QGroupBox *box = new QGroupBox();
	layout->addWidget(box);
	select_format_group = new QButtonGroup(this);
	QVBoxLayout *sublayout = new QVBoxLayout(box);
	for (int i = 0; i < filters.size(); ++i) {
		auto *button = new QRadioButton(filters[i]);
		sublayout->addWidget(button);
		select_format_group->addButton(button, i);
	}
	auto *button = new QRadioButton(i18n("None of the above / try another method"));
	sublayout->addWidget(button);
	select_format_group->addButton(button);
	button->setChecked(true);
	connect(select_format_group, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, &RKImportDialog::updateState);
	layout->addStretch();
	select_format = addPage(page, i18n("Select format"));

	page = new QWidget();
	layout = new QVBoxLayout(page);
	layout->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("The 'rio' package offers generic support for importing many different file formats, but requires a number of additional R packages to be installed (you will be prompted for missing packages). Do you want to give that a try?")));
	if (!rio_handle) {
		auto w = new KMessageWidget(i18n("The generic import plugin (shipped with RKWard) is not presently loaded. Check your settings."));
		w->setMessageType(KMessageWidget::Warning);
		w->setWordWrap(true);
		layout->addWidget(w);
	}
	box = new QGroupBox();
	layout->addWidget(box);
	select_rio_group = new QButtonGroup(this);
	sublayout = new QVBoxLayout(box);
	button = new QRadioButton(i18n("Use 'rio'-based importer"));
	button->setEnabled(rio_handle != nullptr);
	sublayout->addWidget(button);
	select_rio_group->addButton(button, 0);
	button = new QRadioButton(i18n("Try another method"));
	sublayout->addWidget(button);
	select_rio_group->addButton(button);
	button->setChecked(true);
	connect(select_rio_group, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, &RKImportDialog::updateState);
	layout->addStretch();
	select_rio = addPage(page, i18n("Generic 'rio'-based importer"));

	page = new QWidget();
	layout = new QVBoxLayout(page);
	layout->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("If your data is moderate in size, and you can open/view it in another application on your system, importing it from clipboard (copy and paste) may be viable.")));
	box = new QGroupBox();
	layout->addWidget(box);
	select_clipboard_group = new QButtonGroup(this);
	sublayout = new QVBoxLayout(box);
	button = new QRadioButton(i18n("Import from clipboard"));
	sublayout->addWidget(button);
	select_clipboard_group->addButton(button, 0);
	button = new QRadioButton(i18n("Try another method"));
	sublayout->addWidget(button);
	select_clipboard_group->addButton(button);
	button->setChecked(true);
	connect(select_clipboard_group, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, &RKImportDialog::updateState);
	layout->addStretch();
	select_clipboard = addPage(page, i18n("Import from clipboard"));

	page = new QWidget();
	layout = new QVBoxLayout(page);
	layout->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("<b>Ready to go</b><p>Click \"Finish\" to start the selected import dialog, or \"Back\" to explore other options.</p>")));
	layout->addStretch();
	end_with_selection = addPage(page, i18n("Start import dialog"));

	page = new QWidget();
	layout = new QVBoxLayout(page);
	layout->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("No further import methods are available at this time. Things you can try, include: <ul><li>Check for the availability of additional import plugins</li><li>Save your data to a different format in the original application</li><li>Ask for advice on rkward-users@kde.org</li></ul>")));
	layout->addStretch();
	end_without_selection = addPage(page, i18n("No import method found"));

	updateState();
}

RKImportDialog::~RKImportDialog() {
	RK_TRACE(DIALOGS);
}

void RKImportDialog::updateState() {
	RK_TRACE(DIALOGS);
	bool do_format = (select_format_group->checkedId() >= 0);
	bool do_rio = !do_format && (select_rio_group->checkedId() == 0);
	bool do_clipboard = !(do_format || do_rio) && (select_clipboard_group->checkedId() == 0);

	setAppropriate(select_format, true);
	setAppropriate(select_rio, !do_format);
	setAppropriate(select_clipboard, !(do_format || do_rio));
	setAppropriate(end_with_selection, do_format || do_rio || do_clipboard);
	setAppropriate(end_without_selection, !isAppropriate(end_with_selection));
}

void RKImportDialog::accept () {
	RK_TRACE (DIALOGS);

	hide();
	int index = select_format_group->checkedId();
	if (index >= 0) {
		RKComponentMap::invokeComponent(component_ids.value(index), QStringList());
	} else if (rio_handle && (select_rio_group->checkedId() == 0)) {
		rio_handle->invoke(nullptr, nullptr);
	} else if (select_clipboard_group->checkedId() == 0) {
		RKPasteSpecialDialog dia(this, true);
		dia.exec();
	}
	deleteLater();
	KAssistantDialog::accept();
}
