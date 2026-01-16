/*
rkradiogroup - This file is part of the RKWard project. Created: Mon Jul 24 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKRADIOGROUP_H
#define RKRADIOGROUP_H

#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

/** This class makes it easier to create a group of radio buttons in a standard layout */
class RKRadioGroup : public QGroupBox {
	Q_OBJECT
  public:
	explicit RKRadioGroup(QWidget *parent = nullptr);
	explicit RKRadioGroup(const QString &label, QWidget *parent = nullptr);
	~RKRadioGroup() override;

	QRadioButton *addButton(const QString &label, int id = -1);
	QRadioButton *addButton(const QString &label, const QVariant &data);
	/** NOTE: the group, but not the associated button assumes ownership over controlled! */
	QRadioButton *addButton(const QString &label, int id, QWidget *controlled, QBoxLayout::Direction dir = QBoxLayout::TopToBottom);
	QRadioButton *addButton(const QString &label, QWidget *controlled, QBoxLayout::Direction dir = QBoxLayout::TopToBottom) {
		return addButton(label, -1, controlled, dir);
	}
	/** Find the button with the given id, and set it as checked.
	 *  @return false, if there is no such button, true otherwise. */
	bool setButtonChecked(int id, bool checked);
	QButtonGroup *group() const { return _group; };
	const QVariant checkedData() const;
	/** Find the button with the given data, and set it as checked.
	 *  @see checkedData();
	 *  @return false, if there is no such button, true otherwise. */
	bool setButtonChecked(const QVariant &data, bool checked);

  private:
	QButtonGroup *_group;
	QBoxLayout *_layout;
};

#endif
