/*
rkcodenavigation - This file is part of the RKWard project. Created: Fri May 23 2025
SPDX-FileCopyrightText: 2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcodenavigation.h"

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

#include "../misc/rkparsedscript.h"
#include "../misc/rkstyle.h"
#include "rkworkplace.h"

#include "../debug.h"

class RKCodeNavigationBase {
  public:
	RKCodeNavigationBase(KTextEditor::View *view) :
		view(view),
		doc(view->document()),
		rmdmode(doc->highlightingMode() == u"R Markdown"_s),
		ps(doc->text(), rmdmode) {
	};

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

	StoredPosition handleCommandBase(const QChar &command) const {
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

	void navigateBase(const StoredPosition &newpos) const {
		RK_TRACE(APP);
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

  protected:
	KTextEditor::View *view;
	KTextEditor::Document *doc;
	bool rmdmode;
	RKParsedScript ps;
};

class RKCodeNavigationWidget : public QFrame, public RKCodeNavigationBase {
  public:
	RKCodeNavigationWidget(KTextEditor::View *view, QWidget *parent) : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint), RKCodeNavigationBase(view) {
		RK_TRACE(APP);

		auto scheme = RKStyle::viewScheme();
		auto pal = palette();
		pal.setColor(backgroundRole(), scheme->background(KColorScheme::PositiveBackground).color());
		setPalette(pal);

		view->installEventFilter(this);
		if (view->window()) view->window()->installEventFilter(this);
		connect(doc, &KTextEditor::Document::textChanged, this, &RKCodeNavigationWidget::deleteLater);

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
		stored_positions.append(viewPosition());
		updateLabel();
	}

	void updatePos() {
		RK_TRACE(APP);
		move(view->mapToGlobal(view->geometry().topRight() - QPoint(width() + 5, -5)));
	}

	void focusOutEvent(QFocusEvent *) override {
		RK_TRACE(APP);
		deleteLater();
	}

	void navigate(const StoredPosition &newpos) {
		RK_TRACE(APP);
		navigateBase(newpos);
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

		auto pos = handleCommandBase(command);
		if (!pos.message.isEmpty()) {
			message->show();
			updatePos();
			return;
		}

		stored_positions.append(pos);
		navigate(pos);
	}

	bool eventFilter(QObject *from, QEvent *event) override {
		RK_TRACE(APP);
		if (from == view && (event->type() == QEvent::Move || event->type() == QEvent::Resize)) {
			updatePos();
		} else if (from == input && event->type() == QEvent::KeyPress) {
			auto ke = static_cast<QKeyEvent *>(event);
			const auto text = ke->text();
			if (ke->modifiers() == Qt::NoModifier && ke->key() == Qt::Key_Backspace && stored_positions.size() > 1) {
				stored_positions.pop_back();
				navigate(stored_positions.last());
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
	QList<StoredPosition> stored_positions;
};

namespace RKCodeNavigation {
	void doNavigation(KTextEditor::View *view, QWidget *parent) {
		auto w = new RKCodeNavigationWidget(view, parent);
		w->show();
		w->updatePos();
		w->input->setFocus();
	}

	void singleShotNavigation(KTextEditor::View *view, const QChar &command) {
		RKCodeNavigationBase w(view);
		w.navigateBase(w.handleCommandBase(command));
	}

	QAction *directNavigationAction(const QString &label, KTextEditor::View *view, const QChar command) {
		QAction *a = new QAction(label);
		QObject::connect(a, &QAction::triggered, view, [view, command]() { singleShotNavigation(view, command); });
		return a;
	}

	QMenu *actionMenu(KTextEditor::View *view, QWidget *parent) {
		QMenu *menu = new QMenu(parent);
		auto action = menu->addAction(i18n("Code Navigation Mode"));
		action->setIcon(QIcon::fromTheme(u"debug-step-into"_s));
		menu->menuAction()->setIcon(action->icon());
		menu->menuAction()->setText(i18n("Code Navigation"));
		QObject::connect(action, &QAction::triggered, parent, [view, parent]() { doNavigation(view, parent); });

		menu->addSeparator();
		QObject::connect(menu, &QMenu::aboutToShow, menu, [view, menu]() {
			bool rmdmode = view->document()->highlightingMode() == u"R Markdown"_s;
			// TODO: Allow setting and customizing shortcuts
			menu->addAction(directNavigationAction(i18n("Next statement"), view, u'n'));
			menu->addAction(directNavigationAction(i18n("Previous statement"), view, u'N'));
			menu->addAction(directNavigationAction(i18n("Next (inner) statement"), view, u'i'));
			menu->addAction(directNavigationAction(i18n("Previous (inner) statement"), view, u'P'));
			menu->addAction(directNavigationAction(i18n("Next outer statement"), view, u'o'));
			menu->addAction(directNavigationAction(i18n("Previous outer statement"), view, u'O'));
			menu->addAction(directNavigationAction(i18n("Next toplevel statement"), view, u't'));
			menu->addAction(directNavigationAction(i18n("Previous toplevel statement"), view, u'T'));
			if (rmdmode) {
				menu->addAction(directNavigationAction(i18n("Next code chunk"), view, u'c'));
				menu->addAction(directNavigationAction(i18n("Previous code chunk"), view, u'C'));
			}
			menu->addSeparator();
			menu->addAction(directNavigationAction(i18n("Select current statement"), view, u's'));
			if (rmdmode) {
				menu->addAction(directNavigationAction(i18n("Select current code chunk"), view, u'S'));
			}
		});

		QObject::connect(menu, &QMenu::aboutToHide, menu, [menu]() {
			const auto acts = menu->actions();
			for (int i = acts.size() - 1; i >= 2; --i) {
				acts[i]->deleteLater();
			}
		});

		return menu;
	}
};
