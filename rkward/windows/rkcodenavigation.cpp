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

class RKCodeNavigationWidget : public QFrame {
  public:
	RKCodeNavigationWidget(KTextEditor::View *view, QWidget *parent) : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint), view(view), doc(view->document()) {
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

		rmdmode = doc->highlightingMode() == u"R Markdown"_s;
		ps = RKParsedScript(doc->text(), rmdmode);
		StoredPosition initial;
		// translate cursor position to string index
		initial.pos = cursorToPosition(view->cursorPosition());
		initial.selection = view->selectionRange();
		stored_positions.append(initial);
		stored_size = size();
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

	int cursorToPosition(const KTextEditor::Cursor &cursor) {
		RK_TRACE(APP);
		int pos = cursor.column();
		for (int l = 0; l < cursor.line(); ++l) {
			pos += doc->lineLength(l) + 1;
		}
		return pos;
	}

	KTextEditor::Cursor positionToCursor(int pos) {
		RK_TRACE(APP);
		for (int l = 0; l < doc->lines(); ++l) {
			pos -= (doc->lineLength(l) + 1);
			if (pos < 0) {
				return KTextEditor::Cursor(l, pos + doc->lineLength(l) + 1);
			}
		}
		return KTextEditor::Cursor();
	}

	struct StoredPosition {
		QString query;
		int pos;
		KTextEditor::Range selection;
		bool hit_top;
		bool hit_bottom;
		QChar command;
	};

	void navigate(const StoredPosition &newpos) {
		RK_TRACE(APP);
		RK_DEBUG(COMMANDEDITOR, DL_DEBUG, "navigate to %d", newpos.pos);
		// translate final position back to cursor coordinates
		if (!newpos.selection.isEmpty()) {
			view->setSelection(newpos.selection);
			view->setCursorPosition(newpos.selection.start());
		} else {
			if (message->isVisible()) {
				message->hide();
				QTimer::singleShot(0, this, [this]() {
					resize(stored_size);
					updatePos();
				});
			}
			view->setCursorPosition(positionToCursor(newpos.pos));
			view->setSelection(KTextEditor::Range(-1, -1, -1, -1));
		}

		updateLabel();
	}

	void handleCommand(const QChar command) {
		RK_TRACE(APP);
		// start from last known position
		auto last = stored_positions.last();
		auto pos = last.pos;

		// apply navigation command
		StoredPosition newpos;
		newpos.pos = pos;
		newpos.query = stored_positions.last().query + command;
		newpos.hit_bottom = false;
		newpos.hit_top = false;
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
				message->setText(i18n("Command 'S' is for R Markdown, only"));
				message->show();
				updatePos();
				return;
			}
			auto posa = ps.getContext(ps.firstContextInChunk(ci)).start;
			auto posb = ps.lastPositionInChunk(ci);
			newpos.selection = KTextEditor::Range(positionToCursor(posa), positionToCursor(posb + 1));
		} else {
			RK_DEBUG(COMMANDEDITOR, DL_WARNING, "unknown navigation commmand");
			message->setText(i18n("Unknown command <tt><b>%1</b></tt>").arg(command));
			message->show();
			updatePos();
			return;
		}

		if (newpos.pos < 0 && command.toLower() != u's') {
			if (rmdmode && newpos.command.toLower() != u'c') {
				if (command.toLower() == command) {
					message->setText(i18nc("Keep this short", "Search hit bottom.<br/><tt><b>c</b></tt> to move to next chunk."));
				} else {
					message->setText(i18nc("Keep this short", "Search hit top.<br/><tt><b>C</b></tt> to move to previous chunk."));
				}
			} else {
				if (command.toLower() == command) {
					message->setText(i18nc("Keep this short", "Search hit bottom.<br/><tt><b>1</b></tt> to move to top."));
				} else {
					message->setText(i18nc("Keep this short", "Search hit top.<br/><tt><b>!</b></tt> to move to bottom."));
				}
			}
			message->show();
			updatePos();
			return;
		}

		stored_positions.append(newpos);
		navigate(newpos);
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

	KTextEditor::View *view;
	KTextEditor::Document *doc;
	KSqueezedTextLabel *input;
	QList<StoredPosition> stored_positions;
	RKParsedScript ps;
	QLabel *message;
	QSize stored_size;
	bool rmdmode;
};

namespace RKCodeNavigation {
	void doNavigation(KTextEditor::View *view, QWidget *parent) {
		auto w = new RKCodeNavigationWidget(view, parent);
		w->show();
		w->updatePos();
		w->input->setFocus();
	}
};
