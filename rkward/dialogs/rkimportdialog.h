/*
rkimportdialog - This file is part of the RKWard project. Created: Tue Jan 30 2007
SPDX-FileCopyrightText: 2007 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKIMPORTDIALOG_H
#define RKIMPORTDIALOG_H

#include <QStringList>
#include <KAssistantDialog>

class RKComponentGUIXML;
class KPageWidgetitem;
class QButtonGroup;
class RKComponentHandle;

/** This dialog is designed to allow the user to select a file, and file format. After that a suitable plugin
is opened automatically to deal with this type of file . */
class RKImportDialog : public KAssistantDialog {
	Q_OBJECT
public:
/** constructor
@param context_id The id of the context containing the relevant plugins
@param parent Parent widget (dialog will be centered on this widget, if non-zero */
	RKImportDialog (const QString &context_id, QWidget *parent);
/** dtor */
	~RKImportDialog ();
	void accept() override;
private:
	void updateState();

	QStringList filters;
	QStringList component_ids;
	RKComponentHandle *rio_handle;
	RKComponentGUIXML *context;
	KPageWidgetItem *select_format;
	QButtonGroup *select_format_group;
	KPageWidgetItem *select_rio;
	QButtonGroup *select_rio_group;
	KPageWidgetItem *select_clipboard;
	QButtonGroup *select_clipboard_group;
	KPageWidgetItem *end_with_selection;
	KPageWidgetItem *end_without_selection;
};

#endif
