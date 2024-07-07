/*
rkcommandeditorwindow - This file is part of RKWard (https://rkward.kde.org). Created: Mon Aug 30 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkcommandeditorwindow.h"

#include <kxmlguifactory.h>

#include <ktexteditor/editor.h>
#include <KTextEditor/Application>

#include <QApplication>
#include <QActionGroup>
#include <QFile>
#include <QTimer>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#include <QKeyEvent>
#include <QEvent>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QDir>
#include <QSplitter>

#include <KLocalizedString>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <KIO/DeleteJob>
#include <KIO/FileCopyJob>
#include <kconfiggroup.h>
#include <krandom.h>
#include <kwidgetsaddons_version.h>
#include <ktexteditor_version.h>

#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkstandardactions.h"
#include "../misc/rkxmlguisyncer.h"
#include "../misc/rkjobsequence.h"
#include "../misc/rkxmlguipreviewarea.h"
#include "../core/robjectlist.h"
#include "../rbackend/rkrinterface.h"
#include "../settings/rksettings.h"
#include "../settings/rksettingsmodulecommandeditor.h"
#include "../settings/rkrecenturls.h"
#include "../rkconsole.h"
#include "../rkward.h"
#include "rkhelpsearchwindow.h"
#include "rkhtmlwindow.h"
#include "rkworkplace.h"
#include "katepluginintegration.h"
#include "rkcodecompletion.h"
#include "rktexthints.h"

#include "../debug.h"

RKCommandEditorWindowPart::RKCommandEditorWindowPart (QWidget *parent) : KParts::Part (parent) {
	RK_TRACE (COMMANDEDITOR);

	setComponentName (QCoreApplication::applicationName (), QGuiApplication::applicationDisplayName ());
	setWidget (parent);
	setXMLFile ("rkcommandeditorwindowpart.rc");
}

RKCommandEditorWindowPart::~RKCommandEditorWindowPart () {
	RK_TRACE (COMMANDEDITOR);
}

#define GET_HELP_URL 1
#define NUM_BLOCK_RECORDS 6

//static
QMap<QString, KTextEditor::Document*> RKCommandEditorWindow::unnamed_documents;

KTextEditor::Document* createDocument(bool with_signals) {
	KTextEditor::Document* ret = KTextEditor::Editor::instance()->createDocument (RKWardMainWindow::getMain ());
	if (with_signals) {
		Q_EMIT KTextEditor::Editor::instance()->application()->documentCreated(ret);
	}
	return ret;
}

RKCommandEditorWindow::RKCommandEditorWindow (QWidget *parent, const QUrl &_url, const QString& encoding, int flags) : RKMDIWindow (parent, RKMDIWindow::CommandEditorWindow) {
	RK_TRACE (COMMANDEDITOR);

	QString id_header = QStringLiteral ("unnamedscript://");

	KTextEditor::Editor* editor = KTextEditor::Editor::instance ();
	RK_ASSERT (editor);

	QUrl url = _url;
	m_doc = nullptr;
	preview_io = nullptr;
	visible_to_kateplugins = flags & RKCommandEditorFlags::VisibleToKTextEditorPlugins;
	if (visible_to_kateplugins) addUiBuddy(RKWardMainWindow::getMain()->katePluginIntegration()->mainWindow()->dynamicGuiClient());
	bool use_r_highlighting = (flags & RKCommandEditorFlags::ForceRHighlighting) || (url.isEmpty() && (flags & RKCommandEditorFlags::DefaultToRHighlighting)) || RKSettingsModuleCommandEditor::matchesScriptFileFilter (url.fileName ());

	// Lookup of existing text editor documents: First, if no url is given at all, create a new document, and register an id, in case this window will get split, later
	if (url.isEmpty ()) {
		m_doc = createDocument (visible_to_kateplugins);
		_id = id_header + KRandom::randomString (16).toLower ();
		RK_ASSERT (!unnamed_documents.contains (_id));
		unnamed_documents.insert (_id, m_doc);
	} else if (url.url ().startsWith (id_header)) { // Next, handle the case that a pseudo-url is passed in
		_id = url.url ();
		m_doc = unnamed_documents.value (_id);
		url.clear ();
		if (!m_doc) {  // can happen while restoring saved workplace.
			m_doc = createDocument (visible_to_kateplugins);
			unnamed_documents.insert (_id, m_doc);
		}
	} else {   // regular url given. Try to find an existing document for that url
		       // NOTE: we cannot simply use the same map as above, for this purpose, as document urls may change.
		       // instead we iterate over the document list.
		QList<KTextEditor::Document*> docs = editor->documents ();
		for (int i = 0; i < docs.count (); ++i) {
			if (docs[i]->url ().matches (url, QUrl::NormalizePathSegments | QUrl::StripTrailingSlash | QUrl::PreferLocalFile)) {
				m_doc = docs[i];
				break;
			}
		}
	}

	// if an existing document is re-used, try to honor encoding.
	if (m_doc) {
		if (!encoding.isEmpty () && (m_doc->encoding () != encoding)) {
			m_doc->setEncoding (encoding);
			m_doc->documentReload ();
		}
	}

	// no existing document was found, so create one and load the url
	if (!m_doc) {
		m_doc = createDocument (visible_to_kateplugins); // The document may have to outlive this window

		// encoding must be set *before* loading the file
		if (!encoding.isEmpty ()) m_doc->setEncoding (encoding);
		if (!url.isEmpty ()) {
			if (m_doc->openUrl (url)) {
				// KF5 TODO: Check which parts of this are still needed in KF5, and which no longer work
				if (!(flags & RKCommandEditorFlags::DeleteOnClose)) {	// don't litter config with temporary files
					QString p_url = RKWorkplace::mainWorkplace ()->portableUrl (m_doc->url ());
					KConfigGroup conf (RKWorkplace::mainWorkplace ()->workspaceConfig (), QString ("SkriptDocumentSettings %1").arg (p_url));
					// HACK: Hmm. KTextEditor::Document's readSessionConfig() simply restores too much. Yes, I want to load bookmarks and stuff.
					// I do not want to mess with encoding, or risk loading a different url, after the doc is already loaded!
					if (!encoding.isEmpty () && (conf.readEntry ("Encoding", encoding) != encoding)) conf.writeEntry ("Encoding", encoding);
					if (conf.readEntry ("URL", url) != url) conf.writeEntry ("URL", url);
					// HACK: What the...?! Somehow, at least on longer R scripts, stored Mode="Normal" in combination with R Highlighting
					// causes code folding to fail (KDE 4.8.4, http://sourceforge.net/p/rkward/bugs/122/).
					// Forcing Mode == Highlighting appears to help.
					if (use_r_highlighting) conf.writeEntry ("Mode", conf.readEntry ("Highlighting", "Normal"));
					m_doc->readSessionConfig (conf);
				}
			} else {
				KMessageBox::error(this, i18n ("Unable to open \"%1\"", url.toDisplayString ()), i18n ("Could not open command file"));
			}
		}
	}

	setReadOnly (flags & RKCommandEditorFlags::ReadOnly);

	if (flags & RKCommandEditorFlags::DeleteOnClose) {
		if (flags & RKCommandEditorFlags::ReadOnly) {
			RKCommandEditorWindow::delete_on_close = url;
		} else {
			RK_ASSERT (false);
		}
	}

	RK_ASSERT (m_doc);
	// yes, we want to be notified, if the file has changed on disk.
	// why, oh why is this not the default?
	// this needs to be set *before* the view is created!
	m_doc->setModifiedOnDiskWarning (true);
	m_view = m_doc->createView (this, RKWardMainWindow::getMain ()->katePluginIntegration ()->mainWindow ()->mainWindow());
	if (visible_to_kateplugins) {
		Q_EMIT RKWardMainWindow::getMain ()->katePluginIntegration ()->mainWindow ()->mainWindow()->viewCreated (m_view);
	}
	preview_manager = new RKPreviewManager(this);
	preview = new RKXMLGUIPreviewArea(QString(), this, preview_manager);
	connect (preview_manager, &RKPreviewManager::statusChanged, this, [this]() { preview_timer.start (500); });
	RKWorkplace::mainWorkplace()->registerNamedWindow (preview_manager->previewId(), this, preview);
	if (!url.isEmpty ()) {
		KConfigGroup viewconf (RKWorkplace::mainWorkplace ()->workspaceConfig (), QString ("SkriptViewSettings %1").arg (RKWorkplace::mainWorkplace ()->portableUrl (url)));
		m_view->readSessionConfig (viewconf);
	}

	setFocusProxy (m_view);
	setFocusPolicy (Qt::StrongFocus);

	RKCommandEditorWindowPart* part = new RKCommandEditorWindowPart (m_view);
	part->insertChildClient (m_view);
	setPart (part);
	fixupPartGUI ();
	setMetaInfo (i18n ("Script Editor"), QUrl (), RKSettings::PageCommandEditor);
	initializeActions (part->actionCollection ());
	// The kate part is quite a beast to embed, when it comes to shortcuts. New ones get added, conflicting with ours.
	// In this context we show no mercy, and rip out any conflicting shortcuts.
	const auto kate_acs = m_view->findChildren<KActionCollection*>() << m_view->actionCollection();
	QList<KActionCollection*> own_acs;
	own_acs.append(part->actionCollection());
	own_acs.append(standardActionCollection());
	// How's this for a nested for loop...
	for (const auto ac : own_acs) {
		const auto own_actions = ac->actions();
		for (const auto own_action : own_actions) {
			const auto own_scs = ac->defaultShortcuts(own_action);
			for (const auto &own_sc : own_scs) {
				for (const auto kate_ac : kate_acs) {
					const auto kate_actions = kate_ac->actions();
					for (auto kate_action : kate_actions) {
						auto action_shortcuts = kate_ac->defaultShortcuts(kate_action);
						for (int pos = 0; pos < action_shortcuts.size(); ++pos) {
							const auto &kate_sc = action_shortcuts[pos];
							if (own_sc.matches(kate_sc) != QKeySequence::NoMatch || kate_sc.matches(own_sc) != QKeySequence::NoMatch) {
								RK_DEBUG(EDITOR, DL_WARNING, "Removing conflicting shortcut %s in kate part (%s, conflicting with %s)", qPrintable(kate_sc.toString()), qPrintable(kate_action->objectName()), qPrintable(own_action->objectName()));
								action_shortcuts.removeAt(pos);
								kate_ac->setDefaultShortcuts(kate_action, action_shortcuts);
								break;
							}
						}
					}
				}
			}
		}
	}
	initializeActivationSignals ();
	RKXMLGUISyncer::self()->registerChangeListener (m_view, this, SLOT (fixupPartGUI()));

	QHBoxLayout *layout = new QHBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	QSplitter* preview_splitter = new QSplitter (this);
	preview_splitter->addWidget (m_view);
	preview_splitter->addWidget(preview);
	preview->hide();
	connect (m_doc, &KTextEditor::Document::documentSavedOrUploaded, this, &RKCommandEditorWindow::documentSaved);
	layout->addWidget(preview_splitter);

	setGlobalContextProperty("current_filename", m_doc->url().url());
	connect (m_doc, &KTextEditor::Document::documentUrlChanged, this, &RKCommandEditorWindow::urlChanged);
	connect (m_doc, &KTextEditor::Document::modifiedChanged, this, &RKCommandEditorWindow::updateCaption);                // of course most of the time this causes a redundant call to updateCaption. Not if a modification is undone, however.
#ifdef __GNUC__
#warning remove this in favor of KTextEditor::Document::restore()
#endif
	connect (m_doc, &KTextEditor::Document::modifiedChanged, this, &RKCommandEditorWindow::autoSaveHandlerModifiedChanged);
	connect (m_doc, &KTextEditor::Document::textChanged, this, &RKCommandEditorWindow::textChanged);
	connect (m_view, &KTextEditor::View::selectionChanged, this, &RKCommandEditorWindow::selectionChanged);
	// somehow the katepart loses the context menu each time it loses focus
	connect (m_view, &KTextEditor::View::focusIn, this, &RKCommandEditorWindow::focusIn);

	if (use_r_highlighting) {
		RKCommandHighlighter::setHighlighting (m_doc, RKCommandHighlighter::RScript);
	} else {
		RKCommandHighlighter::setHighlighting (m_doc, RKCommandHighlighter::Automatic);
	}
	if (use_r_highlighting || RKSettingsModuleCommandEditor::completionSettings()->completionForAllFileTypes()) {
		if (flags & RKCommandEditorFlags::UseCodeHinting) {
			new RKCompletionManager (m_view, RKSettingsModuleCommandEditor::completionSettings());
			new RKTextHints(m_view, RKSettingsModuleCommandEditor::completionSettings());
			//hinter = new RKFunctionArgHinter (this, m_view);
		}
	}

	initBlocks ();

	connect (&autosave_timer, &QTimer::timeout, this, &RKCommandEditorWindow::doAutoSave);
	connect (&preview_timer, &QTimer::timeout, this, &RKCommandEditorWindow::doRenderPreview);

	urlChanged();	// initialize
}

RKCommandEditorWindow::~RKCommandEditorWindow () {
	RK_TRACE (COMMANDEDITOR);

	bool have_url = !url().isEmpty(); // cache early, as potentially needed after destruction of m_doc (at which point calling url() may crash
	if (have_url) {
		QString p_url = RKWorkplace::mainWorkplace ()->portableUrl (m_doc->url ());
		KConfigGroup conf (RKWorkplace::mainWorkplace ()->workspaceConfig (), QString ("SkriptDocumentSettings %1").arg (p_url));
		m_doc->writeSessionConfig (conf);
		KConfigGroup viewconf (RKWorkplace::mainWorkplace ()->workspaceConfig (), QString ("SkriptViewSettings %1").arg (p_url));
		m_view->writeSessionConfig (viewconf);
	}

	discardPreview ();
	m_doc->waitSaveComplete ();
	QList<KTextEditor::View*> views = m_doc->views();
	if (views.size() == 1) {
		// if this is the only view, destroy the document. Note that doc has to be destroyed before view. konsole plugin (and possibly others) assumes it.
		RK_ASSERT(views.at(0) == m_view);
		if (visible_to_kateplugins) {
			Q_EMIT KTextEditor::Editor::instance()->application()->documentWillBeDeleted(m_doc);
		}
		m_doc->deleteLater();
		if (visible_to_kateplugins) {
			Q_EMIT KTextEditor::Editor::instance()->application()->documentDeleted(m_doc);
		}
		if (!delete_on_close.isEmpty ()) KIO::del (delete_on_close)->start ();
		unnamed_documents.remove (_id);
	} else {
		RK_ASSERT(!views.isEmpty()); // should contain at least m_view at this point
		delete m_view;
	}
	// NOTE, under rather unlikely circumstances, the above may leave stale ids->stale pointers in the map: Create unnamed window, split it, save to a url, split again, close the first two windows, close the last. This situation should be caught by the following, however:
	if (have_url && !_id.isEmpty ()) {
		unnamed_documents.remove (_id);
	}
}

void RKCommandEditorWindow::fixupPartGUI () {
	RK_TRACE (COMMANDEDITOR);

	// strip down the katepart's GUI. remove some stuff we definitely don't need.
	RKCommonFunctions::removeContainers (m_view, QString ("bookmarks,tools_spelling,tools_spelling_from_cursor,tools_spelling_selection,switch_to_cmd_line,set_confdlg").split (','), true);
	RKCommonFunctions::moveContainer (m_view, "Menu", "tools", "edit", true);
}

QAction *findAction (KTextEditor::View* view, const QString &actionName) {
	// katepart has more than one actionCollection
	QList<KActionCollection*> acs = view->findChildren<KActionCollection*>();
	acs.append (view->actionCollection ());

	for (KActionCollection* ac : std::as_const(acs)) {
		QAction* found = ac->action (actionName);
		if (found) return found;
	}

	return nullptr;
}

void RKCommandEditorWindow::initializeActions (KActionCollection* ac) {
	RK_TRACE (COMMANDEDITOR);

	RKStandardActions::copyLinesToOutput (this, this, SLOT (copyLinesToOutput()));
	RKStandardActions::pasteSpecial (this, this, SLOT (paste(QString)));

	action_run_all = RKStandardActions::runAll (this, this, SLOT (runAll()));
	action_run_current = RKStandardActions::runCurrent (this, this, SLOT (runCurrent()), true);
	// NOTE: enter_and_submit is not currently added to the menu
	QAction *action = ac->addAction("enter_and_submit", this, &RKCommandEditorWindow::enterAndSubmit);
	action->setText (i18n ("Insert line break and run"));
	ac->setDefaultShortcuts (action, QList<QKeySequence>() << (Qt::AltModifier | Qt::Key_Return) << (Qt::AltModifier | Qt::Key_Enter));
	ac->setDefaultShortcut (action, Qt::AltModifier | Qt::Key_Return); // KF5 TODO: This line needed only for KF5 < 5.2, according to documentation

	RKStandardActions::functionHelp (this, this);
	RKStandardActions::onlineHelp (this, this);

	actionmenu_run_block = new KActionMenu (i18n ("Run block"), this);
	actionmenu_run_block->setPopupMode(QToolButton::InstantPopup);
	ac->addAction ("run_block", actionmenu_run_block);
	connect (actionmenu_run_block->menu(), &QMenu::aboutToShow, this, &RKCommandEditorWindow::clearUnusedBlocks);
	actionmenu_mark_block = new KActionMenu (i18n ("Mark selection as block"), this);
	ac->addAction ("mark_block", actionmenu_mark_block);
	connect (actionmenu_mark_block->menu(), &QMenu::aboutToShow, this, &RKCommandEditorWindow::clearUnusedBlocks);
	actionmenu_unmark_block = new KActionMenu (i18n ("Unmark block"), this);
	ac->addAction ("unmark_block", actionmenu_unmark_block);
	connect (actionmenu_unmark_block->menu(), &QMenu::aboutToShow, this, &RKCommandEditorWindow::clearUnusedBlocks);

	action_setwd_to_script = ac->addAction("setwd_to_script", this, &RKCommandEditorWindow::setWDToScript);
	action_setwd_to_script->setText (i18n ("CD to script directory"));
	action_setwd_to_script->setWhatsThis(i18n ("Change the working directory to the directory of this script"));
	action_setwd_to_script->setToolTip (action_setwd_to_script->statusTip ());
	action_setwd_to_script->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionCDToScript));

	KActionMenu* actionmenu_preview = new KActionMenu(QIcon::fromTheme("view-preview"), i18n("Preview"), this);
	actionmenu_preview->setPopupMode(QToolButton::InstantPopup);
	preview_modes = new QActionGroup(this);
	action_no_preview = preview_modes->addAction(RKStandardIcons::getIcon(RKStandardIcons::ActionDelete), i18n("No preview"));
	action_no_preview->setToolTip(i18n("Disable preview"));
	action_no_preview->setCheckable(true);
	action_no_preview->setChecked(true);
	actionmenu_preview->addAction(action_no_preview);
	initPreviewModes();
	for (int i = 0; i < preview_mode_list.size(); ++i) {
		const PreviewMode &mode = preview_mode_list[i];
		QAction *a = preview_modes->addAction(mode.icon, mode.actionlabel);
		a->setWhatsThis(mode.tooltip);
		a->setToolTip(mode.tooltip);
		a->setCheckable(true);
		actionmenu_preview->addAction(a);
	}
	connect(preview, &RKXMLGUIPreviewArea::previewClosed, this, &RKCommandEditorWindow::discardPreview);
	connect(preview_modes, &QActionGroup::triggered, this, &RKCommandEditorWindow::changePreviewMode);

	actionmenu_preview->addSeparator ();
	action_preview_as_you_type = new QAction (QIcon::fromTheme ("input-keyboard"), i18nc ("Checkable action: the preview gets updated while typing", "Update as you type"), ac);
	action_preview_as_you_type->setToolTip (i18n ("When this option is enabled, an update of the preview will be triggered every time you modify the script. When this option is disabled, the preview will be updated whenever you save the script, only."));
	action_preview_as_you_type->setCheckable (true);
	action_preview_as_you_type->setChecked (m_doc->url ().isEmpty ());  // By default, update as you type for unsaved "quick and dirty" scripts, preview on save for saved scripts
	actionmenu_preview->addAction (action_preview_as_you_type);
	ac->addAction ("render_preview", actionmenu_preview);

	file_save_action = findAction (m_view, "file_save");
	if (file_save_action) file_save_action->setText (i18n ("Save Script..."));
	file_save_as_action = findAction (m_view, "file_save_as");
	if (file_save_as_action) file_save_as_action->setText (i18n ("Save Script As..."));
}

void RKCommandEditorWindow::initBlocks () {
	RK_TRACE (COMMANDEDITOR);
	RK_ASSERT (block_records.isEmpty ());

	KActionCollection* ac = getPart ()->actionCollection ();

	int i = 0;
	QColor colors[NUM_BLOCK_RECORDS];
	colors[i++] = QColor (255, 0, 0);
	colors[i++] = QColor (0, 255, 0);
	colors[i++] = QColor (0, 0, 255);
	colors[i++] = QColor (255, 255, 0);
	colors[i++] = QColor (255, 0, 255);
	colors[i++] = QColor (0, 255, 255);
	RK_ASSERT (i == NUM_BLOCK_RECORDS);

	// sorry for those idiotic shortcuts, but I just could not find any decent unused ones
	i = 0;
	QKeySequence shortcuts[NUM_BLOCK_RECORDS];
	shortcuts[i++] = QKeySequence (Qt::AltModifier | Qt::ShiftModifier | Qt::Key_F1);
	shortcuts[i++] = QKeySequence (Qt::AltModifier | Qt::ShiftModifier | Qt::Key_F2);
	shortcuts[i++] = QKeySequence (Qt::AltModifier | Qt::ShiftModifier | Qt::Key_F3);
	shortcuts[i++] = QKeySequence (Qt::AltModifier | Qt::ShiftModifier | Qt::Key_F4);
	shortcuts[i++] = QKeySequence (Qt::AltModifier | Qt::ShiftModifier | Qt::Key_F5);
	shortcuts[i++] = QKeySequence (Qt::AltModifier | Qt::ShiftModifier | Qt::Key_F6);
	RK_ASSERT (i == NUM_BLOCK_RECORDS);

	for (i = 0; i < NUM_BLOCK_RECORDS; ++i) {
		BlockRecord record;

		QColor shaded = colors[i];
		shaded.setAlpha (30);
		record.attribute = KTextEditor::Attribute::Ptr (new KTextEditor::Attribute ());
		record.attribute->setBackgroundFillWhitespace (false);
		record.attribute->setBackground (shaded);

		QPixmap colorsquare (16, 16);
		colorsquare.fill (colors[i]);
		QIcon icon (colorsquare);

		record.mark = ac->addAction("markblock" + QString::number(i), this, [this, i](){ markBlock(i); });
		record.mark->setIcon (icon);
		actionmenu_mark_block->addAction (record.mark);
		record.unmark = ac->addAction("unmarkblock" + QString::number(i), this, [this, i](){ unmarkBlock(i); });
		record.unmark->setIcon (icon);
		actionmenu_unmark_block->addAction (record.unmark);
		record.run = ac->addAction("runblock" + QString::number(i), this, [this, i](){ runBlock(i); });
		ac->setDefaultShortcut (record.run, shortcuts[i]);
		record.run->setIcon (icon);
		actionmenu_run_block->addAction (record.run);

		// these two not strictly needed due to removeBlock(), below. Silences a GCC warning, however.
		record.range = nullptr;
		record.active = false;

		block_records.append (record);
		removeBlock (i, true);	// initialize to empty
	}
	RK_ASSERT (block_records.size () == NUM_BLOCK_RECORDS);
}

void RKCommandEditorWindow::focusIn (KTextEditor::View* v) {
	RK_TRACE (COMMANDEDITOR);
	RK_ASSERT (v == m_view);
}

QString RKCommandEditorWindow::fullCaption () {
	RK_TRACE (COMMANDEDITOR);

	if (m_doc->url ().isEmpty ()) {
		return (shortCaption ());
	} else {
		QString cap = m_doc->url ().toDisplayString (QUrl::PreferLocalFile | QUrl::PrettyDecoded);
		if (isModified ()) cap.append (i18n (" [modified]"));
		return (cap);
	}
}

void RKCommandEditorWindow::closeEvent (QCloseEvent *e) {
	if (isModified ()) {
		int status = KMessageBox::warningTwoActions (this, i18n ("The document \"%1\" has been modified. Close it anyway?", windowTitle ()), i18n ("File not saved"), KStandardGuiItem::close(), KStandardGuiItem::cancel());
	
		if (status != KMessageBox::PrimaryAction) {
			e->ignore ();
			return;
		}
	}

	QWidget::closeEvent (e);
}

void RKCommandEditorWindow::setWindowStyleHint (const QString& hint) {
	RK_TRACE (COMMANDEDITOR);

	m_view->setStatusBarEnabled (hint != "preview");
	RKMDIWindow::setWindowStyleHint (hint);
}

void RKCommandEditorWindow::copy () {
	RK_TRACE (COMMANDEDITOR);

	QApplication::clipboard()->setText (m_view->selectionText ());
}

void RKCommandEditorWindow::setReadOnly (bool ro) {
	RK_TRACE (COMMANDEDITOR);

	m_doc->setReadWrite (!ro);
}

void RKCommandEditorWindow::autoSaveHandlerModifiedChanged () {
	RK_TRACE (COMMANDEDITOR);

	if (!isModified ()) {
		autosave_timer.stop ();

		if (RKSettingsModuleCommandEditor::autosaveKeep ()) return;
		if (!previous_autosave_url.isValid ()) return;
		if (previous_autosave_url.isLocalFile ()) {
			QFile::remove (previous_autosave_url.toLocalFile ());
		} else {
			RKJobSequence* dummy = new RKJobSequence ();
			dummy->addJob (KIO::del (previous_autosave_url));
			connect (dummy, &RKJobSequence::finished, this, &RKCommandEditorWindow::autoSaveHandlerJobFinished);
			dummy->start ();
		}
		previous_autosave_url.clear ();
	}
}

class RKScriptPreviewIO {
	QUrl url;
	int preview_mode;
	QTemporaryDir out_dir;
	QFile* infile;
	QString extension;
	RKScriptPreviewIO(const QString &extension, const QUrl &url) : url(url), preview_mode(-1), out_dir(), infile(nullptr), extension(extension) {
		const auto pattern = QLatin1String("tmp_rkward_preview");
		if (url.isEmpty() || !url.isLocalFile()) {
			// Not locally saved: save to tempdir
			infile = new QFile(QDir(out_dir.path()).absoluteFilePath(pattern + extension));
		} else {
			// If the file is already saved, save the preview input as a temp file in the same folder.
			// esp. .Rmd files might try to include other files by relative path.
			QString tempfiletemplate = url.adjusted(QUrl::RemoveFilename).toLocalFile();
			tempfiletemplate.append(pattern + QLatin1String("XXXXXX") + extension);
			infile = new QTemporaryFile(tempfiletemplate);
		}
	}
	void write(KTextEditor::Document *doc) {
		RK_ASSERT(infile->open(QIODevice::WriteOnly));
		QTextStream out(infile);
		out.setEncoding(QStringConverter::Utf8); // make sure that all characters can be saved, without nagging the user
		out << doc->text();
		infile->close();
	}
public:
	~RKScriptPreviewIO() {
		infile->remove();
		delete infile;
	}
	static RKScriptPreviewIO* init(RKScriptPreviewIO *previous, KTextEditor::Document *doc, int preview_mode, const QString &extension) {
		// Whenever possible, we try to reuse an existing temporary file, because
		// a) If build files do spill (as happens with rmarkdown::render()), it will not be quite as many
		// b) Faster in some cases
		if (previous && previous->preview_mode == preview_mode && previous->url == doc->url() && previous->extension == extension) {
			// If re-using an existing filename, remove it first. Somehow, contrary to documentation, this does not happen in QFile::open(WriteOnly).
			previous->infile->remove();
			previous->write(doc);
			return previous;
		}
		delete previous;
		auto ret = new RKScriptPreviewIO(extension, doc->url());
		ret->url = doc->url();
		ret->preview_mode = preview_mode;
		ret->write(doc);
		return ret;
	}
	QString outpath(const QString &filename) const {
		return QDir(out_dir.path()).absoluteFilePath(filename);
	}
	QString inpath() const {
		return infile->fileName();
	}
};

void RKCommandEditorWindow::changePreviewMode (QAction *mode) {
	RK_TRACE (COMMANDEDITOR);

	if (mode != action_no_preview) {
		if (!(preview_io || RKWardMainWindow::suppressModalDialogsForTesting())) {  // triggered on change from no preview to some preview, but not between previews
			if (KMessageBox::warningContinueCancel(this, i18n("<p>The preview feature <b>tries</b> to avoid making any lasting changes to your workspace (technically, by making use of a <i>local()</i> evaluation environment). <b>However, there are cases where using the preview feature may cause unexpected side-effects</b>.</p><p>In particular, this is the case for scripts that contain explicit assignments to <i>globalenv()</i>, or to scripts that alter files on your filesystem. Further, attaching/detaching packages or package namespaces will affect the entire running R session.</p><p>Please keep this in mind when using the preview feature, and especially when using the feature on scripts originating from untrusted sources.</p>"), i18n ("Potential side-effects of previews"), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), QStringLiteral("preview_side_effects")) != KMessageBox::Continue) {
				discardPreview ();
			}
		}
		preview_manager->setUpdatePending ();
		preview_timer.start (0);
	} else {
		discardPreview ();
	}
}

void RKCommandEditorWindow::discardPreview () {
	RK_TRACE (COMMANDEDITOR);

	if (preview_io) {
		preview->hide ();
		preview_manager->setPreviewDisabled ();
		RInterface::issueCommand (QString (".rk.killPreviewDevice(%1)\nrk.discard.preview.data (%1)").arg (RObject::rQuote(preview_manager->previewId ())), RCommand::App | RCommand::Sync);
		delete preview_io;
		preview_io = nullptr;
	}
	action_no_preview->setChecked (true);
}

// TODO: This would probably better go into the R package. The missing ingredient, there, is i18n(), however.
QString RmarkDownRender(const QString &infile, const QString &outdir, const QString &mode_arg) {
	return QStringLiteral(
	".check.for.software <- function(command, message) {\n"
	"	output <- ''\n"
	"	for (i in 1:length(command)) {\n"
	"		if (!nzchar(Sys.which(command[i]))) output <- paste0(output, '<h2>', %1, '</h2><p>', message[i], '</p>\n')\n"
	"	}\n"
	"	output\n"
	"}\n"
	"require(rmarkdown)\n"
	"res <- try({\n"
	"	rmarkdown::render(%6, output_dir=%7,%8 quiet=TRUE)\n"
	"})\n"
	"if (inherits(res, 'try-error')) {\n"
	"	msg <- attr(res, 'condition')$message\n"
	"	out <- paste0('<h1>', %2, '</h1>')\n"
	"	if (length(grep('pandoc', msg))) {\n"
	"		out <- paste0(out, .check.for.software('pandoc', %3))\n"
	"	}\n"
	"	if (length(grep('pdflatex', msg))) {\n"
	"		out <- paste0(out, .check.for.software('pdflatex', %4))\n"
	"	}\n"
	"	rk.show.html(content=out)\n"
	"	stop('%2')\n" // make sure, the status display sees this as an error
	"} else {\n"
	"	if (endsWith(toupper(res), '.PDF')) {\n"
	"		rk.show.pdf(res)\n"
	"	} else if (endsWith(toupper(res), '.HTML')) {\n"
	"		rk.show.html(res)\n"
	"	} else {\n"
	"		rk.show.html(content=paste0(%5, '<p><a href=\"', res, '\">', res, '</a></p>'))\n"
	"	}\n"
	"}\n").arg(
		RObject::rQuote(i18nc("Caption: Some software is missing.", "Missing software")),
		RObject::rQuote(i18n("Rendering the preview failed")),
		RObject::rQuote(i18n("The software <tt>pandoc</tt>, required to rendering R markdown files, is not installed, or not in the system path of the running "
		"R session. You will need to install pandoc from <a href=\"https://pandoc.org/\">https://pandoc.org/</a>.</br>If it is installed, but cannot be found, "
		"try adding it to the system path of the running R session at <a href=\"rkward://settings/rbackend\">Settings->Configure RKward->R-backend</a>.")),
		RObject::rQuote(i18n("The software <tt>pdflatex</tt> is required for rendering PDF previews. The easiest way to install it is by running <tt>install.packages(\"tinytex\"); library(\"tinytex\"); install_tinytex()</tt>")),
		RObject::rQuote(i18n("<h1>Unsupported format</h1><p>The preview cannot be shown, here, because the output format is neither HTML, nor PDF. You can try opening it in an external application, using the link, below, or you can change the preview mode to 'R Markdown (HTML)'.</p>")),
		RObject::rQuote(infile),
		RObject::rQuote(outdir),
		mode_arg
	);
}

void RKCommandEditorWindow::initPreviewModes() {
	RK_TRACE(COMMANDEDITOR);
	preview_mode_list.append(PreviewMode{
		QIcon::fromTheme("preview_math"), i18n("R Markdown (HTML)"), i18n("Preview of rendered R Markdown"),
		i18n("Preview the script as rendered from RMarkdown format (.Rmd) to HTML."),
		QLatin1String(".Rmd"),
		[](const QString& infile, const QString& outdir, const QString& /*preview_id*/) {
			return RmarkDownRender(infile, outdir, "output_format=\"html_document\", ");
		}
	});
	preview_mode_list.append(PreviewMode{
		QIcon::fromTheme("preview_math"), i18n("R Markdown (auto)"), i18n("Preview of rendered R Markdown"),
		i18n("Preview the script as rendered from RMarkdown format (.Rmd) to the format specified in the document header."),
		QLatin1String(".Rmd"),
		[](const QString& infile, const QString& outdir, const QString& /*preview_id*/) {
			return RmarkDownRender(infile, outdir, QString());
		}
	});
	preview_mode_list.append(PreviewMode{
		RKStandardIcons::getIcon(RKStandardIcons::WindowOutput), i18n("RKWard Output"), i18n("Preview of generated RKWard output"),
		i18n("Preview any output to the RKWard Output Window. This preview will be empty, if there is no call to <i>rk.print()</i> or other RKWard output commands."),
		QLatin1String(".R"),
		[](const QString& infile, const QString& outdir, const QString& /*preview_id*/) {
			auto command = QLatin1String("output <- rk.set.output.html.file(%2, silent=TRUE)\n"
				"try(rk.flush.output(ask=FALSE, style=\"preview\", silent=TRUE))\n"
				"try(source(%1, local=TRUE))\n"
				"rk.set.output.html.file(output, silent=TRUE)\n"
				"rk.show.html(%2)\n");
			return command.arg(RObject::rQuote(infile), RObject::rQuote(outdir + QLatin1String("/output.html")));
		}
	});
	preview_mode_list.append(PreviewMode{
		RKStandardIcons::getIcon(RKStandardIcons::WindowConsole), i18n("R Console"), i18n("Preview of script running in interactive R Console"),
		i18n("Preview the script as if it was run in the interactive R Console"),
		QLatin1String(".R"),
		[](const QString& infile, const QString& outdir, const QString& /*preview_id*/) {
			auto command = QLatin1String("output <- rk.set.output.html.file(%2, silent=TRUE)\n"
				"on.exit(rk.set.output.html.file(output, silent=TRUE))\n"
				"try(rk.flush.output(ask=FALSE, style=\"preview\", silent=TRUE))\n"
				"exprs <- expression(NULL)\n"
				"rk.capture.output(suppress.messages=TRUE)\n"
				"try({\n"
				"    exprs <- parse (%1, keep.source=TRUE)\n"
				"})\n"
				".rk.cat.output(rk.end.capture.output(TRUE))\n"
				"for (i in seq_len(length(exprs))) {\n"
				"    rk.print.code(as.character(attr(exprs, \"srcref\")[[i]]))\n"
				"    rk.capture.output(suppress.messages=TRUE)\n"
				"    try({\n"
				"        withAutoprint(exprs[[i]], evaluated=TRUE, echo=FALSE)\n"
				"    })\n"
				"    .rk.cat.output(rk.end.capture.output(TRUE))\n"
				"}\n"
				"rk.set.output.html.file(output, silent=TRUE)\n"
				"rk.show.html(%2)\n");
			return command.arg(RObject::rQuote(infile), RObject::rQuote(outdir + QLatin1String("/output.html")));
		}
	});
	preview_mode_list.append(PreviewMode{
		RKStandardIcons::getIcon(RKStandardIcons::WindowX11), i18n("Plot"), i18n("Preview of generated plot"),
		i18n("Preview any onscreen graphics produced by running this script. This preview will be empty, if there is no call to <i>plot()</i> or other graphics commands."),
		QLatin1String(".R"),
		[](const QString& infile, const QString& /*outdir*/, const QString& preview_id) {
			auto command = QLatin1String("olddev <- dev.cur()\n"
				".rk.startPreviewDevice(%2)\n"
				"try(source(%1, local=TRUE, print.eval=TRUE))\n"
				"if (olddev != 1) dev.set(olddev)\n");
			return command.arg(RObject::rQuote(infile), RObject::rQuote(preview_id));
		}
	});
}

void RKCommandEditorWindow::doRenderPreview () {
	RK_TRACE (COMMANDEDITOR);

	if (action_no_preview->isChecked()) return;
	if (!preview_manager->needsCommand()) return;
	int nmode = preview_modes->actions().indexOf(preview_modes->checkedAction());
	const PreviewMode &mode = preview_mode_list.value(nmode-1);

	if (!preview->findChild<RKMDIWindow*>()) {
		// (lazily) initialize the preview window with _something_, as an RKMDIWindow is needed to display messages (important, if there is an error during the first preview)
		RInterface::issueCommand(".rk.with.window.hints(rk.show.html(content=\"\"), \"\", " + RObject::rQuote(preview_manager->previewId()) + ", style=\"preview\")", RCommand::App | RCommand::Sync);
	}

	preview_io = RKScriptPreviewIO::init(preview_io, m_doc, nmode, mode.input_ext);
	QString command = mode.command(preview_io->inpath(), preview_io->outpath(""), preview_manager->previewId());
	preview->setLabel(mode.previewlabel);
	preview->show();

	RCommand *rcommand = new RCommand(".rk.with.window.hints(local({\n" + command + QStringLiteral("}), \"\", ") + RObject::rQuote(preview_manager->previewId()) + ", style=\"preview\")", RCommand::App);
	preview_manager->setCommand(rcommand);
}

void RKCommandEditorWindow::documentSaved () {
	RK_TRACE (COMMANDEDITOR);

	if (!action_preview_as_you_type->isChecked ()) {
		if (!action_no_preview->isChecked ()) {
			preview_manager->setUpdatePending ();
			preview_timer.start (0);
		}
	}
}

void RKCommandEditorWindow::textChanged () {
	RK_TRACE (COMMANDEDITOR);

	// render preview
	if (!action_no_preview->isChecked ()) {
		if (action_preview_as_you_type->isChecked ()) {
			preview_manager->setUpdatePending ();
			preview_timer.start (500);              // brief delay to buffer keystrokes
		}
	} else {
		discardPreview ();
	}

	// auto save
	if (!isModified ()) return;		// may happen after load or undo
	if (!RKSettingsModuleCommandEditor::autosaveEnabled ()) return;
	if (!autosave_timer.isActive ()) {
		autosave_timer.start (RKSettingsModuleCommandEditor::autosaveInterval () * 60 * 1000);
	}
}

void RKCommandEditorWindow::doAutoSave () {
	RK_TRACE (COMMANDEDITOR);
	RK_ASSERT (isModified ());

	QTemporaryFile save (QDir::tempPath () + QLatin1String ("/rkward_XXXXXX") + RKSettingsModuleCommandEditor::autosaveSuffix ());
	RK_ASSERT (save.open ());
	QTextStream out (&save);
	out.setEncoding (QStringConverter::Utf8);		// make sure that all characters can be saved, without nagging the user
	out << m_doc->text ();
	save.close ();
	save.setAutoRemove (false);

	RKJobSequence* alljobs = new RKJobSequence ();
	// The KJob-Handling below seems to be a bit error-prone, at least for the file-protocol on Windows.
	// Thus, for the simple case of local files, we use QFile, instead.
	connect (alljobs, &RKJobSequence::finished, this, &RKCommandEditorWindow::autoSaveHandlerJobFinished);
	// backup the old autosave file in case something goes wrong during pushing the new one
	QUrl backup_autosave_url;
	if (previous_autosave_url.isValid ()) {
		backup_autosave_url = previous_autosave_url;
		backup_autosave_url = backup_autosave_url.adjusted(QUrl::RemoveFilename);
		backup_autosave_url.setPath(backup_autosave_url.path() + backup_autosave_url.fileName () + '~');
		if (previous_autosave_url.isLocalFile ()) {
			QFile::remove (backup_autosave_url.toLocalFile ());
			QFile::copy (previous_autosave_url.toLocalFile (), backup_autosave_url.toLocalFile ());
		} else {
			alljobs->addJob (KIO::file_move (previous_autosave_url, backup_autosave_url, -1, KIO::HideProgressInfo | KIO::Overwrite));
		}
	}
	
	// push the newly written file
	if (url ().isValid ()) {
		QUrl autosave_url = url ();
		autosave_url = autosave_url.adjusted(QUrl::RemoveFilename);
		autosave_url.setPath(autosave_url.path() + autosave_url.fileName () + RKSettingsModuleCommandEditor::autosaveSuffix ());
		if (autosave_url.isLocalFile ()) {
			QFile::remove (autosave_url.toLocalFile ());
			save.copy (autosave_url.toLocalFile ());
			save.remove ();
		} else {
			alljobs->addJob (KIO::file_move (QUrl::fromLocalFile (save.fileName ()), autosave_url, -1, KIO::HideProgressInfo | KIO::Overwrite));
		}
		previous_autosave_url = autosave_url;
	} else {		// i.e., the document is still "Untitled"
		previous_autosave_url = QUrl::fromLocalFile (save.fileName ());
	}

	// remove the backup
	if (backup_autosave_url.isValid ()) {
		if (backup_autosave_url.isLocalFile ()) {
			QFile::remove (backup_autosave_url.toLocalFile ());
		} else {
			alljobs->addJob (KIO::del (backup_autosave_url, KIO::HideProgressInfo));
		}
	}
	alljobs->start ();

	// do not create any more autosaves until the text is changed, again
	autosave_timer.stop ();
}

void RKCommandEditorWindow::autoSaveHandlerJobFinished (RKJobSequence* seq) {
	RK_TRACE (COMMANDEDITOR);

	if (seq->hadError ()) {
		KMessageBox::detailedError (this, i18n ("An error occurred while trying to create an autosave of the script file '%1':", url ().url ()), "- " + seq->errors ().join ("\n- "));
	}
}

QUrl RKCommandEditorWindow::url () const {
//	RK_TRACE (COMMANDEDITOR);
	return (m_doc->url ());
}

bool RKCommandEditorWindow::isModified() const {
	RK_TRACE (COMMANDEDITOR);
	return m_doc->isModified();
}

bool RKCommandEditorWindow::save () {
	RK_TRACE (COMMANDEDITOR);
	return m_doc->documentSave ();
}

void RKCommandEditorWindow::insertText (const QString &text) {
// KDE4: inline?
	RK_TRACE (COMMANDEDITOR);
	m_view->insertText (text);
	setFocus();
}

void RKCommandEditorWindow::restoreScrollPosition () {
	RK_TRACE (COMMANDEDITOR);

	KTextEditor::Cursor c = saved_scroll_position;
	c.setLine (qMin (c.line (), m_doc->lines () - 1));
	if (c.column () >= m_doc->lineLength (c.line ())) c.setColumn (0);
	m_view->setCursorPosition (c);
}

void RKCommandEditorWindow::saveScrollPosition () {
	RK_TRACE (COMMANDEDITOR);

	KTextEditor::Cursor c = m_view->cursorPosition ();
	if (!c.isValid ()) c = KTextEditor::Cursor::start ();
	saved_scroll_position = c;
}

void RKCommandEditorWindow::setText (const QString &text) {
	RK_TRACE (COMMANDEDITOR);
	bool old_rw = m_doc->isReadWrite ();
	if (!old_rw) m_doc->setReadWrite (true);
	m_doc->setText (text);
	m_doc->clearMarks ();
	if (!old_rw) m_doc->setReadWrite (false);
}

void RKCommandEditorWindow::highlightLine (int linenum) {
	RK_TRACE (COMMANDEDITOR);

	bool old_rw = m_doc->isReadWrite ();
	if (!old_rw) m_doc->setReadWrite (true);
	m_doc->addMark (linenum, KTextEditor::Document::Execution);
	m_view->setCursorPosition (KTextEditor::Cursor (linenum, 0));
	if (!old_rw) m_doc->setReadWrite (false);
}

void RKCommandEditorWindow::urlChanged() {
	RK_TRACE (COMMANDEDITOR);
	updateCaption();
	setGlobalContextProperty("current_filename", url().url());
	if (!url().isEmpty()) RKRecentUrls::addRecentUrl(RKRecentUrls::scriptsId(), url());
	action_setwd_to_script->setEnabled (!url().isEmpty ());
}


void RKCommandEditorWindow::updateCaption () {
	QString name = url ().fileName ();
	if (name.isEmpty ()) name = url ().toDisplayString ();
	if (name.isEmpty ()) name = i18n ("Unnamed");
	if (isModified ()) name.append (i18n (" [modified]"));

	setCaption (name);
}

void RKCommandEditorWindow::currentHelpContext (QString *symbol, QString *package) {
	RK_TRACE (COMMANDEDITOR);
	Q_UNUSED (package);

	if (m_view->selection()) {
		*symbol = m_view->selectionText();
	} else {
		KTextEditor::Cursor c = m_view->cursorPosition();
		QString line = m_doc->line(c.line()) + ' ';
		*symbol = RKCommonFunctions::getCurrentSymbol(line, c.column());
	}
}

QString RKCommandEditorWindow::provideContext (int line_rev) {
	RK_TRACE (COMMANDEDITOR);

	KTextEditor::Cursor c = m_view->cursorPosition();
	int current_line_num=c.line(); int cursor_pos=c.column();

	if (line_rev > current_line_num) return QString ();

	QString ret = m_doc->line (current_line_num - line_rev);
	if (line_rev == 0) ret = ret.left (cursor_pos);
	return ret;
}

void RKCommandEditorWindow::paste (const QString& text) {
	RK_TRACE (COMMANDEDITOR);

	m_view->insertText (text);
}

void RKCommandEditorWindow::setWDToScript () {
	RK_TRACE (COMMANDEDITOR);

	RK_ASSERT (!url ().isEmpty ());
	QString dir = url ().adjusted (QUrl::RemoveFilename).path ();
#ifdef Q_OS_WIN
	// KURL::directory () returns a leading slash on windows as of KDElibs 4.3
	while (dir.startsWith ('/')) dir.remove (0, 1);
#endif
	RKConsole::pipeUserCommand ("setwd (\"" + dir + "\")");
}

void RKCommandEditorWindow::runCurrent () {
	RK_TRACE (COMMANDEDITOR);

	if (m_view->selection ()) {
		RKConsole::pipeUserCommand (m_view->selectionText ());
	} else {
		KTextEditor::Cursor c = m_view->cursorPosition();
		QString command = m_doc->line (c.line());
		if (!command.isEmpty ()) RKConsole::pipeUserCommand (command + '\n');

		// advance to next line (NOTE: m_view->down () won't work on auto-wrapped lines)
		c.setLine(c.line() + 1);
		m_view->setCursorPosition (c);
	}
}

void RKCommandEditorWindow::enterAndSubmit () {
	RK_TRACE (COMMANDEDITOR);

	KTextEditor::Cursor c = m_view->cursorPosition ();
	int line = c.line ();
	m_doc->insertText (c, "\n");
	QString command = m_doc->line (line);
	if (!command.isEmpty ()) RKConsole::pipeUserCommand (command + '\n');
}

void RKCommandEditorWindow::copyLinesToOutput () {
	RK_TRACE (COMMANDEDITOR);

	RKCommandHighlighter::copyLinesToOutput (m_view, RKCommandHighlighter::RScript);
}

void RKCommandEditorWindow::runAll () {
	RK_TRACE (COMMANDEDITOR);

	QString command = m_doc->text ();
	if (command.isEmpty ()) return;

	RKConsole::pipeUserCommand (command);
}

void RKCommandEditorWindow::runBlock(int index) {
	RK_TRACE (COMMANDEDITOR);

	clearUnusedBlocks ();	// this block might have been removed meanwhile
	RK_ASSERT ((index >= 0) && (index < block_records.size ()));
	if (block_records[index].active) {
		QString command = m_doc->text (*(block_records[index].range));
		if (command.isEmpty ()) return;
	
		RKConsole::pipeUserCommand (command);
	}
}

void RKCommandEditorWindow::markBlock(int index) {
	RK_TRACE (COMMANDEDITOR);

	RK_ASSERT ((index >= 0) && (index < block_records.size ()));
	if (m_view->selection ()) {
		addBlock (index, m_view->selectionRange ());
	} else {
		RK_ASSERT (false);
	}
}

void RKCommandEditorWindow::unmarkBlock(int index) {
	RK_TRACE (COMMANDEDITOR);

	RK_ASSERT ((index >= 0) && (index < block_records.size ()));
	removeBlock (index);
}

void RKCommandEditorWindow::clearUnusedBlocks () {
	RK_TRACE (COMMANDEDITOR);

	for (int i = 0; i < block_records.size (); ++i) {
		if (block_records[i].active) {
// TODO: do we need to check whether the range was deleted? Does the katepart do such evil things?
			if (block_records[i].range->isEmpty ()) {
				removeBlock (i, true);
			}
		}
	}
}

void RKCommandEditorWindow::addBlock (int index, const KTextEditor::Range& range) {
	RK_TRACE (COMMANDEDITOR);
	RK_ASSERT ((index >= 0) && (index < block_records.size ()));

	clearUnusedBlocks ();
	removeBlock (index);

	KTextEditor::MovingRange* srange = m_doc->newMovingRange (range);
	srange->setInsertBehaviors (KTextEditor::MovingRange::ExpandRight);

	QString actiontext = i18n ("%1 (Active)", index + 1);
	block_records[index].range = srange;
	srange->setAttribute (block_records[index].attribute);
	block_records[index].active = true;
	block_records[index].mark->setText (actiontext);
	block_records[index].unmark->setText (actiontext);
	block_records[index].unmark->setEnabled (true);
	block_records[index].run->setText (actiontext);
	block_records[index].run->setEnabled (true);
}

void RKCommandEditorWindow::removeBlock (int index, bool was_deleted) {
	RK_TRACE (COMMANDEDITOR);
	RK_ASSERT ((index >= 0) && (index < block_records.size ()));

	if (!was_deleted) {
		delete (block_records[index].range);
	}

	QString actiontext = i18n ("%1 (Unused)", index + 1);
	block_records[index].range = nullptr;
	block_records[index].active = false;
	block_records[index].mark->setText (actiontext);
	block_records[index].unmark->setText (actiontext);
	block_records[index].unmark->setEnabled (false);
	block_records[index].run->setText (actiontext);
	block_records[index].run->setEnabled (false);
}

void RKCommandEditorWindow::selectionChanged (KTextEditor::View* view) {
	RK_TRACE (COMMANDEDITOR);
	RK_ASSERT (view == m_view);

	if (view->selection ()) {
		actionmenu_mark_block->setEnabled (true);
	} else {
		actionmenu_mark_block->setEnabled (false);
	}
}

// static
KTextEditor::Document* RKCommandHighlighter::_doc = nullptr;
KTextEditor::View* RKCommandHighlighter::_view = nullptr;
KTextEditor::Document* RKCommandHighlighter::getDoc () {
	if (_doc) return _doc;

	RK_TRACE (COMMANDEDITOR);
	KTextEditor::Editor* editor = KTextEditor::Editor::instance ();
	RK_ASSERT (editor);

	_doc = editor->createDocument (RKWardMainWindow::getMain ());
// NOTE: A (dummy) view is needed to access highlighting attributes.
	_view = _doc->createView(nullptr);
	_view->hide ();
	RK_ASSERT (_doc);
	return _doc;
}

KTextEditor::View* RKCommandHighlighter::getView () {
	if (!_view) getDoc ();
	return _view;
}

#include <QTextDocument>
#include "rkhtmlwindow.h"
#include "rkhtmlwindow.h"
#include "rkhtmlwindow.h"

//////////
// NOTE: Most of the exporting code is copied from the katepart HTML exporter plugin more or less verbatim! (Source license: LGPL v2)
//////////
QString exportText(const QString& text, const KTextEditor::Attribute::Ptr& attrib, const KTextEditor::Attribute::Ptr& m_defaultAttribute) {
	if ( !attrib || !attrib->hasAnyProperty() || attrib == m_defaultAttribute ) {
		return (text.toHtmlEscaped());
	}

	QString ret;
	if ( attrib->fontBold() ) {
		ret.append ("<b>");
	}
	if ( attrib->fontItalic() ) {
		ret.append ("<i>");
	}

	bool writeForeground = attrib->hasProperty(QTextCharFormat::ForegroundBrush)
		&& (!m_defaultAttribute || attrib->foreground().color() != m_defaultAttribute->foreground().color());
	bool writeBackground = attrib->hasProperty(QTextCharFormat::BackgroundBrush)
		&& (!m_defaultAttribute || attrib->background().color() != m_defaultAttribute->background().color());

	if ( writeForeground || writeBackground ) {
		ret.append (QString("<span style='%1%2'>")
					.arg(writeForeground ? QString(QLatin1String("color:") + attrib->foreground().color().name() + QLatin1Char(';')) : QString(),
					     writeBackground ? QString(QLatin1String("background:") + attrib->background().color().name() + QLatin1Char(';')) : QString()));
	}

	ret.append (text.toHtmlEscaped());

	if ( writeBackground || writeForeground ) {
		ret.append ("</span>");
	}
	if ( attrib->fontItalic() ) {
		ret.append ("</i>");
	}
	if ( attrib->fontBold() ) {
		ret.append ("</b>");
	}

	return ret;
}

QString RKCommandHighlighter::commandToHTML (const QString &r_command, HighlightingMode mode) {
	KTextEditor::Document* doc = getDoc ();
	KTextEditor::View* view = getView ();
	doc->setText (r_command);
	if (r_command.endsWith ('\n')) doc->removeLine (doc->lines () - 1);
	setHighlighting (doc, mode);
	QString ret;

	QString opening;
	KTextEditor::Attribute::Ptr m_defaultAttribute = view->defaultStyleAttribute (KSyntaxHighlighting::Theme::TextStyle::Normal);
	if ( !m_defaultAttribute ) {
		opening = "<pre class=\"%3\">";
	} else {
		opening = QString("<pre style='%1%2' class=\"%3\">")
				.arg(m_defaultAttribute->fontBold() ? "font-weight:bold;" : "",
				     m_defaultAttribute->fontItalic() ? "text-style:italic;" : "");
				// Note: copying the default text/background colors is pointless in our case, and leads to subtle inconsistencies.
	}

	const KTextEditor::Attribute::Ptr noAttrib(nullptr);

	if (mode == RScript) ret = opening.arg ("code");
	enum {
		Command,
		Output,
		Warning,
		None
	} previous_chunk = None;
	for (int i = 0; i < doc->lines (); ++i)
	{
		const QString &line = doc->line(i);
		const QList<KTextEditor::AttributeBlock> attribs = view->lineAttributes(i);
		int lineStart = 0;

		if (mode == RInteractiveSession) {
			if (line.startsWith ("> ") || line.startsWith ("+ ")) {
				lineStart = 2;	// skip the prompt. Prompt will be indicated by the CSS, instead
				if (previous_chunk != Command) {
					if (previous_chunk != None) ret.append ("</pre>");
					ret.append (opening.arg ("code"));
					previous_chunk = Command;
				}
			} else {
				if (previous_chunk != Output) {
					if (previous_chunk != None) ret.append ("</pre>");
					ret.append (opening.arg ("output_normal"));
					previous_chunk = Output;
				}
				ret.append (line.toHtmlEscaped () + '\n');	// don't copy output "highlighting". It is set using CSS, instead
				continue;
			}
		}

		int handledUntil = lineStart;
		int remainingChars = line.length();
		for ( const KTextEditor::AttributeBlock& block : attribs ) {
			if ((block.start + block.length) <= handledUntil) continue;
			int start = qMax(block.start, lineStart);
			if ( start > handledUntil ) {
				ret += exportText( line.mid( handledUntil, start - handledUntil ), noAttrib, m_defaultAttribute );
			}
			int end = qMin (block.start + block.length, remainingChars);
			int length = end - start;
			ret += exportText( line.mid( start, length ), block.attribute, m_defaultAttribute);
			handledUntil = end;
		}

		if ( handledUntil < lineStart + remainingChars ) {
			ret += exportText( line.mid( handledUntil, remainingChars ), noAttrib, m_defaultAttribute );
		}

		if (i < (doc->lines () - 1)) ret.append ("\n");
	}
	ret.append ("</pre>\n");

	return ret;
}

/** set syntax highlighting-mode to R syntax. Outside of class, in order to allow use from the on demand code highlighter */
void RKCommandHighlighter::setHighlighting (KTextEditor::Document *doc, HighlightingMode mode) {
	RK_TRACE (COMMANDEDITOR);

	QString mode_string;
	if (mode == RScript) mode_string = "R Script";
	else if (mode == RInteractiveSession) mode_string = "R interactive session";
	else {
		QString fn = doc->url ().fileName ().toLower ();
		if (fn.endsWith (QLatin1String (".pluginmap")) || fn.endsWith (QLatin1String (".rkh")) || fn.endsWith (QLatin1String (".xml")) || fn.endsWith (QLatin1String (".inc"))) {
			mode_string = "XML";
		} else if (fn.endsWith (QLatin1String (".js"))) {
			mode_string = "JavaScript";
		} else {
			return;
		}
	}
	if (!(doc->setHighlightingMode (mode_string) && doc->setMode (mode_string))) RK_DEBUG (COMMANDEDITOR, DL_ERROR, "R syntax highlighting definition ('%s')not found!", qPrintable (mode_string));
}

void RKCommandHighlighter::copyLinesToOutput (KTextEditor::View *view, HighlightingMode mode) {
	RK_TRACE (COMMANDEDITOR);

	// expand selection to full lines (or current line)
	KTextEditor::Document *doc = view->document ();
	KTextEditor::Range sel = view->selectionRange ();
	if (!sel.isValid ()) {
		KTextEditor::Cursor pos = view->cursorPosition ();
		sel.setRange (KTextEditor::Cursor (pos.line (), 0),
					  KTextEditor::Cursor (pos.line (), doc->lineLength (pos.line ())));
	} else {
		sel.setRange (KTextEditor::Cursor (sel.start ().line (), 0),
					  KTextEditor::Cursor (sel.end ().line (), doc->lineLength (sel.end ().line ())));
	}

	// highlight and submit
	QString highlighted = commandToHTML (doc->text (sel), mode);
	if (!highlighted.isEmpty ()) {
		RInterface::issueCommand (".rk.cat.output (" + RObject::rQuote (highlighted) + ")\n", RCommand::App | RCommand::Silent);
	}
}


