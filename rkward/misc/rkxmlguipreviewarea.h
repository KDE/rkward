/*
rkxmlguipreviewarea - This file is part of the RKWard project. Created: Wed Feb 03 2016
SPDX-FileCopyrightText: 2016-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKXMLGUIPREVIEWAREA_H
#define RKXMLGUIPREVIEWAREA_H

#include <kxmlguiwindow.h>
#include <kparts/part.h>

#include <QPointer>

class QMenu;
class QToolButton;
class QLabel;
class RKMDIWindow;

class RKXMLGUIPreviewArea : public KXmlGuiWindow {
	Q_OBJECT
public:
	RKXMLGUIPreviewArea (const QString &label, QWidget* parent);
	~RKXMLGUIPreviewArea ();
	/** Returns a wrapper widget (created on first call of this function) that contains this widget along with a caption (see setLabel()), menu button, and close button. */
	QWidget *wrapperWidget ();
	QString label () const { return _label; };
	void setLabel (const QString &label);
	void setWindow(RKMDIWindow* window);
protected slots:
	void prepareMenu ();
signals:
	void previewClosed (RKXMLGUIPreviewArea *preview);
private:
	QWidget *wrapper_widget;
	QString _label;
	QLabel *lab;
	QMenu *menu;
	QPointer<KParts::Part> current;
};

class RCommand;
/** Simple manager (state machine) for previews. Keeps track of whether a preview is currently updating / up-to-date, and provides
 *  status information to any preview window. */
class RKPreviewManager : public QObject {
	Q_OBJECT
public:
	explicit RKPreviewManager (QObject *parent);
	~RKPreviewManager ();

	void setUpdatePending ();
	void setPreviewDisabled ();
	void setNoPreviewAvailable ();
	/** Start the next preview update, as given by command. You must call needsCommand() first, to check whether the next command is
	 *  ready to go. */
	void setCommand (RCommand *command);
	bool needsCommand () const { return !updating && (update_pending == UpdatePending); };
	QString previewId () const { return id; };
	QString shortStatusLabel () const;
signals:
	void statusChanged ();
private slots:
	void previewCommandDone (RCommand *command);
private:
	void setStatusMessage (const QString &);
	enum {
		NoUpdatePending,
		NoUpdatePossible,
		PreviewDisabled,
		UpdatePending
	} update_pending;
	bool updating;
	QString id;
};

#endif
