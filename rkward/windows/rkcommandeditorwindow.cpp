/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2004-2020 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "rkcommandeditorwindow.h"

#include <kxmlguifactory.h>

#include <ktexteditor/editor.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/markinterface.h>
#include <KTextEditor/Application>

#include <qapplication.h>
#include <qfile.h>
#include <qtimer.h>
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
#include <kio/deletejob.h>
#include <kio/job.h>
#include <kconfiggroup.h>
#include <krandom.h>

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
#include "../rkconsole.h"
#include "../rkglobals.h"
#include "../rkward.h"
#include "rkhelpsearchwindow.h"
#include "rkhtmlwindow.h"
#include "rkworkplace.h"
#include "katepluginintegration.h"
#include "rkcodecompletion.h"

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
	if (with_signals) {
		emit KTextEditor::Editor::instance()->application()->aboutToCreateDocuments();
	}
	KTextEditor::Document* ret = KTextEditor::Editor::instance()->createDocument (RKWardMainWindow::getMain ());
	if (with_signals) {
		emit KTextEditor::Editor::instance()->application()->documentCreated(ret);
		emit KTextEditor::Editor::instance()->application()->documentsCreated(QList<KTextEditor::Document*>() << ret);
	}
	return ret;
}

RKCommandEditorWindow::RKCommandEditorWindow (QWidget *parent, const QUrl _url, const QString& encoding, int flags) : RKMDIWindow (parent, RKMDIWindow::CommandEditorWindow) {
	RK_TRACE (COMMANDEDITOR);

	QString id_header = QStringLiteral ("unnamedscript://");

	KTextEditor::Editor* editor = KTextEditor::Editor::instance ();
	RK_ASSERT (editor);

	QUrl url = _url;
	m_doc = 0;
	preview_dir = 0;
	visible_to_kateplugins = flags & RKCommandEditorFlags::VisibleToKTextEditorPlugins;
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
				KMessageBox::messageBox (this, KMessageBox::Error, i18n ("Unable to open \"%1\"", url.toDisplayString ()), i18n ("Could not open command file"));
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
	KTextEditor::ModificationInterface* em_iface = qobject_cast<KTextEditor::ModificationInterface*> (m_doc);
	if (em_iface) em_iface->setModifiedOnDiskWarning (true);
	else RK_ASSERT (false);
	m_view = m_doc->createView (this, RKWardMainWindow::getMain ()->katePluginIntegration ()->mainWindow ()->mainWindow());
	if (visible_to_kateplugins) {
		emit RKWardMainWindow::getMain ()->katePluginIntegration ()->mainWindow ()->mainWindow()->viewCreated (m_view);
	}
	preview = new RKXMLGUIPreviewArea (QString(), this);
	preview_manager = new RKPreviewManager (this);
	connect (preview_manager, &RKPreviewManager::statusChanged, [this]() { preview_timer.start (500); });
	m_view = m_doc->createView (this);
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
	initializeActivationSignals ();
	RKXMLGUISyncer::self()->registerChangeListener (m_view, this, SLOT (fixupPartGUI()));

	QHBoxLayout *layout = new QHBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	QSplitter* preview_splitter = new QSplitter (this);
	preview_splitter->addWidget (m_view);
	QWidget *preview_widget = preview->wrapperWidget ();
	preview_splitter->addWidget (preview_widget);
	preview_widget->hide ();
	connect (m_doc, &KTextEditor::Document::documentSavedOrUploaded, this, &RKCommandEditorWindow::documentSaved);
	layout->addWidget(preview_splitter);

	setGlobalContextProperty ("current_filename", m_doc->url ().url ());
	connect (m_doc, &KTextEditor::Document::documentUrlChanged, [this]() { updateCaption(); setGlobalContextProperty ("current_filename", m_doc->url ().url ()); });
	connect (m_doc, &KTextEditor::Document::modifiedChanged, this, &RKCommandEditorWindow::updateCaption);                // of course most of the time this causes a redundant call to updateCaption. Not if a modification is undone, however.
#ifdef __GNUC__
#warning remove this in favor of KTextEditor::Document::restore()
#endif
	connect (m_doc, &KTextEditor::Document::modifiedChanged, this, &RKCommandEditorWindow::autoSaveHandlerModifiedChanged);
	connect (m_doc, &KTextEditor::Document::textChanged, this, &RKCommandEditorWindow::textChanged);
	connect (m_view, &KTextEditor::View::selectionChanged, this, &RKCommandEditorWindow::selectionChanged);
	// somehow the katepart loses the context menu each time it loses focus
	connect (m_view, &KTextEditor::View::focusIn, this, &RKCommandEditorWindow::focusIn);

	hinter = 0;
	if (use_r_highlighting) {
		RKCommandHighlighter::setHighlighting (m_doc, RKCommandHighlighter::RScript);
		if (flags & RKCommandEditorFlags::UseCodeHinting) {
			new RKCompletionManager (m_view);
			//hinter = new RKFunctionArgHinter (this, m_view);
		}
	} else {
		RKCommandHighlighter::setHighlighting (m_doc, RKCommandHighlighter::Automatic);
	}

	smart_iface = qobject_cast<KTextEditor::MovingInterface*> (m_doc);
	initBlocks ();
	RK_ASSERT (smart_iface);

	connect (&autosave_timer, &QTimer::timeout, this, &RKCommandEditorWindow::doAutoSave);
	connect (&preview_timer, &QTimer::timeout, this, &RKCommandEditorWindow::doRenderPreview);

	updateCaption ();	// initialize
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

	delete hinter;
	discardPreview ();
	delete m_view;
	QList<KTextEditor::View*> views = m_doc->views ();
	if (views.isEmpty ()) {
		if (visible_to_kateplugins) {
			emit KTextEditor::Editor::instance()->application()->documentWillBeDeleted(m_doc);
			emit KTextEditor::Editor::instance()->application()->aboutToDeleteDocuments(QList<KTextEditor::Document*>() << m_doc);
		}
		delete m_doc;
		if (visible_to_kateplugins) {
			emit KTextEditor::Editor::instance()->application()->documentDeleted(m_doc);
			emit KTextEditor::Editor::instance()->application()->documentsDeleted(QList<KTextEditor::Document*>() << m_doc);
		}
		if (!delete_on_close.isEmpty ()) KIO::del (delete_on_close)->start ();
		unnamed_documents.remove (_id);
	}
	// NOTE, under rather unlikely circumstances, the above may leave stale ids->stale pointers in the map: Create unnamed window, split it, save to a url, split again, close the first two windows, close the last. This situation should be caught by the following, however:
	if (have_url && !_id.isEmpty ()) {
		unnamed_documents.remove (_id);
	}
}

void RKCommandEditorWindow::fixupPartGUI () {
	RK_TRACE (COMMANDEDITOR);

	// strip down the katepart's GUI. remove some stuff we definitely don't need.
	RKCommonFunctions::removeContainers (m_view, QString ("bookmarks,tools_spelling,tools_spelling_from_cursor,tools_spelling_selection,switch_to_cmd_line").split (','), true);
	RKCommonFunctions::moveContainer (m_view, "Menu", "tools", "edit", true);
}

QAction *findAction (KTextEditor::View* view, const QString &actionName) {
	// katepart has more than one actionCollection
	QList<KActionCollection*> acs = view->findChildren<KActionCollection*>();
	acs.append (view->actionCollection ());

	foreach (KActionCollection* ac, acs) {
		QAction* found = ac->action (actionName);
		if (found) return found;
	}

	return 0;
}

void RKCommandEditorWindow::initializeActions (KActionCollection* ac) {
	RK_TRACE (COMMANDEDITOR);

	RKStandardActions::copyLinesToOutput (this, this, SLOT (copyLinesToOutput()));
	RKStandardActions::pasteSpecial (this, this, SLOT (paste(QString)));

	action_run_all = RKStandardActions::runAll (this, this, SLOT (runAll()));
	action_run_current = RKStandardActions::runCurrent (this, this, SLOT (runCurrent()), true);
	// NOTE: enter_and_submit is not currently added to the menu
	QAction *action = ac->addAction ("enter_and_submit", this, SLOT (enterAndSubmit()));
	action->setText (i18n ("Insert line break and run"));
	ac->setDefaultShortcuts (action, QList<QKeySequence>() << Qt::AltModifier + Qt::Key_Return << Qt::AltModifier + Qt::Key_Enter);
	ac->setDefaultShortcut (action, Qt::AltModifier + Qt::Key_Return); // KF5 TODO: This line needed only for KF5 < 5.2, according to documentation

	RKStandardActions::functionHelp (this, this);
	RKStandardActions::onlineHelp (this, this);

	actionmenu_run_block = new KActionMenu (i18n ("Run block"), this);
	actionmenu_run_block->setDelayed (false);	// KDE4: TODO does not work correctly in the tool bar.
	ac->addAction ("run_block", actionmenu_run_block);
	connect (actionmenu_run_block->menu(), &QMenu::aboutToShow, this, &RKCommandEditorWindow::clearUnusedBlocks);
	actionmenu_mark_block = new KActionMenu (i18n ("Mark selection as block"), this);
	ac->addAction ("mark_block", actionmenu_mark_block);
	connect (actionmenu_mark_block->menu(), &QMenu::aboutToShow, this, &RKCommandEditorWindow::clearUnusedBlocks);
	actionmenu_unmark_block = new KActionMenu (i18n ("Unmark block"), this);
	ac->addAction ("unmark_block", actionmenu_unmark_block);
	connect (actionmenu_unmark_block->menu(), &QMenu::aboutToShow, this, &RKCommandEditorWindow::clearUnusedBlocks);

	action_setwd_to_script = ac->addAction ("setwd_to_script", this, SLOT (setWDToScript()));
	action_setwd_to_script->setText (i18n ("CD to script directory"));
	action_setwd_to_script->setStatusTip (i18n ("Change the working directory to the directory of this script"));
	action_setwd_to_script->setToolTip (action_setwd_to_script->statusTip ());
	action_setwd_to_script->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionCDToScript));

	KActionMenu* actionmenu_preview = new KActionMenu (QIcon::fromTheme ("view-preview"), i18n ("Preview"), this);
	actionmenu_preview->setDelayed (false);
	preview_modes = new QActionGroup (this);
	actionmenu_preview->addAction (action_no_preview = new QAction (RKStandardIcons::getIcon (RKStandardIcons::ActionDelete), i18n ("No preview"), preview_modes));
	actionmenu_preview->addAction (new QAction (QIcon::fromTheme ("preview_math"), i18n ("R Markdown"), preview_modes));
	actionmenu_preview->addAction (new QAction (RKStandardIcons::getIcon (RKStandardIcons::WindowOutput), i18n ("RKWard Output"), preview_modes));
	actionmenu_preview->addAction (new QAction (RKStandardIcons::getIcon (RKStandardIcons::WindowX11), i18n ("Plot"), preview_modes));
	actionmenu_preview->addAction (new QAction (RKStandardIcons::getIcon (RKStandardIcons::WindowConsole), i18n ("R Console"), preview_modes));
	QList<QAction*> preview_actions = preview_modes->actions ();
	preview_actions[NoPreview]->setToolTip (i18n ("Disable preview"));
	preview_actions[RMarkdownPreview]->setToolTip (i18n ("Preview the script as rendered from RMarkdown format (appropriate for .Rmd files)."));
	preview_actions[ConsolePreview]->setToolTip (i18n ("Preview the script as if it was run in the interactive R Console"));
	preview_actions[GraphPreview]->setToolTip (i18n ("Preview any onscreen graphics produced by running this script. This preview will be empty, if there is no call to <i>plot()</i> or other graphics commands."));
	preview_actions[OutputWindow]->setToolTip (i18n ("Preview any output to the RKWard Output Window. This preview will be empty, if there is no call to <i>rk.print()</i> or other RKWard output commands."));
	for (int i = 0; i < preview_actions.size (); ++i) {
		preview_actions[i]->setCheckable (true);
		preview_actions[i]->setStatusTip (preview_actions[i]->toolTip ());
	}
	action_no_preview->setChecked (true);
	connect (preview, &RKXMLGUIPreviewArea::previewClosed, this, &RKCommandEditorWindow::discardPreview);
	connect (preview_modes, &QActionGroup::triggered, this, &RKCommandEditorWindow::changePreviewMode);

	actionmenu_preview->addSeparator ();
	action_preview_as_you_type = new QAction (QIcon::fromTheme ("input-keyboard"), i18nc ("Checkable action: the preview gets updated while typing", "Update as you type"), ac);
	action_preview_as_you_type->setToolTip (i18n ("When this option is enabled, an update of the preview will be triggered every time you modify the script. When this option is disabled, the preview will be updated whenever you save the script, only."));
	action_preview_as_you_type->setCheckable (true);
	action_preview_as_you_type->setChecked (m_doc->url ().isEmpty ());  // By default, update as you type for unsaved "quick and dirty" scripts, preview on save for saved scripts
	actionmenu_preview->addAction (action_preview_as_you_type);
	ac->addAction ("render_preview", actionmenu_preview);

	file_save = findAction (m_view, "file_save");
	if (file_save) file_save->setText (i18n ("Save Script..."));
	file_save_as = findAction (m_view, "file_save_as");
	if (file_save_as) file_save_as->setText (i18n ("Save Script As..."));
}

void RKCommandEditorWindow::initBlocks () {
	RK_TRACE (COMMANDEDITOR);
	if (!smart_iface) return;	// may happen in KDE => 4.6 if compiled with KDE <= 4.4
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

		record.mark = ac->addAction ("markblock" + QString::number (i), this, SLOT (markBlock()));
		record.mark->setIcon (icon);
		record.mark->setData (i);
		actionmenu_mark_block->addAction (record.mark);
		record.unmark = ac->addAction ("unmarkblock" + QString::number (i), this, SLOT (unmarkBlock()));
		record.unmark->setIcon (icon);
		record.unmark->setData (i);
		actionmenu_unmark_block->addAction (record.unmark);
		record.run = ac->addAction ("runblock" + QString::number (i), this, SLOT (runBlock()));
		ac->setDefaultShortcut (record.run, shortcuts[i]);
		record.run->setIcon (icon);
		record.run->setData (i);
		actionmenu_run_block->addAction (record.run);

		// these two not strictly needed due to removeBlock(), below. Silences a GCC warning, however.
		record.range = 0;
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
		int status = KMessageBox::warningYesNo (this, i18n ("The document \"%1\" has been modified. Close it anyway?", windowTitle ()), i18n ("File not saved"));
	
		if (status != KMessageBox::Yes) {
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

void RKCommandEditorWindow::changePreviewMode (QAction *mode) {
	RK_TRACE (COMMANDEDITOR);

	if (mode != action_no_preview) {
		if (!preview_dir) {  // triggered on change from no preview to some preview, but not between previews
			if (KMessageBox::warningContinueCancel (this, i18n ("<p>The preview feature <b>tries</b> to avoid making any lasting changes to your workspace (technically, by making use of a <i>local()</i> evaluation environment). <b>However, there are cases where using the preview feature may cause unexpected side-effects</b>.</p><p>In particular, this is the case for scripts that contain explicit assignments to <i>globalenv()</i>, or to scripts that alter files on your filesystem. Further, attaching/detaching packages or package namespaces will affect the entire running R session.</p><p>Please keep this in mind when using the preview feature, and especially when using the feature on scripts originating from untrusted sources.</p>"), i18n ("Potential side-effects of previews"), KStandardGuiItem::cont (), KStandardGuiItem::cancel (), QStringLiteral ("preview_side_effects")) != KMessageBox::Continue) {
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

	if (preview_dir) {
		preview->wrapperWidget ()->hide ();
		preview_manager->setPreviewDisabled ();
		RKGlobals::rInterface ()->issueCommand (QString (".rk.killPreviewDevice(%1)\nrk.discard.preview.data (%1)").arg (RObject::rQuote(preview_manager->previewId ())), RCommand::App | RCommand::Sync);
		delete preview_dir;
		preview_dir = 0;
		delete preview_input_file;
		preview_input_file = 0;
	}
	action_no_preview->setChecked (true);
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
	out.setCodec ("UTF-8");		// make sure that all characters can be saved, without nagging the user
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

bool RKCommandEditorWindow::isModified () {
	RK_TRACE (COMMANDEDITOR);
	return m_doc->isModified();
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
	KTextEditor::MarkInterface *markiface = qobject_cast<KTextEditor::MarkInterface*> (m_doc);
	if (markiface) markiface->clearMarks ();
	if (!old_rw) m_doc->setReadWrite (false);
}

void RKCommandEditorWindow::highlightLine (int linenum) {
	RK_TRACE (COMMANDEDITOR);

	KTextEditor::MarkInterface *markiface = qobject_cast<KTextEditor::MarkInterface*> (m_doc);
	if (!markiface) {
		RK_ASSERT (markiface);
		return;
	}
	bool old_rw = m_doc->isReadWrite ();
	if (!old_rw) m_doc->setReadWrite (true);
	markiface->addMark (linenum, KTextEditor::MarkInterface::Execution);
	m_view->setCursorPosition (KTextEditor::Cursor (linenum, 0));
	if (!old_rw) m_doc->setReadWrite (false);
}

void RKCommandEditorWindow::updateCaption () {
	RK_TRACE (COMMANDEDITOR);
	QString name = url ().fileName ();
	if (name.isEmpty ()) name = url ().toDisplayString ();
	if (name.isEmpty ()) name = i18n ("Unnamed");
	if (isModified ()) name.append (i18n (" [modified]"));

	setCaption (name);

	// Well, these do not really belong, here, but need to happen on pretty much the same occasions:
	action_setwd_to_script->setEnabled (!url ().isEmpty ());
	RKWardMainWindow::getMain ()->addScriptUrl (url ());
}

void RKCommandEditorWindow::currentHelpContext (QString *symbol, QString *package) {
	RK_TRACE (COMMANDEDITOR);
	Q_UNUSED (package);

	KTextEditor::Cursor c = m_view->cursorPosition();
	QString line = m_doc->line(c.line ()) + ' ';

	*symbol = RKCommonFunctions::getCurrentSymbol (line, c.column ());
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

void RKCommandEditorWindow::doRenderPreview () {
	RK_TRACE (COMMANDEDITOR);

	if (action_no_preview->isChecked ()) return;
	if (!preview_manager->needsCommand ()) return;
	int mode = preview_modes->actions ().indexOf (preview_modes->checkedAction ());

	if (!preview_dir) {
		preview_dir = new QTemporaryDir ();
		preview_input_file = 0;
	}
	if (preview_input_file) {
		// When switching between .Rmd and .R previews, discard input file
		if ((mode == RMarkdownPreview) != (preview_input_file->fileName().endsWith (".Rmd"))) {
			delete preview_input_file;
			preview_input_file = 0;
		} else {
			preview_input_file->remove ();  // If re-using an existing filename, remove it first. Somehow, contrary to documentation, this does not happen in open(WriteOnly), below.
		}
	}
	if (!preview_input_file) { // NOT an else!
		if (m_doc->url ().isEmpty () || !m_doc->url ().isLocalFile ()) {
			preview_input_file = new QFile (QDir (preview_dir->path()).absoluteFilePath (mode == RMarkdownPreview ? "script.Rmd" : "script.R"));
		} else {
			// If the file is already saved, save the preview input as a temp file in the same folder.
			// esp. .Rmd files might try to include other files by relative path.
			QString tempfiletemplate = m_doc->url ().toLocalFile ();
			tempfiletemplate.append ("_XXXXXX.rkward_preview.R");
			if (mode == RMarkdownPreview) tempfiletemplate.append ("md");
			preview_input_file = new QTemporaryFile (tempfiletemplate);
		}
	}

	QString output_file = QDir (preview_dir->path()).absoluteFilePath ("output.html");  // NOTE: not needed by all types of preview

	if (mode != GraphPreview && !preview->findChild<RKMDIWindow*>()) {
		// (lazily) initialize the preview window with _something_, as an RKMDIWindow is needed to display messages (important, if there is an error during the first preview)
		RKGlobals::rInterface()->issueCommand (".rk.with.window.hints (rk.show.html(" + RObject::rQuote (output_file) + "), \"\", " + RObject::rQuote (preview_manager->previewId ()) + ", style=\"preview\")", RCommand::App | RCommand::Sync);
	}

	RK_ASSERT (preview_input_file->open (QIODevice::WriteOnly));
	QTextStream out (preview_input_file);
	out.setCodec ("UTF-8");     // make sure that all characters can be saved, without nagging the user
	out << m_doc->text ();
	preview_input_file->close ();

	QString command;
	if (mode == RMarkdownPreview) {
		preview->setLabel (i18n ("Preview of rendered R Markdown"));

		command = "if (!nzchar(Sys.which(\"pandoc\"))) {\n"
		           "	output <- rk.set.output.html.file(%2, silent=TRUE)\n"
		           "	rk.header (" + RObject::rQuote (i18n ("Pandoc is not installed")) + ")\n"
		           "	rk.print (" + RObject::rQuote (i18n ("The software <tt>pandoc</tt>, required to rendering R markdown files, is not installed, or not in the system path of "
		                         "the running R session. You will need to install pandoc from <a href=\"https://pandoc.org/\">https://pandoc.org/</a>.</br>"
		                         "If it is installed, but cannot be found, try adding it to the system path of the running R session at "
		                         "<a href=\"rkward://settings/rbackend\">Settings->Configure RKward->R-backend</a>.")) + ")\n"
		           "	rk.set.output.html.file(output, silent=TRUE)\n"
		           "} else {\n"
		           "	require(rmarkdown)\n"
		           "	rmarkdown::render (%1, output_format=\"html_document\", output_file=%2, quiet=TRUE)\n"
		           "}\n"
		           "rk.show.html(%2)\n";
		command = command.arg (RObject::rQuote (preview_input_file->fileName ()), RObject::rQuote (output_file));
	} else if (mode == RKOutputPreview) {
		preview->setLabel (i18n ("Preview of generated RKWard output"));
		command = "output <- rk.set.output.html.file(%2, silent=TRUE)\n"
		          "try(rk.flush.output(ask=FALSE, style=\"preview\", silent=TRUE))\n"
		          "try(source(%1, local=TRUE))\n"
		          "rk.set.output.html.file(output, silent=TRUE)\n"
		          "rk.show.html(%2)\n";
		command = command.arg (RObject::rQuote (preview_input_file->fileName ()), RObject::rQuote (output_file));
	} else if (mode == GraphPreview) {
		preview->setLabel (i18n ("Preview of generated plot"));
		command = "olddev <- dev.cur()\n"
		          ".rk.startPreviewDevice(%2)\n"
		          "try(source(%1, local=TRUE))\n"
		          "if (olddev != 1) dev.set(olddev)\n";
		command = command.arg (RObject::rQuote (preview_input_file->fileName ()), RObject::rQuote (preview_manager->previewId ()));
	} else if (mode == ConsolePreview) {
		preview->setLabel (i18n ("Preview of script running in interactive R Console"));
		command = "output <- rk.set.output.html.file(%2, silent=TRUE)\n"
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
		          "rk.show.html(%2)\n";
		command = command.arg (RObject::rQuote (preview_input_file->fileName ())).arg (RObject::rQuote (output_file));
	} else {
		RK_ASSERT (false);
	}

	preview->wrapperWidget ()->show ();

	RCommand *rcommand = new RCommand (".rk.with.window.hints (local ({\n" + command + QStringLiteral ("}), \"\", ") + RObject::rQuote (preview_manager->previewId ()) + ", style=\"preview\")", RCommand::App);
	preview_manager->setCommand (rcommand);
}

void RKCommandEditorWindow::runAll () {
	RK_TRACE (COMMANDEDITOR);

	QString command = m_doc->text ();
	if (command.isEmpty ()) return;

	RKConsole::pipeUserCommand (command);
}

void RKCommandEditorWindow::runBlock () {
	RK_TRACE (COMMANDEDITOR);

	QAction* action = qobject_cast<QAction*>(sender ());
	if (!action) {
		RK_ASSERT (false);
		return;
	}

	clearUnusedBlocks ();	// this block might have been removed meanwhile
	int index = action->data ().toInt ();
	RK_ASSERT ((index >= 0) && (index < block_records.size ()));
	if (block_records[index].active) {
		QString command = m_doc->text (*(block_records[index].range));
		if (command.isEmpty ()) return;
	
		RKConsole::pipeUserCommand (command);
	}
}

void RKCommandEditorWindow::markBlock () {
	RK_TRACE (COMMANDEDITOR);

	QAction* action = qobject_cast<QAction*>(sender ());
	if (!action) {
		RK_ASSERT (false);
		return;
	}

	int index = action->data ().toInt ();
	RK_ASSERT ((index >= 0) && (index < block_records.size ()));
	if (m_view->selection ()) {
		addBlock (index, m_view->selectionRange ());
	} else {
		RK_ASSERT (false);
	}
}

void RKCommandEditorWindow::unmarkBlock () {
	RK_TRACE (COMMANDEDITOR);

	QAction* action = qobject_cast<QAction*>(sender ());
	if (!action) {
		RK_ASSERT (false);
		return;
	}

	int index = action->data ().toInt ();
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
	if (!smart_iface) return;	// may happen in KDE => 4.6 if compiled with KDE <= 4.4
	RK_ASSERT ((index >= 0) && (index < block_records.size ()));

	clearUnusedBlocks ();
	removeBlock (index);

	KTextEditor::MovingRange* srange = smart_iface->newMovingRange (range);
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
	if (!smart_iface) return;	// may happen in KDE => 4.6 if compiled with KDE <= 4.4
	RK_ASSERT ((index >= 0) && (index < block_records.size ()));

	if (!was_deleted) {
		delete (block_records[index].range);
	}

	QString actiontext = i18n ("%1 (Unused)", index + 1);
	block_records[index].range = 0;
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

//////////////////////// RKFunctionArgHinter //////////////////////////////

#include <QToolTip>
#include <QStyle>

#include "../core/rfunctionobject.h"

RKFunctionArgHinter::RKFunctionArgHinter (RKScriptContextProvider *provider, KTextEditor::View* view) {
	RK_TRACE (COMMANDEDITOR);

	RKFunctionArgHinter::provider = provider;
	RKFunctionArgHinter::view = view;

	const QObjectList children = view->children ();
	for (QObjectList::const_iterator it = children.constBegin(); it != children.constEnd (); ++it) {
		QObject *obj = *it;
		obj->installEventFilter (this);
	}

	arghints_popup = new QLabel (0, Qt::ToolTip);
	arghints_popup->setMargin (2);
	// HACK trying hard to trick the style into using the correct color
	// ... and sometimes we still get white on yellow in some styles. Sigh...
	// A simple heuristic tries to detect the worst cases of unreasonably low contrast, and forces black on light blue, then.
	QPalette p = QToolTip::palette ();
	QColor b = p.color (QPalette::Inactive, QPalette::ToolTipBase);
	QColor f = p.color (QPalette::Inactive, QPalette::ToolTipText);
	if ((qAbs (f.greenF () - b.greenF ()) + qAbs (f.redF () -  b.redF ()) + qAbs (f.yellowF () - b.yellowF ())) < .6) {
		f = Qt::black;
		b = QColor (192, 219, 255);
	}
	p.setColor (QPalette::Inactive, QPalette::WindowText, f);
	p.setColor (QPalette::Inactive, QPalette::Window, b);
	p.setColor (QPalette::Inactive, QPalette::ToolTipText, f);
	p.setColor (QPalette::Inactive, QPalette::ToolTipBase, b);
	arghints_popup->setForegroundRole (QPalette::ToolTipText);
	arghints_popup->setBackgroundRole (QPalette::ToolTipBase);
	arghints_popup->setPalette (p);
	arghints_popup->setFrameStyle (QFrame::Box);
	arghints_popup->setLineWidth (1);
	arghints_popup->setWordWrap (true);
	arghints_popup->setWindowOpacity (arghints_popup->style ()->styleHint (QStyle::SH_ToolTipLabel_Opacity, 0, arghints_popup) / 255.0);
	arghints_popup->hide ();
	active = false;

	connect (&updater, &QTimer::timeout, this, &RKFunctionArgHinter::updateArgHintWindow);
}

RKFunctionArgHinter::~RKFunctionArgHinter () {
	RK_TRACE (COMMANDEDITOR);
	delete arghints_popup;
}

void RKFunctionArgHinter::tryArgHint () {
	RK_TRACE (COMMANDEDITOR);

	if (!RKSettingsModuleCommandEditor::completionSettings()->argHintingEnabled ()) return;

	// do this in the next event cycle to make sure any inserted characters have truly been inserted
	QTimer::singleShot (0, this, SLOT (tryArgHintNow()));
}

void RKFunctionArgHinter::tryArgHintNow () {
	RK_TRACE (COMMANDEDITOR);

	// find the active opening brace
	int line_rev = -1;
	QList<int> unclosed_braces;
	QString full_context;
	while (unclosed_braces.isEmpty ()) {
		QString context_line = provider->provideContext (++line_rev);
		if (context_line.isNull ()) break;
		full_context.prepend (context_line);
		for (int i = 0; i < context_line.length (); ++i) {
			QChar c = context_line.at (i);
			if (c == '"' || c == '\'' || c == '`') {  // NOTE: this algo does not produce good results on string constants spanning newlines.
				i = RKCommonFunctions::quoteEndPosition (c, context_line, i + 1);
				if (i < 0) break;
				continue;
			} else if (c == '\\') {
				++i;
				continue;
			} else if (c == '(') {
				unclosed_braces.append (i);
			} else if (c == ')') {
				if (!unclosed_braces.isEmpty()) unclosed_braces.pop_back ();
			}
		}
	}

	int potential_symbol_end = unclosed_braces.isEmpty () ? -1 : unclosed_braces.last () - 1;

	// now find out where the symbol to the left of the opening brace ends
	// there cannot be a line-break between the opening brace, and the symbol name (or can there?), so no need to fetch further context
	while ((potential_symbol_end >= 0) && full_context.at (potential_symbol_end).isSpace ()) {
		--potential_symbol_end;
	}
	if (potential_symbol_end <= 0) {
		hideArgHint ();
		return;
	}

	// now identify the symbol and object (if any)
	QString effective_symbol = RKCommonFunctions::getCurrentSymbol (full_context, potential_symbol_end);
	if (effective_symbol.isEmpty ()) {
		hideArgHint ();
		return;
	}

	RObject *object = RObjectList::getObjectList ()->findObject (effective_symbol);
	if ((!object) || (!object->isType (RObject::Function))) {
		hideArgHint ();
		return;
	}

	// initialize and show popup
	arghints_popup->setText (effective_symbol + " (" + static_cast<RFunctionObject*> (object)->printArgs () + ')');
	arghints_popup->resize (arghints_popup->sizeHint () + QSize (2, 2));
	active = true;
	updater.start (50);
	updateArgHintWindow ();
}

void RKFunctionArgHinter::updateArgHintWindow () {
	RK_TRACE (COMMANDEDITOR);

	if (!active) return;

	arghints_popup->move (view->mapToGlobal (view->cursorPositionCoordinates () + QPoint (0, arghints_popup->fontMetrics ().lineSpacing () + arghints_popup->margin ()*2)));
	if (view->hasFocus ()) arghints_popup->show ();
	else arghints_popup->hide ();
}

void RKFunctionArgHinter::hideArgHint () {
	RK_TRACE (COMMANDEDITOR);
	arghints_popup->hide ();
	active = false;
	updater.stop ();
}

bool RKFunctionArgHinter::eventFilter (QObject *, QEvent *e) {
	if (e->type () == QEvent::KeyPress || e->type () == QEvent::ShortcutOverride) {
		RK_TRACE (COMMANDEDITOR);	// avoid loads of empty traces, putting this here
		QKeyEvent *k = static_cast<QKeyEvent *> (e);

		if (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return || k->key () == Qt::Key_Up || k->key () == Qt::Key_Down || k->key () == Qt::Key_Left || k->key () == Qt::Key_Right || k->key () == Qt::Key_Home || k->key () == Qt::Key_Tab) {
			hideArgHint ();
		} else if (k->key () == Qt::Key_Backspace || k->key () == Qt::Key_Delete){
			tryArgHint ();
		} else {
			QString text = k->text ();
			if ((text == "(") || (text == ")") || (text == ",")) {
				tryArgHint ();
			}
		}
	}

	return false;
}

// static
KTextEditor::Document* RKCommandHighlighter::_doc = 0;
KTextEditor::View* RKCommandHighlighter::_view = 0;
KTextEditor::Document* RKCommandHighlighter::getDoc () {
	if (_doc) return _doc;

	RK_TRACE (COMMANDEDITOR);
	KTextEditor::Editor* editor = KTextEditor::Editor::instance ();
	RK_ASSERT (editor);

	_doc = editor->createDocument (RKWardMainWindow::getMain ());
// NOTE: A (dummy) view is needed to access highlighting attributes.
	_view = _doc->createView (0);
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
					.arg(writeForeground ? QString(QLatin1String("color:") + attrib->foreground().color().name() + QLatin1Char(';')) : QString())
					.arg(writeBackground ? QString(QLatin1String("background:") + attrib->background().color().name() + QLatin1Char(';')) : QString()));
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

QString RKCommandHighlighter::commandToHTML (const QString r_command, HighlightingMode mode) {
	KTextEditor::Document* doc = getDoc ();
	KTextEditor::View* view = getView ();
	doc->setText (r_command);
	if (r_command.endsWith ('\n')) doc->removeLine (doc->lines () - 1);
	setHighlighting (doc, mode);
	QString ret;

	QString opening;
	KTextEditor::Attribute::Ptr m_defaultAttribute = view->defaultStyleAttribute (KTextEditor::dsNormal);
	if ( !m_defaultAttribute ) {
		opening = "<pre class=\"%3\">";
	} else {
		opening = QString("<pre style='%1%2' class=\"%3\">")
				.arg(m_defaultAttribute->fontBold() ? "font-weight:bold;" : "")
				.arg(m_defaultAttribute->fontItalic() ? "text-style:italic;" : "");
				// Note: copying the default text/background colors is pointless in our case, and leads to subtle inconsistencies.
	}

	const KTextEditor::Attribute::Ptr noAttrib(0);

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
		QList<KTextEditor::AttributeBlock> attribs = view->lineAttributes(i);
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
		foreach ( const KTextEditor::AttributeBlock& block, attribs ) {
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
		RKGlobals::rInterface ()->issueCommand (".rk.cat.output (" + RObject::rQuote (highlighted) + ")\n", RCommand::App | RCommand::Silent);
	}
}


