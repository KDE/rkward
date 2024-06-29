/*
rkxmlguipreviewarea - This file is part of the RKWard project. Created: Wed Feb 03 2016
SPDX-FileCopyrightText: 2016-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKXMLGUIPREVIEWAREA_H
#define RKXMLGUIPREVIEWAREA_H

#include <kparts/part.h>

#include <QWidget>
#include <QMenuBar>
#include <QPointer>
#include <QVBoxLayout>

class QMenu;
class QToolButton;
class QLabel;
class RKMDIWindow;
class KXMLGUIBuilder;
class KXMLGUIFactory;
class RKPreviewStatusNote;
class RKPreviewManager;

class RKXMLGUIPreviewArea : public QWidget {
	Q_OBJECT
public:
	RKXMLGUIPreviewArea(const QString &label, QWidget* parent, RKPreviewManager* manager);
	~RKXMLGUIPreviewArea();
	QString label() const;
	void setLabel(const QString &label);
	void setWindow(RKMDIWindow* window);
protected Q_SLOTS:
	void prepareMenu ();
Q_SIGNALS:
	void previewClosed (RKXMLGUIPreviewArea *preview);
private:
	QLabel *lab;
	RKPreviewStatusNote *statusnote;
	QMenu *menu;
	QMenuBar *menubar;
	QPointer<KParts::Part> current;
	KXMLGUIFactory *factory;
	KXMLGUIBuilder *builder;
	QVBoxLayout *internal_layout;
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
	bool needsCommand() const { return !updating && (update_pending == UpdatePending); };
	QString previewId() const { return id; };
	QWidget* inlineStatusWidget();
Q_SIGNALS:
	void statusChanged(RKPreviewManager *);
private Q_SLOTS:
	void previewCommandDone (RCommand *command);
private:
friend class RKPreviewStatusNote;
	void updateStatusDisplay(const QString &warnings=QString());
	enum {
		NoUpdatePending,
		NoUpdatePossible,
		PreviewDisabled,
		UpdatePending
	} update_pending;
	bool updating;
	bool current_preview_failed;
	QString id;
};

#endif
