/*
rkcodenavigation - This file is part of the RKWard project. Created: Fri May 23 2025
SPDX-FileCopyrightText: 2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcodenavigation.h"

#include <QActionGroup>
#include <QApplication>
#include <QFrame>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QVBoxLayout>

#include <KColorScheme>
#include <KLocalizedString>
#include <KSqueezedTextLabel>
#include <KTextEditor/Document>
#include <KTextEditor/Message>

#include "../misc/rkparsedscript.h"
#include "../misc/rkstyle.h"
#include "rkworkplace.h"

#include "../debug.h"

/** TODO: This would benefit from some re-organisation: Wrapper around parsed script and helper functions that may be used
 * with or without ann RKCodeNavigationWidget. Probably, this should own the widget, rather than the other way around. */
class RKCodeNavigationInternal : public QObject {
  public:
	RKCodeNavigationInternal(KTextEditor::View *view, QObject *parent) : QObject(parent),
	                                                                     view(view),
	                                                                     doc(view->document()),
	                                                                     rmdmode(doc->highlightingMode() == u"R Markdown"_s),
	                                                                     ps(doc->text(), rmdmode){};

	struct StoredPosition {
		int pos;
		KTextEditor::Range selection;
		QChar command;
		QString message;
	};

	StoredPosition viewPosition() const {
		StoredPosition pos;
		// translate cursor position to string index
		pos.pos = cursorToPosition(view->cursorPosition());
		pos.selection = view->selectionRange();
		return pos;
	}

	StoredPosition handleCommand(const QChar &command) const {
		auto last = viewPosition();
		auto pos = last.pos;

		// apply navigation command
		StoredPosition newpos;
		newpos.pos = pos;
		newpos.command = command;

		auto ci = ps.contextAtPos(pos);
		if (command == u'n') {
			newpos.pos = ps.getContext(ps.nextStatement(ci)).start;
		} else if (command == u'N') {
			newpos.pos = ps.getContext(ps.prevStatement(ci)).start;
		} else if (command == u'i') {
			newpos.pos = ps.getContext(ps.nextStatementOrInner(ci)).start;
		} else if (command == u'I') {
			newpos.pos = ps.getContext(ps.prevStatementOrInner(ci)).start;
		} else if (command == u'o') {
			newpos.pos = ps.getContext(ps.nextOuter(ci)).start;
		} else if (command == u'O') {
			newpos.pos = ps.getContext(ps.prevOuter(ci)).start;
		} else if (command == u't') {
			newpos.pos = ps.getContext(ps.nextToplevel(ci)).start;
		} else if (command == u'T') {
			newpos.pos = ps.getContext(ps.prevToplevel(ci)).start;
		} else if (command == u'c') {
			newpos.pos = ps.getContext(ps.nextCodeChunk(ci)).start;
		} else if (command == u'C') {
			newpos.pos = ps.getContext(ps.prevCodeChunk(ci)).start;
		} else if (command == u'1') {
			newpos.pos = 0;
		} else if (command == u'!') {
			newpos.pos = cursorToPosition(KTextEditor::Cursor(doc->lines() - 1, doc->lineLength(doc->lines() - 1)));
		} else if (command == u's') {
			auto posa = ps.getContext(ps.firstContextInStatement(ci)).start;
			auto posb = ps.lastPositionInStatement(ci);
			newpos.selection = KTextEditor::Range(positionToCursor(posa), positionToCursor(posb + 1));
		} else if (command == u'S') {
			if (!rmdmode) {
				newpos.message = i18n("Command 'S' is for R Markdown, only");
				return newpos;
			}
			auto posa = ps.getContext(ps.firstContextInChunk(ci)).start;
			auto posb = ps.lastPositionInChunk(ci);
			newpos.selection = KTextEditor::Range(positionToCursor(posa), positionToCursor(posb + 1));
		} else {
			RK_DEBUG(COMMANDEDITOR, DL_WARNING, "unknown navigation commmand");
			newpos.message = i18n("Unknown command <tt><b>%1</b></tt>").arg(command);
			return newpos;
		}

		if (newpos.pos < 0 && command.toLower() != u's') {
			if (rmdmode && newpos.command.toLower() != u'c') {
				if (command.toLower() == command) {
					newpos.message = i18nc("Keep this short", "Search hit bottom.<br/><tt><b>c</b></tt> to move to next chunk.");
				} else {
					newpos.message = i18nc("Keep this short", "Search hit top.<br/><tt><b>C</b></tt> to move to previous chunk.");
				}
			} else {
				if (command.toLower() == command) {
					newpos.message = i18nc("Keep this short", "Search hit bottom.<br/><tt><b>1</b></tt> to move to top.");
				} else {
					newpos.message = i18nc("Keep this short", "Search hit top.<br/><tt><b>!</b></tt> to move to bottom.");
				}
			}
		}

		return newpos;
	}

	void navigate(const StoredPosition &newpos) const {
		RK_TRACE(APP);
		if (!newpos.message.isEmpty()) {
			auto msg = new KTextEditor::Message(newpos.message, KTextEditor::Message::Information);
			msg->setPosition(KTextEditor::Message::BottomInView);
			msg->setAutoHide(2000);
			msg->setAutoHideMode(KTextEditor::Message::Immediate);
			doc->postMessage(msg);
			return;
		}

		RK_DEBUG(COMMANDEDITOR, DL_DEBUG, "navigate to %d", newpos.pos);
		// translate final position back to cursor coordinates
		if (!newpos.selection.isEmpty()) {
			view->setSelection(newpos.selection);
			view->setCursorPosition(newpos.selection.start());
		} else {
			view->setCursorPosition(positionToCursor(newpos.pos));
			view->setSelection(KTextEditor::Range(-1, -1, -1, -1));
		}
	}

	int cursorToPosition(const KTextEditor::Cursor &cursor) const {
		RK_TRACE(APP);
		int pos = cursor.column();
		for (int l = 0; l < cursor.line(); ++l) {
			pos += doc->lineLength(l) + 1;
		}
		return pos;
	}

	KTextEditor::Cursor positionToCursor(int pos) const {
		RK_TRACE(APP);
		for (int l = 0; l < doc->lines(); ++l) {
			pos -= (doc->lineLength(l) + 1);
			if (pos < 0) {
				return KTextEditor::Cursor(l, pos + doc->lineLength(l) + 1);
			}
		}
		return KTextEditor::Cursor();
	}

	KTextEditor::View *view;
	KTextEditor::Document *doc;
	bool rmdmode;
	RKParsedScript ps;
};

class RKCodeNavigationWidget : public QFrame {
  public:
	RKCodeNavigationWidget(QWidget *parent, RKCodeNavigationInternal *internal) : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint), internal(internal) {
		RK_TRACE(APP);

		auto scheme = RKStyle::viewScheme();
		auto pal = palette();
		pal.setColor(backgroundRole(), scheme->background(KColorScheme::PositiveBackground).color());
		setPalette(pal);

		internal->view->installEventFilter(this);
		if (internal->view->window()) internal->view->window()->installEventFilter(this);
		connect(internal->doc, &KTextEditor::Document::textChanged, this, &RKCodeNavigationWidget::deleteLater);

		auto box = new QVBoxLayout(this);
		auto label = new QLabel(i18n("<b>Code Navigation</b> (<a href=\"rkward://page/rkward_code_navigation\">Help</a>)"));
		QObject::connect(label, &QLabel::linkActivated, RKWorkplace::mainWorkplace(), &RKWorkplace::openAnyUrlString);
		box->addWidget(label);

		input = new KSqueezedTextLabel();
		input->setFocusPolicy(Qt::StrongFocus);
		input->installEventFilter(this);
		box->addWidget(input);

		message = new QLabel();
		message->setTextFormat(Qt::RichText);
		message->hide();
		box->addWidget(message);

		stored_size = size();
		stored_positions.append(internal->viewPosition());
		updateLabel();
	}

	void updatePos() {
		RK_TRACE(APP);
		move(internal->view->mapToGlobal(internal->view->geometry().topRight() - QPoint(width() + 5, -5)));
	}

	void focusOutEvent(QFocusEvent *) override {
		RK_TRACE(APP);
		deleteLater();
	}

	void navigate(const RKCodeNavigationInternal::StoredPosition &newpos) {
		RK_TRACE(APP);
		internal->navigate(newpos);
		if (message->isVisible()) {
			message->hide();
			QTimer::singleShot(0, this, [this]() {
				resize(stored_size);
				updatePos();
			});
		}
		updateLabel();
	}

	void handleCommand(const QChar command) {
		RK_TRACE(APP);

		auto pos = internal->handleCommand(command);
		if (!pos.message.isEmpty()) {
			message->setText(pos.message);
			message->show();
			updatePos();
			return;
		}

		stored_positions.append(pos);
		navigate(pos);
	}

	bool eventFilter(QObject *from, QEvent *event) override {
		RK_TRACE(APP);
		if (from == internal->view && (event->type() == QEvent::Move || event->type() == QEvent::Resize)) {
			updatePos();
		} else if (from == input && event->type() == QEvent::KeyPress) {
			auto ke = static_cast<QKeyEvent *>(event);
			const auto text = ke->text();
			if (ke->modifiers() == Qt::NoModifier && ke->key() == Qt::Key_Backspace) {
				if (stored_positions.size() > 1) {
					stored_positions.pop_back();
					navigate(stored_positions.last());
				}
			} else if (ke->modifiers() == Qt::NoModifier && (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)) {
				deleteLater();
			} else if (ke->modifiers() == Qt::NoModifier && (ke->key() == Qt::Key_Escape)) {
				RK_ASSERT(!stored_positions.isEmpty());
				navigate(stored_positions.first());
				deleteLater();
			} else if (!text.simplified().isEmpty()) {
				handleCommand(text.back());
			} else {
				return false;
			}
			return true;
		}
		return false;
	}

	void updateLabel() {
		RK_TRACE(APP);
		if (stored_positions.size() < 2) {
			input->setText(i18n("Enter command"));
		} else {
			QString sequence;
			for (const auto &p : std::as_const(stored_positions)) {
				sequence += p.command;
			}
			input->setText(sequence);
		}
	}

	void paintEvent(QPaintEvent *e) override {
		RK_TRACE(APP);
		QFrame::paintEvent(e);

		QPainter paint(this);
		paint.setPen(QApplication::palette().color(QPalette::Highlight));
		paint.drawRect(0, 0, width() - 1, height() - 1);
	}

	KSqueezedTextLabel *input;
	QLabel *message;
	QSize stored_size;
	QList<RKCodeNavigationInternal::StoredPosition> stored_positions;
	RKCodeNavigationInternal *internal;
};

RKCodeNavigation::RKCodeNavigation(KTextEditor::View *view, QWidget *parent) : QObject(parent), view(view), internal(nullptr) {
	RK_TRACE(APP);

	connect(view->document(), &KTextEditor::Document::textChanged, this, [this]() {
		if (internal) {
			internal->deleteLater();
			internal = nullptr;
		}
	});

	QMenu *menu = new QMenu(parent);
	auto action = menu->addAction(i18n("Quick Code Navigation Mode"));
	action->setIcon(QIcon::fromTheme(u"debug-step-into"_s));
	menu->menuAction()->setWhatsThis(i18n("Step through your code based on its structure or enter <a href=\"rkward://page/rkward_code_navigation\">Quick Code Navigation Mode</a>"));
	menu->menuAction()->setIcon(action->icon());
	menu->menuAction()->setText(i18n("Code Navigation"));
	menu->menuAction()->setObjectName(u"rkcodenav_menu"_s);
	action->setObjectName(u"rkcodenav"_s);
	_actions.append(menu->menuAction());
	_actions.append(action);
	connect(action, &QAction::triggered, parent, [this, parent]() {
		if (!internal) {
			internal = new RKCodeNavigationInternal(this->view, this);
		}
		auto w = new RKCodeNavigationWidget(parent, internal);
		w->show();
		w->updatePos();
		w->input->setFocus();
	});

	rmdactions = new QActionGroup(this);

	menu->addSeparator();

	menu->addSection(i18nc("verb: To jump to a different place in the code", "Go"));
	addAction(menu, u"rkcodenav_next"_s, i18n("Next statement"), u'n');
	addAction(menu, u"rkcodenav_prev"_s, i18n("Previous statement"), u'N');
	addAction(menu, u"rkcodenav_inner"_s, i18n("Next (inner) statement"), u'i');
	addAction(menu, u"rkcodenav_prev_inner"_s, i18n("Previous (inner) statement"), u'P');
	addAction(menu, u"rkcodenav_outer"_s, i18n("Next outer statement"), u'o');
	addAction(menu, u"rkcodenav_prev_outer"_s, i18n("Previous outer statement"), u'O');
	addAction(menu, u"rkcodenav_toplevel"_s, i18n("Next toplevel statement"), u't');
	addAction(menu, u"rkcodenav_prev_toplevel"_s, i18n("Previous toplevel statement"), u'T');

	rmdactions->addAction(addAction(menu, u"rkcodenav_chunk"_s, i18n("Next code chunk"), u'c'));
	rmdactions->addAction(addAction(menu, u"rkcodenav_prev_chunk"_s, i18n("Previous code chunk"), u'C'));
	menu->addSection(i18n("Select"));
	addAction(menu, u"rkcodenav_select"_s, i18n("Select current statement"), u's');
	rmdactions->addAction(addAction(menu, u"rkcodenav_select_chunk"_s, i18n("Select current code chunk"), u'S'));

	QObject::connect(menu, &QMenu::aboutToShow, this, [view, this]() {
		bool rmdmode = view->document()->highlightingMode() == u"R Markdown"_s;
		rmdactions->setVisible(rmdmode);
	});
}

QAction *RKCodeNavigation::addAction(QMenu *menu, const QString &name, const QString &label, const QChar command) {
	QAction *a = new QAction(label);
	a->setObjectName(name);
	QObject::connect(a, &QAction::triggered, view, [this, command]() {
		if (!internal) {
			internal = new RKCodeNavigationInternal(view, this);
		}
		internal->navigate(internal->handleCommand(command));
	});
	menu->addAction(a);
	_actions.append(a);
	return a;
}
