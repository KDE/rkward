/*
rkprogresscontol - This file is part of the RKWard project. Created: Sun Sep 10 2006
SPDX-FileCopyrightText: 2006-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKPROGRESSCONTROL_H
#define RKPROGRESSCONTROL_H

#include <qobject.h>
#include <QList>
#include <qstring.h>

#include "../rbackend/rcommandreceiver.h"

class QCloseEvent;
class QDialog;
class QLabel;
class QTextEdit;

class RKProgressControlDialog;

/**
This class is a functional combination of the older RKCancelDialog and RKErrorDialog classes. Depending on the selected mode it can be used to allow the user to cancel running actions, to display errors / warning / regular output, only when errors occur or also during normal progress, etc. It provides facilities to get output from an RCommand directly, or you can manually submit output fragments via newOutput and newError.
Note that this class is not a dialog in itself. Rather, a dialog is only created, if / when it is actually needed.
TODO: This and RKwatch should use a common means of displaying the output to achieve a common look and feel.

@author Thomas Friedrichsmeier
*/
class RKProgressControl : public QObject, public RCommandReceiver {
	Q_OBJECT
public:
/** create an RKProgressContol dialog
@param text Text to be shown in the dialog
@param caption caption of the dialog
@param mode_flags a bitwise OR combination of RKProgressControlFlags */
	RKProgressControl (QObject *parent, const QString &text, const QString &caption, int mode_flags);
/** destructor */
	~RKProgressControl ();

/** These flags control the mode of operation. Generally you will use on of the predefined sets (StandardCancel, StandardError, DetailedError, StandardProgress, or CancellableProgress) */
	enum RKProgressControlFlags {
		AllowCancel=1,			 			/**< Show a cancel button. When the cancel button is pressed or the dialog is closed, the signal cancelled () is emitted, and (if invoked that way) doModal returns QDialog::rejected */
		AutoCancelCommands=1024, 	/**< if the user cancels the dialog, automatically cancel all commands previously added via addRCommand () (and not yet finished. Only meaningful if AllowCancel is set as well */
		IncludeErrorOutput=2,			/**< Include erros output in the output shown */
		IncludeRegularOutput=4,		/**< Include regular (no error) output in the output shown */
		RaiseOnError=16,					/**< dialog is shown/raised, when there are errors. Only meaningful, if IncludeErrorOutput is set as well */
		RaiseOnRegularOutput=32,	/**< dialog is also shown/raised, when there is new regular output. Only meaningful, if IncludeRegularOutput is set as well  */
		OutputShownByDefault=64,	/**< the textfield with the output is shown by default, not only when requested by the user. Requires at least one of IncludeErrorOutput or IncludeRegularOutput */
		OutputSwitchable=128,		/**< the textfield with the output can be shown/hidden by the user */
		ShowAtOnce=256,				/**< dialog is shown at once, instead of only when there is an error/output */
		PreventClose=512,				/**< do not accept close events */
		StandardError=IncludeErrorOutput | RaiseOnError | OutputShownByDefault,
		DetailedError=StandardError | IncludeRegularOutput,
		StandardProgress=DetailedError | OutputSwitchable | RaiseOnRegularOutput,
		CancellableProgress=(StandardProgress | AllowCancel | AutoCancelCommands | PreventClose | ShowAtOnce) - (RaiseOnRegularOutput | OutputShownByDefault),
		CancellableNoProgress=CancellableProgress - OutputSwitchable,
		SimpleSplash=(StandardProgress | PreventClose | ShowAtOnce) - (RaiseOnRegularOutput | OutputShownByDefault | OutputSwitchable)
	};

/** show the dialog modal. This will always show the dialog right away
@returns true, if ended by done () or false if it was cancelled / closed */
	bool doModal (bool autodelete);
/** initialize the dialog non modal. The dialog is only shown if needed or set in the constructor flags */
	void doNonModal (bool autodelete);

/** you don't need this, unless you feed regular output to the dialog using newOutput. */
	void resetOutput ();
/** add a command to listen to. Warning: You will always first call addRCommand, then submit the command to RInterface, never the other way around. Else there could be a race condition!
@param done_when_finished If set to true, the done () -slot is auto-called when the given command has completed */
	void addRCommand (RCommand *command, bool done_when_finished=false);
	QString fullCommandOutput ();
signals:
	void cancelled ();
public slots:
/** needed internally so we can easily keep track of whether the dialog is alive or not */
	void dialogDestroyed ();
/** the corresponding action has finished. If there have been no errors, the dialog is also closed. Otherwise, the text of the "cancel" button is changed to "finished". */
	void done ();
	void newError (const QString &error);
/** usually you will call newError instead. However, if in case of an error, you also want to show the regular output, use this function to add output. The output is added to the internal error_log, but the dialog is not shown until you call newError (). */
	void newOutput (const QString &output);
private:
	void createDialog ();

	RKProgressControlDialog *dialog;
	QList<ROutput> output_log;

	RCommand *done_command;

	bool autodelete;
	bool modal;
	bool is_done;
	int mode;
	QString text;
	QString caption;
protected:
	void newOutput (RCommand *, ROutput *output) override;
	void rCommandDone (RCommand *command) override;
};

class KMessageWidget;
class QAction;

class RKInlineProgressControl : public QObject {
public:
/** @param allow_cancel Whether the operation may be cancelled. If true, a cancel button will be added. If false, will also prevent the parent dialog from being closed. Default: true */
	RKInlineProgressControl(QWidget *display_area, bool allow_cancel);
	~RKInlineProgressControl();
	void addRCommand(RCommand *command);
	void setAutoCloseWhenCommandsDone(bool _autoclose) { autoclose = _autoclose; };
	void show(int delay_ms=0);
	KMessageWidget *messageWidget() { return message_widget; };
private:
	void setCloseAction(const QString &label);
	bool eventFilter(QObject *, QEvent *e) override;
	bool autoclose;
	bool allow_cancel;
	QWidget* wrapper;
	QWidget* display_area;
	KMessageWidget *message_widget;
	KMessageWidget *prevent_close_message;
	QTextEdit *output_display;
	QList<RCommand*> unfinished_commands;
	QAction* close_action;
};

#endif
