/*
rkprogresscontol - This file is part of RKWard (https://rkward.kde.org). Created: Sun Sep 10 2006
SPDX-FileCopyrightText: 2006-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkprogresscontrol.h"

#include <QHBoxLayout>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>

#include <KLocalizedString>
#include <KColorScheme>

#include "../rbackend/rkrinterface.h"
#include "../settings/rksettingsmoduler.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstyle.h"

#include "../debug.h"

/** This class provides the dialog shown as part of an RKProgressControl. Generally you should not use this class directly, but rather use RKProgressControl. */
class RKProgressControlDialog : public QDialog {
public:
/** constructor. */
	RKProgressControlDialog (const QString &text, const QString &caption, int mode_flags, bool modal);
/** destructor. */
	~RKProgressControlDialog ();
public:
	void addOutput (const ROutput *output);
	void finished ();
protected:
	void closeEvent (QCloseEvent *e) override;
	void scrollDown ();
	void toggleDetails ();
private:
/** Replace "Cancel" button text with "Close" (to be called, when underlying command has finished */
	void setCloseTextToClose ();

	QLabel *error_indicator;
	QTextEdit *output_text;
	QWidget *detailsbox;
	QDialogButtonBox *buttons;

	QString output_button_text;

	ROutput::ROutputType last_output_type;
	bool prevent_close;
	bool is_done;
};


RKProgressControl::RKProgressControl (QObject *parent, const QString &text, const QString &caption, int mode_flags) : QObject (parent) {
	RK_TRACE (MISC);

	RKProgressControl::text = text;
	RKProgressControl::caption = caption;
	RKProgressControl::mode = mode_flags;

	dialog = nullptr;
	is_done = false;
	modal = false;
	autodelete = false;
}

RKProgressControl::~RKProgressControl () {
	RK_TRACE (MISC);

	if (!is_done) done ();
}

void RKProgressControl::autoDeleteWhenDone(){
	RK_TRACE (MISC);
	autodelete = true;
	if (outstanding_commands.isEmpty()) deleteLater();
}

bool RKProgressControl::doModal (bool autodelete) {
	RK_TRACE (MISC);
	RK_ASSERT (!dialog);

	modal = true;
	createDialog ();

	int res = dialog->exec ();
	RK_ASSERT ((res == QDialog::Accepted) || (res == QDialog::Rejected));

	bool ret = is_done;		// for good style, we copy this onto the stack, as we are about to self destruct
	if (autodelete) deleteLater ();

	return ret;
}

void RKProgressControl::doNonModal (bool autodelete) {
	RK_TRACE (MISC);
	RK_ASSERT (!dialog);

	RKProgressControl::autodelete = autodelete;
	if ((!dialog) && (mode & ShowAtOnce)) {		// actually, dialog should always be 0 at this point
		createDialog ();
		dialog->show ();
	}
}

void RKProgressControl::newError (const QString &error) {
	RK_TRACE (MISC);

	if (!(mode & IncludeErrorOutput)) return;

	ROutput outputc;
	outputc.type = ROutput::Error;
	outputc.output = error;

	output_log.append (outputc);

	if (mode & RaiseOnError) {
		if (!dialog) createDialog ();
		dialog->raise ();
	}
	if (dialog) dialog->addOutput (&outputc);
}

void RKProgressControl::newOutput (const QString &output) {
	RK_TRACE (MISC);

	if (!(mode & IncludeRegularOutput)) return;

	ROutput outputc;
	outputc.type = ROutput::Output;
	outputc.output = output;

	output_log.append (outputc);

	if (mode & RaiseOnRegularOutput) {
		if (!dialog) createDialog ();
		dialog->raise ();
	}
	if (dialog) dialog->addOutput (&outputc);
}

void RKProgressControl::addRCommand (RCommand *command, bool done_when_finished) {
	RK_TRACE (MISC);
	RK_ASSERT (command);

	outstanding_commands.append(command);
	connect(command->notifier(), &RCommandNotifier::commandOutput, this, QOverload<RCommand*, const ROutput*>::of(&RKProgressControl::newOutput));
	if (done_when_finished) {
		command->whenFinished(this, [this](RCommand* command) {
			outstanding_commands.removeAll(command);
			done();
		});
	}
}

void RKProgressControl::dialogDestroyed () {
	RK_TRACE (MISC);

	dialog = nullptr;
	if ((!is_done) && (mode & AllowCancel)) {
		is_done = true;
		if (mode & AutoCancelCommands) {
			for (auto it = outstanding_commands.cbegin (); it != outstanding_commands.cend (); ++it) {
				RInterface::instance()->cancelCommand(*it);
			}
		}
		Q_EMIT cancelled();
	}
}

void RKProgressControl::done () {
	RK_TRACE (MISC);

	is_done = true;
	if (dialog) {
		dialog->finished ();
	}

	if ((!modal) && autodelete) {
		if (dialog) disconnect (dialog, &QObject::destroyed, this, &RKProgressControl::dialogDestroyed);		// we're already dead
		deleteLater ();
	}
}

void RKProgressControl::createDialog () {
	RK_TRACE (MISC);

	dialog = new RKProgressControlDialog (text, caption, mode, modal);
	connect (dialog, &QObject::destroyed, this, &RKProgressControl::dialogDestroyed);
	if (is_done) done ();
	for (int i = 0; i < output_log.count (); ++i) {
		dialog->addOutput (&(output_log[i]));
	}
}

void RKProgressControl::newOutput (RCommand *, const ROutput *output) {
	RK_TRACE (MISC);
	RK_ASSERT (output);

	if (output->type == ROutput::Output) {
		newOutput (output->output);
	} else {
		newError (output->output);
	}
}

QString RKProgressControl::fullCommandOutput() {
	RK_TRACE (MISC);

	QString ret;
	for (const ROutput& out : std::as_const(output_log)) ret.append (out.output);
	return ret;
}

//////////////////////////// RKProgressControlDialog ///////////////////////////////////////////7

#include <qlayout.h>
#include <QTextEdit>
#include <qlabel.h>
#include <QPushButton>

#include <kstandardguiitem.h>

RKProgressControlDialog::RKProgressControlDialog(const QString &text, const QString &caption, int mode_flags, bool modal) : QDialog(nullptr) {
	RK_TRACE (MISC);

	setAttribute (Qt::WA_DeleteOnClose, true);
	setModal (modal);
	setWindowTitle (caption);

	QVBoxLayout *layout = new QVBoxLayout (this);

	QWidget *mainbox = new QWidget (this);
	QVBoxLayout *mainboxlayout = new QVBoxLayout (mainbox);
	mainbox->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	mainboxlayout->setContentsMargins (0, 0, 0, 0);

	mainboxlayout->addWidget (RKCommonFunctions::linkedWrappedLabel (text));

	error_indicator = new QLabel (i18n ("<b>There have been errors and / or warnings. See below for a transcript</b>"), mainbox);
	QPalette palette = error_indicator->palette ();
	palette.setColor (error_indicator->foregroundRole (), QColor (255, 0, 0));
	error_indicator->setPalette (palette);
	error_indicator->hide ();
	mainboxlayout->addWidget (error_indicator);

	detailsbox = new QWidget (this);
	QVBoxLayout *detailsboxlayout = new QVBoxLayout (detailsbox);
	detailsboxlayout->setContentsMargins (0, 0, 0, 0);

	QString ocaption;
	if (mode_flags & RKProgressControl::IncludeRegularOutput) {
		output_button_text = i18n ("Output");
		ocaption = i18n ("Output:");
	} else {
		output_button_text = i18n ("Errors / Warnings");
		ocaption = i18n ("Errors / Warnings:");
	}
	QLabel* label = new QLabel (ocaption, detailsbox);
	detailsboxlayout->addWidget (label);

	output_text = new QTextEdit (detailsbox);
	output_text->setReadOnly (true);
	output_text->setPlainText (QString ());
	output_text->setUndoRedoEnabled (false);
	output_text->setLineWrapMode (QTextEdit::NoWrap);
	output_text->setMinimumWidth (QFontMetrics (output_text->font ()).averageCharWidth () * RKSettingsModuleR::getDefaultWidth ());
	detailsboxlayout->addWidget (output_text);
	detailsboxlayout->setStretchFactor (output_text, 10);

	buttons = new QDialogButtonBox (QDialogButtonBox::Cancel, this);
	connect (buttons->button (QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);
	if (mode_flags & RKProgressControl::OutputSwitchable) {
		QPushButton* button = buttons->addButton (output_button_text, QDialogButtonBox::HelpRole);
		connect (button, &QPushButton::clicked, this, &RKProgressControlDialog::toggleDetails);
	}
	if (mode_flags & RKProgressControl::AllowCancel) buttons->button (QDialogButtonBox::Cancel)->setText (i18n ("Cancel"));
	else (setCloseTextToClose ());

	detailsbox->setVisible (mode_flags & RKProgressControl::OutputShownByDefault);

	layout->addWidget (mainbox);
	layout->addWidget (detailsbox);
	layout->addWidget (buttons);

	prevent_close = (mode_flags & RKProgressControl::PreventClose);

	last_output_type = ROutput::Output;
	is_done = false;
}

RKProgressControlDialog::~RKProgressControlDialog () {
	RK_TRACE (MISC);
}

void RKProgressControlDialog::addOutput (const ROutput *output) {
	RK_TRACE (MISC);

	// scrolled all the way to the bottom?
	bool at_end = true;
	if (detailsbox->isVisible ()) {
		QScrollBar *bar = output_text->verticalScrollBar ();
		if (bar && (bar->value () < bar->maximum ())) at_end = false;
	}

	if (output->type != last_output_type) {
		last_output_type = output->type;
		output_text->insertPlainText ("\n");

		if (output->type == ROutput::Output) {
			output_text->setTextColor(RKStyle::viewScheme()->foreground(KColorScheme::NormalText).color());
		} else {
			output_text->setTextColor(RKStyle::viewScheme()->foreground(KColorScheme::NegativeText).color());
			if (!detailsbox->isVisible ()) toggleDetails ();
			error_indicator->show ();
		}
	}

	output_text->insertPlainText (output->output);

	// if previously at end, auto-scroll
	if (at_end && output_text->isVisible ()) scrollDown ();
}

void RKProgressControlDialog::toggleDetails () {
	RK_TRACE (MISC);

	int new_height = height ();
	if (detailsbox->isVisible ()) {
		new_height -= detailsbox->height ();
		detailsbox->hide ();
	} else {
		int h = detailsbox->height ();
		if (h <= 0) h = detailsbox->sizeHint ().height ();
		new_height += h;
		detailsbox->show ();
		scrollDown ();
	}

	layout ()->activate ();
	resize (width (), new_height);
}

void RKProgressControlDialog::scrollDown () {
	RK_TRACE (MISC);

	QScrollBar *bar = output_text->verticalScrollBar ();
	if (bar) bar->setValue (bar->maximum ());
}

void RKProgressControlDialog::setCloseTextToClose () {
	RK_TRACE (MISC);

	buttons->removeButton (buttons->button (QDialogButtonBox::Cancel));
	QPushButton *done_button = buttons->addButton (QDialogButtonBox::Ok);
	done_button->setText (i18n ("Done"));
	connect (done_button, &QPushButton::clicked, this, &QDialog::reject);
}

void RKProgressControlDialog::finished () {
	RK_TRACE (MISC);

	is_done = true;
	setCloseTextToClose ();
	if (!detailsbox->isVisible ()) reject ();
}

void RKProgressControlDialog::closeEvent (QCloseEvent *e) {
	RK_TRACE (DIALOGS);

	if (prevent_close && (!is_done)) {
		e->ignore ();
	} else {
		QDialog::closeEvent (e);
	}
}

#include <KMessageWidget>
#include <KMessageBox>
#include <KStandardAction>

#include "../windows/rkmdiwindow.h"
#include "rkstandardicons.h"

RKInlineProgressControl::RKInlineProgressControl(QWidget *display_area, bool allow_cancel) : QObject(display_area),
						autoclose(false),
						allow_cancel(allow_cancel),
						is_done(false),
						any_failed(false),
						display_area(display_area),
						prevent_close_message(nullptr),
						close_action(nullptr) {
	RK_TRACE(MISC);

	QWidget* logical_parent = display_area;
	while(logical_parent->parentWidget() && !(logical_parent->isWindow() || qobject_cast<RKMDIWindow*>(logical_parent))) {
		logical_parent = logical_parent->parentWidget();
	}
	logical_parent->installEventFilter(this);
	//display_area->installEventFilter(this);

	wrapper = new QWidget(display_area);
	wrapper->setAutoFillBackground(true);
	auto layout = new QVBoxLayout(wrapper);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	message_widget = new KMessageWidget();
	message_widget->setPosition(KMessageWidget::Position::Header);
	message_widget->setWordWrap(true);
	message_widget->setCloseButtonVisible(false);  // we want a button, instead, for consistency with cancel
	if (allow_cancel) {
		setCloseAction(i18n("Cancel"));
	}
	output_display = new QTextEdit();
	output_display->setProperty("_breeze_force_frame", false);
	layout->addWidget(message_widget);
	layout->addWidget(output_display);
	wrapper->resize(display_area->size());
}

RKInlineProgressControl::~RKInlineProgressControl() {
	RK_TRACE(MISC);

	delete wrapper;
}

void RKInlineProgressControl::setCloseAction(const QString &label) {
	RK_TRACE(MISC);

	if (!close_action) {
		close_action = KStandardAction::create(KStandardAction::Close, this, &RKInlineProgressControl::cancelAndClose, this);
		message_widget->addAction(close_action);
	}
	close_action->setText(label);
}

void RKInlineProgressControl::cancelAndClose() {
	RK_TRACE(MISC);

	is_done = true;
	for (int i = 0; i < unfinished_commands.size(); ++i) {
		RInterface::instance()->cancelCommand(unfinished_commands[i]);
	}
	deleteLater();
}

void RKInlineProgressControl::addRCommand(RCommand *command) {
	RK_TRACE(MISC);
	unfinished_commands.append(command);
	connect(command->notifier(), &RCommandNotifier::commandFinished, this, [this](RCommand *c) {
		unfinished_commands.removeAll(c);
		if (c->failed()) {
			any_failed = true;
		}
		if (unfinished_commands.isEmpty()) {
			done();
		}
	});
	connect(command->notifier(), &RCommandNotifier:: commandOutput, this, [this](RCommand *, const ROutput *o) {
		addOutput(o->output, o->type != ROutput::Output);
	});
}

void RKInlineProgressControl::addOutput(const QString& output, bool is_error_warning) {
	RK_TRACE(MISC);
	if (is_error_warning) {
		output_display->setTextColor(RKStyle::viewScheme()->foreground(KColorScheme::NegativeText).color());
	} else {
		output_display->setTextColor(RKStyle::viewScheme()->foreground(KColorScheme::NormalText).color());
	}
	output_display->insertPlainText(output);
}

void RKInlineProgressControl::done() {
	RK_TRACE(MISC);
	if (autoclose && !any_failed) {
		deleteLater();
	} else {
		message_widget->setMessageType(any_failed ? KMessageWidget::Error : KMessageWidget::Positive);
		message_widget->setText(text + ' ' + (any_failed ? i18n("<b>An error occurred</b> (see below for details)") : i18n("<b>Done</b>")));
		message_widget->setIcon(QIcon::fromTheme(any_failed ? "emblem-error" : "emblem-success"));
		message_widget->animatedShow(); // to force an update of geometry
		setCloseAction(i18n("Close"));
	}
	is_done = true;
}

void RKInlineProgressControl::setText(const QString& _text){
	RK_TRACE(MISC);
	text = _text;
	message_widget->setText(text);
}

void RKInlineProgressControl::show(int delay_ms) {
	RK_TRACE(MISC);
	if (delay_ms > 0) {
		QTimer::singleShot(delay_ms, wrapper, &QWidget::show);
	} else {
		wrapper->show();
	}
	RKStandardIcons::busyAnimation(this, [this](const QIcon &icon) {
		if (!is_done) message_widget->setIcon(icon);
	});
}

bool RKInlineProgressControl::eventFilter(QObject *w, QEvent *e) {
	RK_TRACE(MISC);
	Q_UNUSED(w); // we only installed filter on one object (logical_parent)
	if (e->type() == QEvent::Resize || e->type() == QEvent::Show) {
		QTimer::singleShot(0, this, [this]() {
			wrapper->resize(display_area->size());
		});
		return false;
	}
	if ((e->type() == QEvent::Close) && !is_done) {
		if (allow_cancel) {
			bool autoclose_save = autoclose;  // must prevent self-destruction while dialog below is active (the operation might complete, while it exec's)
			autoclose = false;

			bool ignore = (KMessageBox::warningContinueCancel(display_area, i18n("Closing this window will cancel the current operation. Are you sure?"), i18n("Cancel operation"), KGuiItem(i18n("Keep waiting")), KGuiItem(i18n("Cancel && Close"))) == KMessageBox::Continue);

			autoclose = autoclose_save;
			if (ignore) {
				e->ignore();
				return true;
			}
			cancelAndClose();
			return false;
		}
		// TODO
		if (!prevent_close_message) {
			prevent_close_message = new KMessageWidget(i18n("An operation is still running, please wait."), display_area->window());
		}
		QSize s = prevent_close_message->sizeHint();
		prevent_close_message->resize(display_area->window()->width(), s.height());
		prevent_close_message->setMessageType(KMessageWidget::Error);
		prevent_close_message->animatedShow();
		QTimer::singleShot(5000, prevent_close_message, &KMessageWidget::animatedHide);
		e->ignore();
		return true;
	}
	return false;
}


