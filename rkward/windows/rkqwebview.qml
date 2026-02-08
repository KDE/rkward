// rkqwebview - This file is part of the RKWard project. Created: Sun Feb 08 2026
// SPDX-FileCopyrightText: 2026 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
// SPDX-FileContributor: The RKWard Team <rkward@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtWebView
import QtQuick.Layouts

Item {
        id: frame
        visible: true

        WebView {
                id: webView
                objectName: "webView"
                visible: true
                width: parent.width
                height: parent.height
        }
}
