/*
rkmdiwindow - This file is part of RKWard (https://rkward.kde.org). Created: Tue Sep 26 2006
SPDX-FileCopyrightText: 2006-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkmdiwindow.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qtimer.h>
#include <QEvent>
#include <QPaintEvent>
#include <QAction>
#include <QVBoxLayout>

#include <kparts/partactivateevent.h>
#include <kparts/readwritepart.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <KLocalizedString>
#include <kmessagewidget.h>
#include <kwidgetsaddons_version.h>

#include "rkworkplace.h"
#include "rkworkplaceview.h"
#include "rktoolwindowbar.h"
#include "rktoolwindowlist.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkxmlguisyncer.h"
#include "../rbackend/rcommand.h"

#include "../debug.h"

RKMDIStandardActionClient::RKMDIStandardActionClient () : KXMLGUIClient () {
	RK_TRACE (APP);

	setComponentName (QCoreApplication::applicationName (), QGuiApplication::applicationDisplayName ());
	setXMLFile ("rkstandardactions.rc", true);
}

RKMDIStandardActionClient::~RKMDIStandardActionClient () {
	RK_TRACE (APP);
}


// TODO: remove name parameter
RKMDIWindow::RKMDIWindow (QWidget *parent, int type, bool tool_window, const char *) : QFrame (parent) {
	RK_TRACE (APP);

	if (tool_window) {
		type |= ToolWindow;
	} else {
		type |= DocumentWindow;
	}
	RKMDIWindow::type = type;
	state = Attached;
	tool_window_bar = nullptr;
	part = nullptr;
	active = false;
	no_border_when_active = false;
	standard_client = nullptr;
	status_popup = nullptr;
	status_popup_container = nullptr;
	ui_buddy = nullptr;
	file_save_action = nullptr;
	file_save_as_action = nullptr;
	connect(&status_message_timer, &QTimer::timeout, this, &RKMDIWindow::showStatusMessageNow);
	status_message_timer.setSingleShot(true);

	if (!(type & KatePluginWindow)) setWindowIcon (RKStandardIcons::iconForWindow (this));
}

RKMDIWindow::~RKMDIWindow () {
	RK_TRACE (APP);

	if (isToolWindow ()) RKToolWindowList::unregisterToolWindow (this);
	delete standard_client;
}

KActionCollection *RKMDIWindow::standardActionCollection () {
	if (!standard_client) {
		RK_TRACE (APP);
		standard_client = new RKMDIStandardActionClient ();
		RK_ASSERT (part);	// call setPart () first!
		part->insertChildClient (standard_client);
	}
	return standard_client->actionCollection ();
}

//virtual
QString RKMDIWindow::fullCaption () {
	RK_TRACE (APP);
	return shortCaption ();
}

//virtual
QString RKMDIWindow::shortCaption () {
	RK_TRACE (APP);
	return windowTitle ();
}

void RKMDIWindow::setCaption(const QString &caption) {
	RK_TRACE(APP);
	QWidget::setWindowTitle(caption);
	Q_EMIT captionChanged(this);
	if (tool_window_bar) tool_window_bar->captionChanged(this);
}

bool RKMDIWindow::isActive () {
	// don't trace, called pretty often

	if (!topLevelWidget ()->isActiveWindow ()) return false;
	return isActiveInsideToplevelWindow ();
}

bool RKMDIWindow::isActiveInsideToplevelWindow () {
	// don't trace, called pretty often
	return (active || (!isAttached ()));
}

void RKMDIWindow::activate (bool with_focus) {
	RK_TRACE (APP);

	QPointer<QWidget> old_focus = qApp->focusWidget ();

	if (isToolWindow ()) {
		if (tool_window_bar) tool_window_bar->showWidget (this);
		else if (!isVisible ()) RKWorkplace::mainWorkplace ()->detachWindow (this, true);
		else {
			topLevelWidget ()->show ();
			topLevelWidget ()->raise ();
		}
	} else {
		if (isAttached ()) RKWorkplace::mainWorkplace ()->view ()->showWindow (this);
		else {
			topLevelWidget ()->show ();
			topLevelWidget ()->raise ();
		}
	}

	Q_EMIT windowActivated(this);
	if (with_focus) {
		if (old_focus) old_focus->clearFocus ();
		topLevelWidget ()->activateWindow ();
		setFocus();
	} else {
		if (old_focus) {
			old_focus->setFocus ();
			active = false;
		}
	}
}

bool RKMDIWindow::close (CloseWindowMode ask_save) {
	RK_TRACE (APP);

	if (isToolWindow ()) {
		if (!isAttached ()) {
			topLevelWidget ()->deleteLater ();
			// flee the dying DetachedWindowContainer
			if (tool_window_bar) RKWorkplace::mainWorkplace ()->attachWindow (this);
			else {
				state = Attached;
				hide();
				setParent(nullptr);
			}
		}

		if (tool_window_bar) tool_window_bar->hideWidget (this);
		else hide ();

		return true;
	}

	bool ok_to_close = (ask_save == NoAskSaveModified) || QWidget::close ();
	if (!ok_to_close) return false;

	// WORKAROUND for https://bugs.kde.org/show_bug.cgi?id=170806
	// NOTE: can't move this to the d'tor, since the part is already partially deleted, then
	// TODO: use version check / remove once fixed in kdelibs
	if (part && part->factory ()) {
		part->factory ()->removeClient (part);
	}
	// WORKAROUND end

	delete this;	// Note: using deleteLater(), here does not work well while restoring workplaces (window is not fully removed from workplace before restoring)

	return true;
}

void RKMDIWindow::prepareToBeAttached () {
	RK_TRACE (APP);
}

void RKMDIWindow::prepareToBeDetached () {
	RK_TRACE (APP);

	if (isToolWindow ()) {
		if (tool_window_bar) tool_window_bar->hideWidget (this);
	}
}

bool RKMDIWindow::eventFilter (QObject *watched, QEvent *e) {
	// WARNING: The derived object and the part may both the destroyed at this point of time!
	// Make sure not to call any virtual function on this object!
	RK_ASSERT (acceptsEventsFor (watched));

	if (watched == getPart ()) {
		if (KParts::PartActivateEvent::test (e)) {
			RK_TRACE(APP);           // trace only the "interesting" calls to this function

			KParts::PartActivateEvent *ev = static_cast<KParts::PartActivateEvent *>(e);
			if (ev->activated()) {
				Q_EMIT windowActivated(this);
				setFocus();      // focus doesn't always work correctly for the kate part
				active = true;
			} else {
				active = false;
			}
			if (layout()->contentsMargins ().top() < 1) {
				layout()->setContentsMargins (1, 1, 1, 1);
			}
			update ();
		}
	}
	return false;
}

bool RKMDIWindow::acceptsEventsFor (QObject *object) {
	// called very often. Don't trace

	if (object == getPart ()) return true;
	return false;
}

void RKMDIWindow::initializeActivationSignals () {
	RK_TRACE (APP);

	RK_ASSERT (getPart ());
	getPart ()->installEventFilter (this);

	RKXMLGUISyncer::self ()->watchXMLGUIClientUIrc (getPart ());
}

void RKMDIWindow::paintEvent (QPaintEvent *e) {
	// RK_TRACE (APP); Do not trace!

	QFrame::paintEvent (e);

	if (isActive () && !no_border_when_active) {
		QPainter paint (this);
		paint.setPen (QApplication::palette ().color(QPalette::Highlight));
		paint.drawRect (0, 0, width ()-1, height ()-1);
	}
}

void RKMDIWindow::changeEvent (QEvent *event) {
	RK_TRACE (APP);

	if (event->type () == QEvent::ActivationChange) {
		// NOTE: active is NOT the same as isActive(). Active just means that this window *would* be active, if its toplevel window is active.
		if (active || (!isAttached ())) update ();
	}
	QFrame::changeEvent (event);
}

void RKMDIWindow::slotActivateForFocusFollowsMouse () {
	RK_TRACE (APP);

	if (!underMouse ()) return;

	// we can't do without activateWindow(), below. Unfortunately, this has the side effect of raising the window (in some cases). This is not always what we want, e.g. if a 
	// plot window is stacked above this window. (And since this is activation by mouse hover, this window is already visible, by definition!)
	// So we try a heuristic (imperfect) to find, if there are any other windows stacked above this one, in order to re-raise them above this.
	QWidgetList toplevels = qApp->topLevelWidgets ();
	QWidgetList overlappers;
	QWidget *window = topLevelWidget ();
	QRect rect = window->frameGeometry ();
	for (int i = toplevels.size () - 1; i >= 0; --i) {
		QWidget *tl = toplevels[i];
		if (!tl->isWindow ()) continue;
		if (tl == window) continue;
		if (tl->isHidden ()) continue;

		QRect tlrect = tl->geometry ();
		QRect intersected = tlrect.intersected (rect);
		if (!intersected.isEmpty ()) {
			QWidget *above = qApp->topLevelAt ((intersected.left () +intersected.right ()) / 2, (intersected.top () +intersected.bottom ()) / 2);
			if (above && (above != window) && (above->isWindow ()) && (!above->isHidden ()) && (overlappers.indexOf (above) < 0)) overlappers.append (above);
		}
	}

	activate (true);

	for (int i = 0; i < overlappers.size (); ++i) {
		overlappers[i]->raise ();
	}
}

void RKMDIWindow::enterEvent (QEnterEvent *event) {
	RK_TRACE (APP);

	if (!isActive ()) {
		if (RKSettingsModuleGeneral::mdiFocusPolicy () == RKSettingsModuleGeneral::RKMDIFocusFollowsMouse) {
			if (!QApplication::activePopupWidget ()) {
				// see http://sourceforge.net/p/rkward/bugs/90/
				// enter events may be delivered while a popup-menu (in a different window) is executing. If we activate in this case, the popup-menu might get deleted
				// while still handling events.
				//
				// Similar problems seem to occur, when the popup menu has just finished (by the user selecting an action) and this results
				// in the mouse entering this widget. To prevent crashes in this second case, we delay the activation until the next iteration of the event loop.
				//
				// Finally, in some cases (such as when a new script window was created), we need a short delay, as we may be catching an enter event on a window that is in the same place,
				// where the newly created window goes. This would cause activation to switch back, immediately.
				QTimer::singleShot(50, this, [this](){ slotActivateForFocusFollowsMouse(); });
			}
		}
	}

	QFrame::enterEvent (event);
}

void RKMDIWindow::showStatusMessageNow() {
	RK_TRACE (MISC);

	if (!status_popup) {
		// NOTE: For plots, against the recommendation, we want the status message as an overlay to the main widget.
		//       This is because changing the plot area geometry will trigger redraws of the plot.
		//       Note that these messages are mostly used on previews, so far, where they will either be a) transient ("preview updating"),
		//       or b) in case of errors, the place of interest will be outside the preview widget _and_ the preview will generally be invalid.
		status_popup_container = new QWidget (this);
		if (!isType(RKMDIWindow::X11Window)) {
			auto blayout = qobject_cast<QVBoxLayout*> (layout());
			if (blayout) {
				blayout->insertWidget(0, status_popup_container);
				blayout->setStretch(1, 2); // main widget should get most space
			}
		}
		QVBoxLayout *layout = new QVBoxLayout (status_popup_container);
		layout->setContentsMargins (10, 10, 10, 10);
		status_popup = new KMessageWidget (status_popup_container);
		status_popup->setCloseButtonVisible (true);
		status_popup->setMessageType (KMessageWidget::Warning);
		layout->addWidget (status_popup);
		layout->addStretch ();

		// when animation is finished, squeeze the popup-container, so as not to interfere with mouse events in the main window
		connect (status_popup, &KMessageWidget::showAnimationFinished, this, [this]() { status_popup_container->resize (QSize(width(), status_popup->height () + 20)); });
		connect (status_popup, &KMessageWidget::hideAnimationFinished, status_popup_container, &QWidget::hide);
	}

	if (!status_message.isEmpty ()) {
		status_popup_container->resize (size ());
		status_popup_container->setMaximumWidth(width()); // also in resizeEvent(). Without this, extra long messages could force the whole preview window to resize, taking up all horziontal space.
		status_popup_container->setMinimumWidth(width()/2);
		status_popup_container->show ();
		if (status_popup->text () == status_message) {
			if (!status_popup->isVisible ()) status_popup->animatedShow ();  // it might have been closed by user. And no, simply show() is _not_ good enough. KF5 (5.15.0)
		}
		if (status_popup->text () != status_message) {
			if (status_popup->isVisible ()) status_popup->hide (); // otherwise, the KMessageWidget does not update geometry (KF5, 5.15.0)
			status_popup->setText (status_message);
			status_popup->animatedShow ();
		}
	} else {
		status_popup_container->hide ();
		status_popup->hide ();
		status_popup->setText (QString ());
	}
}

void RKMDIWindow::setStatusMessage(const QString& message, RCommand *command) {
	RK_TRACE (MISC);

	if (command) connect(command->notifier(), &RCommandNotifier::commandFinished, this, &RKMDIWindow::clearStatusMessage);
	status_message = message;
	if (message.isEmpty()) {
		status_message_timer.stop();
		showStatusMessageNow();
	} else {
		// delay the actual show a bit. Often it's just a very brief "preview updating", that will just look like an annoying flicker
		status_message_timer.start(500);
	}
}

void RKMDIWindow::clearStatusMessage () {
	RK_TRACE (APP);

	setStatusMessage(QString());
}

void RKMDIWindow::resizeEvent (QResizeEvent*) {
	if (status_popup_container && status_popup_container->isVisible ()) {
		status_popup_container->setMaximumWidth(width());
		status_popup_container->setMinimumWidth(width()/2);
		status_popup_container->resize(QSize(width(), status_popup->height () + 20));
	}
}


void RKMDIWindow::setWindowStyleHint (const QString& hint) {
	RK_TRACE (APP);

	if (hint == "preview") {
		if (standard_client) {
			QAction *act = standardActionCollection ()->action ("window_help");
			if (act) act->setVisible (false);
			act = standardActionCollection ()->action ("window_configure");
			if (act) act->setVisible (false);
		}
		no_border_when_active = true;
	}
}

void RKMDIWindow::setMetaInfo (const QString& _generic_window_name, const QUrl& _help_url, RKSettings::SettingsPage _settings_page) {
	RK_TRACE (APP);

	// only meant to be called once
	RK_ASSERT (generic_window_name.isEmpty() && help_url.isEmpty ());
	generic_window_name = _generic_window_name;
	help_url = _help_url;
	settings_page = _settings_page;

	if (!help_url.isEmpty ()) {
		QAction *action = standardActionCollection()->addAction("window_help", this, &RKMDIWindow::showWindowHelp);
		action->setText (i18n ("Help on %1", generic_window_name));
	}
	if (settings_page != RKSettings::NoPage) {
		QAction *action = standardActionCollection()->addAction("window_configure", this, &RKMDIWindow::showWindowSettings);
		action->setText (i18n ("Configure %1", generic_window_name));
	}
}

void RKMDIWindow::showWindowHelp () {
	RK_TRACE (APP);

	RK_ASSERT (!help_url.isEmpty ());
	RKWorkplace::mainWorkplace()->openHelpWindow (help_url, true);
}

void RKMDIWindow::showWindowSettings () {
	RK_TRACE (APP);

	RK_ASSERT (settings_page != RKSettings::NoPage);
	RKSettings::configureSettings (settings_page, this);
}

void RKMDIWindow::addUiBuddy(KXMLGUIClient* buddy) {
	RK_TRACE(APP);
	RK_ASSERT(!ui_buddy);
	ui_buddy = buddy;
}

