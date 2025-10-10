/*
rkradiogroup - This file is part of the RKWard project. Created: Mon Jul 24 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkradiogroup.h"

#include <QSinglePointEvent>

#include "../debug.h"

/** Internal radio button class, which allows to be installed as an event filter on "controlled" widgets.
 *  Using a separate class rather than filtering in RKRadioGroup has the advantage, that the filter will
 *  automatically be removed, in case the button is deleted independent of the controlled widget. */
class RKRadioGroupButton : public QRadioButton {
  public:
	explicit RKRadioGroupButton(const QString &label) : QRadioButton(label) {};
	void control(QWidget *controlled) {
		RKRadioGroupButton::controlled = controlled;
		controlled->installEventFilter(this);
		const auto children = controlled->findChildren<QWidget *>();
		for (auto child : children)
			child->installEventFilter(this); // need to receive clicks on all child widgets!
		controlled->setEnabled(isChecked());
		connect(this, &QAbstractButton::toggled, controlled, &QWidget::setEnabled);
	}

  private:
	bool eventFilter(QObject *obj, QEvent *ev) override {
		if (ev->isSinglePointEvent()) {
			// When clicking in the widget controlled by a button, automatically check the button (enabling the controlled widget).
			// In effect, the appearance is that the widget "belongs" to the button.
			// NOTE: This does not wait for a full click to occur, only a mouse press. The reason is that some widgets will not react on the click,
			//       if they have been disabled during the press.
			auto e = static_cast<QSinglePointEvent *>(ev);
			if (e->isBeginEvent()) {
				// Perhaps a bit paranoid: Only react on controlled and its children
				while (obj) {
					if (obj == controlled) break;
					obj = obj->parent();
				}
				if (obj && isEnabled()) setChecked(true);
			}
		}
		return false;
	}
	QWidget *controlled;
};

RKRadioGroup::RKRadioGroup(QWidget *parent) : RKRadioGroup(QString(), parent) {
}

RKRadioGroup::RKRadioGroup(const QString &label, QWidget *parent) : QGroupBox(label, parent), _group(new QButtonGroup(this)), _layout(new QVBoxLayout(this)) {
	RK_TRACE(MISC);
}

RKRadioGroup::~RKRadioGroup() {
	RK_TRACE(MISC);
}

QRadioButton *RKRadioGroup::addButton(const QString &label, int id) {
	RK_TRACE(MISC);
	auto button = new RKRadioGroupButton(label);
	_group->addButton(button, id);
	_layout->addWidget(button);
	return button;
}

QRadioButton *RKRadioGroup::addButton(const QString &label, int id, QWidget *controlled, QBoxLayout::Direction dir) {
	RK_TRACE(MISC);
	auto old_layout = _layout;
	if (dir != _layout->direction()) {
		_layout = new QBoxLayout(dir);
		old_layout->addLayout(_layout);
	}
	RKRadioGroupButton *button = static_cast<RKRadioGroupButton *>(addButton(label, id));
	_layout->addWidget(controlled);
	_layout = old_layout;
	button->control(controlled);
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
