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
                signal loaded(url murl, string error, int status)
                id: webView
                objectName: "webView"
                visible: true
                width: parent.width
                height: parent.height
                property string acceptedUrl: ""

                onLoadingChanged: request => {
                        // give the frontend a chance to intervene, e.g. rewriting rkward:// url, opening in new window, 
                        // denying external urls, etc.
                        loaded(request.url, request.error, request.status);
                        if (request.status == WebView.LoadStartedStatus) {
                                if (request.url != acceptedUrl) {
                                        //console.log("Navigation request to " + request.url + "denied");
                                        url = acceptedUrl;
                                }
                        }
                }
        }
}
