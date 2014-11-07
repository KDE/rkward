/****************************************************************************
**
** This file is part of a Qt Solutions component.
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** 
** Contact:  Qt Software Information (qt-info@nokia.com)
** 
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** 
****************************************************************************/


// Declaration of the QWinHost classes

#ifndef QWINHOST_H
#define QWINHOST_H

#include <QtGui/QWidget>
#include <QtCore/QMap>
#include <qt_windows.h>

#if defined(Q_WS_WIN)
#  if !defined(QT_QTWINMIGRATE_EXPORT) && !defined(QT_QTWINMIGRATE_IMPORT)
#    define QT_QTWINMIGRATE_EXPORT
#  elif defined(QT_QTWINMIGRATE_IMPORT)
#    if defined(QT_QTWINMIGRATE_EXPORT)
#      undef QT_QTWINMIGRATE_EXPORT
#    endif
#    define QT_QTWINMIGRATE_EXPORT __declspec(dllimport)
#  elif defined(QT_QTWINMIGRATE_EXPORT)
#    undef QT_QTWINMIGRATE_EXPORT
#    define QT_QTWINMIGRATE_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTWINMIGRATE_EXPORT
#endif

class QT_QTWINMIGRATE_EXPORT QWinHost : public QWidget
{
    Q_OBJECT
public:
    QWinHost(QWidget *parent = 0, Qt::WFlags f = 0);
    ~QWinHost();

    void setWindow(HWND);
    HWND window() const;

    bool isAutoDestruct() const;
    void setAutoDestruct(bool);

    static QWinHost *findHost(HWND);

    bool handleWindowCallback (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    QString getClientTitle() const;
public slots:
    void setFocusSlot();
signals:
    void clientDestroyed();
    void clientTitleChanged(const QString& new_title);
    void clientFocused();
protected:
    virtual HWND createWindow(HWND parent, HINSTANCE instance);

    bool event(QEvent *e);
    void showEvent(QShowEvent *);
    void focusInEvent(QFocusEvent*);
    void resizeEvent(QResizeEvent*);

    bool winEvent(MSG *msg, long *result);

private:
    void fixParent();
    friend void* getWindowProc(QWinHost*);

    void *wndproc;
    bool auto_destruct;
    HWND hwnd;

    static QMap<HWND, QWinHost*> winhosts;
};

#endif // QWINHOST_H
