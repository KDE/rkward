/*
rkradiogroup - This file is part of the RKWard project. Created: Mon Jul 24 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkradiogroup.h"

#include <QSinglePointEvent>

#include "../debug.h"

RKRadioGroup::RKRadioGroup(QWidget *parent) : RKRadioGroup(QString(), parent) {
}

RKRadioGroup::RKRadioGroup(const QString &label, QWidget *parent) : QGroupBox(label, parent), _group(new QButtonGroup(this)), _layout(new QVBoxLayout(this)) {
	RK_TRACE(MISC);
}

RKRadioGroup::~RKRadioGroup() {
	RK_TRACE(MISC);
}

QRadioButton* RKRadioGroup::addButton(const QString &label, int id) {
	RK_TRACE(MISC);
	auto button = new QRadioButton(label);
	_group->addButton(button, id);
	_layout->addWidget(button);
	return button;
}

QRadioButton* RKRadioGroup::addButton(const QString &label, int id, QWidget* controlled, QBoxLayout::Direction dir) {
	RK_TRACE(MISC);
	QRadioButton *button;
	auto old_layout = _layout;
	if (dir != _layout->direction()) {
		_layout = new QBoxLayout(dir);
		old_layout->addLayout(_layout);
	}
	button = addButton(label, id);
	_layout->addWidget(controlled);
	controlled->installEventFilter(this);
	controlled->setProperty(property, QVariant::fromValue(button));
	_layout = old_layout;
	controlled->setEnabled(false);
	// TODO (see EditFormatDialog: it may also make sense the other way around: if the associated widget is clicked, set this button as active)
	connect(button, &QAbstractButton::toggled, controlled, &QWidget::setEnabled);
	return button;
}

bool RKRadioGroup::setButtonChecked(int id, bool checked) {
	RK_TRACE(MISC);
	auto button = _group->button(id);
	if (button) {
		button->setChecked(checked);
		return true;
	}
	return false;
}

bool RKRadioGroup::eventFilter(QObject *obj, QEvent *ev) {
	if (ev->isSinglePointEvent()) {
		// When clicking in the widget controlled by a button, automatically check the button (enabling the controlled widget).
		// In effect, the appearance is that the widget "belongs" to the button.
		// NOTE: This does not wait for a full click to occur, only a mouse press. The reason is that some widgets will not react on the click,
		//       if they have been disabled during the press.
		auto e = static_cast<QSinglePointEvent *>(ev);
		auto w = static_cast<QWidget *>(obj);
		if (!w->isEnabled() && e->isBeginEvent()) {
			auto button = w->property(property).value<QRadioButton*>();
			if (button && button->isEnabled()) button->setChecked(true);
		}
	}
	return false;
}
