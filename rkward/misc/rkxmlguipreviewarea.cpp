/*
rkxmlguipreviewarea - This file is part of RKWard (https://rkward.kde.org). Created: Wed Feb 03 2016
SPDX-FileCopyrightText: 2016-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkxmlguipreviewarea.h"

#include <QMenu>
#include <QToolButton>
#include <QEvent>
#include <QMenuBar>
#include <QWidgetAction>
#include <QLabel>
#include <QVBoxLayout>
#include <QDomElement>
#include <QPainter>
#include <QPen>

#include <KXMLGUIFactory>
#include <KXMLGUIBuilder>
#include <KToolBar>
#include <KLocalizedString>
#include <KColorScheme>
#include <KMessageWidget>

#include "../windows/rkmdiwindow.h"
#include "../windows/rkworkplace.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rkrinterface.h"
#include "rkstandardicons.h"

#include "../debug.h"

class RKXMLGUIPreviewBuilder : public KXMLGUIBuilder {
public:
	RKXMLGUIPreviewBuilder(QWidget* parent, QMenuBar* menubar) : KXMLGUIBuilder(parent), menubar(menubar) {};
	QStringList containerTags() const override {
		QStringList ret;
		ret << QStringLiteral("menubar") << QStringLiteral("menu");
		return ret;
	}
	QWidget* createContainer(QWidget *parent, int index, const QDomElement &element, QAction *&containerAction) override {
		QString tagname = element.tagName().toLower();
		if (tagname == QStringLiteral("menubar")) {
			return menubar;
		}
		// only menus will actually be built, as we effectively disabled toolbars via containerTags(), above
		return KXMLGUIBuilder::createContainer(parent, index, element, containerAction);
	}
	void removeContainer(QWidget *container, QWidget *parent, QDomElement &element, QAction *containerAction) override {
		if (container == menubar) {
			menubar->clear();
			return; // do not delete it
		}
		KXMLGUIBuilder::removeContainer(container, parent, element, containerAction);
	}
private:
	QMenuBar *menubar;
};

RKXMLGUIPreviewArea::RKXMLGUIPreviewArea(const QString &label, QWidget* parent, RKPreviewManager *manager) : QWidget (parent) {
	RK_TRACE (PLUGIN);

	current = nullptr;
	menubar = nullptr;
	builder = nullptr;
	factory = nullptr;

	QVBoxLayout *vl = new QVBoxLayout(this);
	vl->setContentsMargins(0, 0, 0, 0);
	QFrame *line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	vl->addWidget(line);
	QHBoxLayout *hl = new QHBoxLayout();
	vl->addLayout(hl);
	lab = new QLabel(label);
	QFont fnt(lab->font());
	fnt.setBold(true);
	lab->setFont(fnt);
	lab->setAlignment(Qt::AlignCenter);
	QToolButton *tb = new QToolButton();
	tb->setAutoRaise(true);
	tb->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionClose));
	connect(tb, &QAbstractButton::clicked, this, [this]() { hide(); Q_EMIT previewClosed(this); });

	QToolButton *menu_button = new QToolButton(this);
	menu_button->setPopupMode(QToolButton::InstantPopup);
	menu_button->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionShowMenu));
	menu_button->setMenu(menu = new QMenu(this));
	connect(menu, &QMenu::aboutToShow, this, &RKXMLGUIPreviewArea::prepareMenu);

	hl->addWidget(menu_button);
	hl->addStretch();
	hl->addWidget(lab);
	if (manager) hl->addWidget(manager->inlineStatusWidget());
	hl->addStretch();
	hl->addWidget(tb);
	internal_layout = new QVBoxLayout();
	vl->addLayout(internal_layout);

	menubar = new QMenuBar(nullptr); // it is important that the menubar never is a child of the main window, not even indirectly! https://bugs.kde.org/show_bug.cgi?id=416911
	builder = new RKXMLGUIPreviewBuilder(this, menubar);
	factory = new KXMLGUIFactory(builder, this);
}

RKXMLGUIPreviewArea::~RKXMLGUIPreviewArea () {
	RK_TRACE (PLUGIN);

	if (current && factory) {
		factory->removeClient(current);
		current->setFactory(nullptr);
	}
	delete menubar;
	delete builder;
	delete factory;
}

void RKXMLGUIPreviewArea::setLabel (const QString& label) {
	RK_TRACE(PLUGIN);
	lab->setText(label);
}

QString RKXMLGUIPreviewArea::label() const {
	RK_TRACE(PLUGIN);
	return lab->text();
}

void RKXMLGUIPreviewArea::setWindow(RKMDIWindow* window) {
	RK_TRACE(PLUGIN);

	if (current) {
		factory->removeClient(current);  // _always_ remove before adding, or the previous child will be leaked in the factory
	}
	window->setWindowStyleHint("preview");
	current = window->getPart();
	internal_layout->addWidget(window);
	factory->addClient(current);
	QList<QAction*> acts = actions();
	for (int i = 0; i < acts.size (); ++i) acts[i]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	RKWorkplace::mainWorkplace()->setWindowNotManaged(window);
}

void RKXMLGUIPreviewArea::prepareMenu () {
	RK_TRACE (PLUGIN);

	// flatten menu, and try to purge irrelevant actions
	menu->clear ();
	QList<QAction*> entries = menubar->actions ();
	for (int i = 0; i < entries.size (); ++i) {
		QMenu *smenu = entries[i]->menu ();
		if (!smenu) continue;    // Don't think it can happen...
		if (entries[i]->objectName () == "settings") continue;  // Skip settings menu, entirely

		QList<QAction*> subentries = smenu->actions ();
		QList<QAction*> entries_to_add;
		bool menu_empty = true;
		for (int j = 0; j < subentries.size (); ++j) {
			QAction *act = subentries[j];
			if (act->isVisible () && act->isEnabled()) {
				entries_to_add.append (act);
				if (!act->isSeparator ()) menu_empty = false;  // Copy separators, but purge menus with only separators in them.
			}
		}
		if (menu_empty) continue;

		QWidgetAction *act = new QWidgetAction (this);
		QLabel *lab = new QLabel ("<b>" + entries[i]->text ().remove ('&') + "</b>");
		lab->setAlignment (Qt::AlignCenter);
		act->setDefaultWidget (lab);
		menu->addAction (act);

		QMenu *where_to_add = menu;
		if (entries_to_add.size () >= 8) {                     // if there are really many entries in the menu don't flatten it, keep it as a (shortened) submenu
			where_to_add = menu->addMenu (entries[i]->text ());
		}
		for (int j = 0; j < entries_to_add.size (); ++j) {
			where_to_add->addAction (entries_to_add[j]);
		}
	}
}



/** Similar to KMessageWidget, but much smaller margins / spacings */
class RKPreviewStatusNote : public QFrame {
friend class RKPreviewManager;
	RKPreviewStatusNote(RKPreviewManager *manager) :
	QFrame(),
	updating(i18nc("very short: Preview is updating", "updating")),
	error(i18nc("very short: Error while generating preview", "error")),
	ready(i18nc("very short: Preview is up to date", "ready")),
	unavailable(i18nc("very short: Preview is not(yet) possible", "n/a")),
	off(i18nc("very short: Preview is turned off", "off"))
	{
		QFontMetrics fm(font());
		em = fm.horizontalAdvance('w');
		auto l = new QVBoxLayout(this);
		l->setContentsMargins(em,0,em,0);
		lab = new QLabel();
		l->addWidget(lab, 0, Qt::AlignHCenter);
		setAutoFillBackground(false);
		setFixedHeight(fm.height()+4);
		int maxw = fm.horizontalAdvance(updating);
		maxw = qMax(maxw, fm.horizontalAdvance(error));
		maxw = qMax(maxw, fm.horizontalAdvance(ready));
		maxw = qMax(maxw, fm.horizontalAdvance(unavailable));
		maxw = qMax(maxw, fm.horizontalAdvance(off));
		setFixedWidth(maxw+2*em+2);

		connect(manager, &RKPreviewManager::statusChanged, this, [this](RKPreviewManager *m) {
			if (m->update_pending == RKPreviewManager::NoUpdatePossible) {
				lab->setText(unavailable);
				setToolTip(i18n("The preview is current unavailable (you will need to make further selections)."));
				setColors(QPalette::Disabled, KColorScheme::NegativeBackground, KColorScheme::NegativeText);
			} else if (m->update_pending == RKPreviewManager::PreviewDisabled) {
				lab->setText(off);
				setToolTip(i18n("Preview is turned off."));
				setColors(QPalette::Disabled, KColorScheme::NormalBackground, KColorScheme::NormalText);
			} else if (m->updating || (m->update_pending == RKPreviewManager::UpdatePending)) {
				lab->setText(updating);
				setToolTip(i18n("Preview is currently being updated."));
				setColors(QPalette::Active, KColorScheme::ActiveBackground, KColorScheme::ActiveText);
			} else if (m->current_preview_failed) {
				lab->setText(error);
				setToolTip(i18n("The command to generate the preview has failed."));
				setColors(QPalette::Active, KColorScheme::NegativeBackground, KColorScheme::NegativeText);
			} else {
				lab->setText(ready);
				setToolTip(i18n("Preview is up to date."));
				setColors(QPalette::Active, KColorScheme::PositiveBackground, KColorScheme::PositiveText);
			}
			update();
		});
	}
	void paintEvent(QPaintEvent *event) {
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(pen);
		painter.setBrush(brush);
		const QRect innerRect = rect().marginsRemoved(QMargins() + 1);
		painter.drawRoundedRect(innerRect, em, em);
		QFrame::paintEvent(event);
	}
	const QString updating;
	const QString error;
	const QString ready;
	const QString unavailable;
	const QString off;
	void setColors(QPalette::ColorGroup group, KColorScheme::BackgroundRole bg, KColorScheme::ForegroundRole fg) {
		auto scheme = KColorScheme(group);
		pen = QPen(scheme.foreground(fg).color());
		brush = QBrush(scheme.background(bg));
	}
	QLabel *lab;
	int em;
	QBrush brush;
	QPen pen;
};

QWidget* RKPreviewManager::inlineStatusWidget() {
	return new RKPreviewStatusNote(this);
}

#include "../windows/rkworkplace.h"

RKPreviewManager::RKPreviewManager(QObject* parent) : QObject (parent), current_preview_failed(false) {
	RK_TRACE (PLUGIN);

	update_pending = NoUpdatePending;
	updating = false;
	id = "0x" + QString::number((quint64) (quintptr) this, 16);
}

RKPreviewManager::~RKPreviewManager () {
	RK_TRACE (PLUGIN);
}

void RKPreviewManager::previewCommandDone (RCommand* command) {
	RK_TRACE (PLUGIN);

	updating = false;
	if (update_pending == NoUpdatePossible) {
		setNoPreviewAvailable();
	} else {
		QString warnings = command->warnings() + command->error();
		if (!warnings.isEmpty()) warnings = QString("<b>%1</b>\n<pre>%2</pre>").arg(i18n("Warnings or Errors:"), warnings.toHtmlEscaped());
		current_preview_failed = command->failed();
		updateStatusDisplay(warnings);
	}
}

void RKPreviewManager::setCommand (RCommand* command) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (!updating);
	updating = true;
	update_pending = NoUpdatePending;
	connect (command->notifier(), &RCommandNotifier::commandFinished, this, &RKPreviewManager::previewCommandDone);

	// Send an empty dummy command first. This is to sync up with any commands that should have been run _before_ the preview (e.g. to set up the preview area, so that status labels can be shown)
	RInterface::whenAllFinished(this, [this]() { updateStatusDisplay(); });

	RInterface::issueCommand (command);
	updateStatusDisplay();
}

void RKPreviewManager::setUpdatePending () {
	if (update_pending == UpdatePending) return;
	RK_TRACE (PLUGIN);

	update_pending = UpdatePending;
	updateStatusDisplay();
}

void RKPreviewManager::setNoPreviewAvailable () {
	if (update_pending == NoUpdatePossible) return;
	RK_TRACE (PLUGIN);

	update_pending = NoUpdatePossible;
	updateStatusDisplay();
}

void RKPreviewManager::setPreviewDisabled () {
	if (update_pending == PreviewDisabled) return;
	RK_TRACE (PLUGIN);

	update_pending = PreviewDisabled;
	updateStatusDisplay();
}

void RKPreviewManager::updateStatusDisplay(const QString &warnings) {
	RK_TRACE (PLUGIN);

	RKMDIWindow *window = RKWorkplace::mainWorkplace()->getNamedWindow(id);
	if (window) window->setStatusMessage(warnings);

	Q_EMIT statusChanged(this);
}
