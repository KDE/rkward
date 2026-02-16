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
                signal pageUrlChanged(url murl, string error, int status)
                signal loadFinished(url murl)
                signal runJSResult(variant result)

                function runJSWrapper(script: string) {
                        runJavaScript(script, runJSResult)
                }
                id: webView
                objectName: "webView"
                visible: true
                width: parent.width
                height: parent.height
                property string acceptedUrl: ""
                property string requestedUrl: ""

                onLoadingChanged: request => {
                        // notify frontend of any url changes, giving it a chance to intervene (e.g. rewriting
                        // rkward:// urls etc.)
                        // Note that the semantics of just when loadingChanged is emitted differ between the QWebView
                        // plugins. E.g. for webengine, we get a LoadStartedStatus, then LoadFailedStatus for rkward://-urls,
                        // while for webview2, we only get LoadFailedStatus.
                        if (request.url != acceptedUrl) {
                                pageUrlChanged(request.url, request.error, request.status);
                                // if request did not get accepted: stay
                                if (requestedUrl != acceptedUrl) {
                                        // TODO: save and restore scroll position
                                        url = acceptedUrl;
                                }
                        }
                        if (request.status == WebView.LoadSucceededStatus) {
                                loadFinished(request.url);
                        }
                }
        }
}
