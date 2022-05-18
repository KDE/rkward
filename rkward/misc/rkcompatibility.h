/*
rkcommonfunctions - This file is part of the RKWard project. Created: Sat May 14 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKCOMPATIBILTY_H
#define RKCOMPATIBILTY_H

#include <QRect>
#include <QString>
#include <QButtonGroup>

class QWidget;

/** Some helper functions / enums for the sole purpose of working around API changes while keeping compatibility across a wide range of Qt/KF versions.

By their very nature, these functions are not meant to stay, but should be removed, as soon as the incompatibility falls outside the range of supported versions.

@author Thomas Friedrichsmeier
*/
namespace RKCompatibility {
//// NOTE: Functions / constants below are porting aids, to be removed, eventually. ////
/** Small wrapper around QScreen::availableGeometry(), mostly to ease porting */
	QRect availableGeometry(QWidget *for_widget);

/** Porting aid: Qt::SplitBehaviorFlags was added in Qt 5.14, deprecating the previous flags in QString. Remove, once we depend on Qt >= 5.14 */
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
	inline Qt::SplitBehaviorFlags KeepEmptyParts() { return Qt::KeepEmptyParts; };
	inline Qt::SplitBehaviorFlags SkipEmptyParts() { return Qt::SkipEmptyParts; };
#else
	inline QString::SplitBehavior KeepEmptyParts() { return QString::KeepEmptyParts; };
	inline QString::SplitBehavior SkipEmptyParts() { return QString::SkipEmptyParts; };
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
	inline void(QButtonGroup::* groupButtonClicked())(int) { return static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::idClicked); };
#else
	inline void(QButtonGroup::* groupButtonClicked())(int) { return static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked); };
#endif
};

#endif
