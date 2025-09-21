/*
rkcompatibility - This file is part of the RKWard project. Created: Sun Sep 21 2025
SPDX-FileCopyrightText: 2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKCOMPATIBILITY_H
#define RKCOMPATIBILITY_H

#include <QCheckBox>
/**
This namespace holds helpers for supporting different versions of Q6/KF6, without spreading too many #ifdefs all around

@author Thomas Friedrichsmeier
*/
namespace RKCompatibility {
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
inline const constexpr auto QCheckBox_checkStateChanged = &QCheckBox::checkStateChanged;
#else
inline const constexpr auto QCheckBox_checkStateChanged = &QCheckBox::stateChanged;
#endif
}; //namespace RKCompatibility

#endif
