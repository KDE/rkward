/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2004-2019 by Thomas Friedrichsmeier
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

RKCommandEditorWindow::RKCommandEditorWindow (QWidget *parent, const QUrl _url, const QString& encoding, bool use_r_highlighting, bool use_codehinting, bool read_only, bool delete_on_close) : RKMDIWindow (parent, RKMDIWindow::CommandEditorWindow) {
	RK_TRACE (COMMANDEDITOR);

	QString id_header = QStringLiteral ("unnamedscript://");

	KTextEditor::Editor* editor = KTextEditor::Editor::instance ();
	RK_ASSERT (editor);

	QUrl url = _url;
	m_doc = 0;
	preview_dir = 0;

	// Lookup of existing text editor documents: First, if no url is given at all, create a new document, and register an id, in case this window will get split, later
	if (url.isEmpty ()) {
		m_doc = editor->createDocument (RKWardMainWindow::getMain ());
		_id = id_header + KRandom::randomString (16).toLower ();
		RK_ASSERT (!unnamed_documents.contains (_id));
		unnamed_documents.insert (_id, m_doc);
	} else if (url.url ().startsWith (id_header)) { // Next, handle the case that a pseudo-url is passed in
		_id = url.url ();
		m_doc = unnamed_documents.value (_id);
		url.clear ();
		if (!m_doc) {  // can happen while restoring saved workplace.
			m_doc = editor->createDocument (RKWardMainWindow::getMain ());
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

	// if an existing document is re-used, try to honor decoding.
	if (m_doc) {
		if (!encoding.isEmpty () && (m_doc->encoding () != encoding)) {
			m_doc->setEncoding (encoding);
			m_doc->documentReload ();
		}
	}

	// no existing document was found, so create one and load the url
	if (!m_doc) {
		m_doc = editor->createDocument (RKWardMainWindow::getMain ()); // The document may have to outlive this window

		// encoding must be set *before* loading the file
		if (!encoding.isEmpty ()) m_doc->setEncoding (encoding);
		if (!url.isEmpty ()) {
			if (m_doc->openUrl (url)) {
				// KF5 TODO: Check which parts of this are still needed in KF5, and which no longer work
				if (!delete_on_close) {	// don't litter config with temporary files
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

	setReadOnly (read_only);

	if (delete_on_close) {
		if (read_only) {
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
		if (use_codehinting) {
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
	QTimer::singleShot (0, this, SLOT (setPopupMenu()));
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
		delete m_doc;
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

	setPopupMenu ();
}

/** NOTE: this function still needed?
- Still needed in KDE 4.3.4. */
void RKCommandEditorWindow::setPopupMenu () {
	RK_TRACE (COMMANDEDITOR);

	if (!getPart ()->factory ()) return;
	m_view->setContextMenu (static_cast<QMenu *> (getPart ()->factory ()->container ("ktexteditor_popup", getPart ())));
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
		          "for (i in 1:length (exprs)) {\n"
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

	if (!RKSettingsModuleCommandEditor::argHintingEnabled ()) return;

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

//////////////////////// RKCompletionManager //////////////////////

RKCompletionManager::RKCompletionManager (KTextEditor::View* view) : QObject (view) {
	RK_TRACE (COMMANDEDITOR);

	_view = view;
	active = false;
	update_call = true;
	cached_position = KTextEditor::Cursor (-1, -1);

	cc_iface = qobject_cast<KTextEditor::CodeCompletionInterface*> (view);
	if (cc_iface) {
		cc_iface->setAutomaticInvocationEnabled (false);
		completion_model = new RKCodeCompletionModel (this);
		file_completion_model = new RKFileCompletionModel (this);
		callhint_model = new RKCallHintModel (this);
		arghint_model = new RKArgumentHintModel (this);
		cc_iface->registerCompletionModel (callhint_model);
		completion_timer = new QTimer (this);
		completion_timer->setSingleShot (true);
		connect (completion_timer, &QTimer::timeout, this, &RKCompletionManager::tryCompletion);
		connect (view->document (), &KTextEditor::Document::textInserted, this, &RKCompletionManager::textInserted);
		connect (view->document (), &KTextEditor::Document::textRemoved, this, &RKCompletionManager::textRemoved);
		connect (view->document (), &KTextEditor::Document::lineWrapped, this, &RKCompletionManager::lineWrapped);
		connect (view->document (), &KTextEditor::Document::lineUnwrapped, this, &RKCompletionManager::lineUnwrapped);
		connect (view, &KTextEditor::View::cursorPositionChanged, this, &RKCompletionManager::cursorPositionChanged);
		const QObjectList children = _view->children ();
		for (QObjectList::const_iterator it = children.constBegin(); it != children.constEnd (); ++it) {
			(*it)->installEventFilter (this);  // to handle Tab-key; installing on the view, alone, is not enough.
		}

		// HACK: I just can't see to make the object name completion model play nice with automatic invocation.
		//       However, there is no official way to invoke all registered models, manually. So we try to hack our way
		//       to a pointer to the default kate keyword completion model
		kate_keyword_completion_model = KTextEditor::Editor::instance ()->findChild<KTextEditor::CodeCompletionModel *> ();
		if (!kate_keyword_completion_model) kate_keyword_completion_model = view->findChild<KTextEditor::CodeCompletionModel *> (QString());

	} else {
		RK_ASSERT (false);  // Not a katepart?
	}
}

RKCompletionManager::~RKCompletionManager () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCompletionManager::tryCompletionProxy () {
	if (RKSettingsModuleCommandEditor::completionEnabled ()) {
		if (active) {
			// Handle this in the next event cycle, as more than one event may trigger
			completion_timer->start (0);
			// TODO: Actually, in this case we should try to _update_ the existing completion, i.e. try to save some CPU cycles.
			tryCompletion ();
		}
		completion_timer->start (RKSettingsModuleCommandEditor::completionTimeout ());
	}
}

QString RKCompletionManager::currentCompletionWord () const {
	RK_TRACE (COMMANDEDITOR);

	if (symbol_range.isValid ()) return _view->document ()->text (symbol_range);
	return QString ();
}

void RKCompletionManager::tryCompletion () {
	// TODO: merge this with RKConsole::doTabCompletion () somehow
	RK_TRACE (COMMANDEDITOR);
	if (!cc_iface) {
		// NOTE: This should not be possible, because the connections  have not been set up in the constructor, in this case.
		RK_ASSERT (cc_iface);
		return;
	}

	KTextEditor::Document *doc = _view->document ();
	KTextEditor::Cursor c = _view->cursorPosition();
	cached_position = c;
	uint para=c.line(); int cursor_pos=c.column();

	QString current_line = doc->line (para);
	int start;
	int end;
	RKCommonFunctions::getCurrentSymbolOffset (current_line, cursor_pos-1, false, &start, &end);
	if (end > cursor_pos) {
		symbol_range = KTextEditor::Range (-1, -1, -1, -1);   // Only hint when at the end of a word/symbol: https://mail.kde.org/pipermail/rkward-devel/2015-April/004122.html
	} else if (current_line.lastIndexOf ("#", cursor_pos) >= 0) symbol_range = KTextEditor::Range ();	// do not hint while in comments
	else symbol_range = KTextEditor::Range (para, start, para, end);

	QString word = currentCompletionWord ();
	if (word.length () >= RKSettingsModuleCommandEditor::completionMinChars ()) {
		QString filename;
		// as a very simple heuristic: If the current symbol starts with a quote, we should probably attempt file name completion, instead of symbol name completion
		if (word.startsWith ('\"') || word.startsWith ('\'') || word.startsWith ('`')) {
			symbol_range.setStart (KTextEditor::Cursor (symbol_range.start ().line (), symbol_range.start ().column () + 1));   // confine range to inside the quotes.
			filename = word.mid (1);
			word.clear ();
		}

		completion_model->updateCompletionList (word);
		file_completion_model->updateCompletionList (filename);
	} else {
		completion_model->updateCompletionList (QString ());
		file_completion_model->updateCompletionList (QString ());
	}

	updateCallHint ();

	// update ArgHint.
	argname_range = KTextEditor::Range (-1, -1, -1, -1);
	// Named arguments are just like regular symbols, *but* we must require that they are preceeded by either a ',', or the opening '(', immediately.
	// Otherwise, they are an argument value expression, for sure.
	if (callhint_model->currentFunction ()) {
		for (int i = symbol_range.start ().column () - 1; i >= 0; --i) {
			QChar c = current_line.at (i);
			if (c == ',' || c == '(') {
				argname_range = symbol_range;
				break;
			} else if (!c.isSpace ()) {
				break;
			}
		}
	}
	arghint_model->updateCompletionList (callhint_model->currentFunction (), argname_range.isValid () ? doc->text (argname_range) : QString ());

	updateVisibility ();
}

void RKCompletionManager::updateCallHint () {
	RK_TRACE (COMMANDEDITOR);

	if (!update_call) return;
	update_call = false;

	int line = cached_position.line () + 1;
	QString full_context;
	int potential_symbol_end = -2;
	KTextEditor::Document *doc = _view->document ();
	while (potential_symbol_end < -1 && line >= 0) {
		--line;
		QString context_line = doc->line (line);
		if (context_line.startsWith ('>')) continue; // For console: TODO limit to console
		full_context.prepend (context_line);

		int pos = context_line.length () - 1;
		if (line == cached_position.line ()) pos = cached_position.column () - 1;
		for (int i = pos; i >= 0; --i) {
			QChar c = context_line.at (i);
			if (c == '(') {
				KTextEditor::DefaultStyle style = doc->defaultStyleAt (KTextEditor::Cursor (line, i));
				if (style != KTextEditor::dsComment && style != KTextEditor::dsString && style != KTextEditor::dsChar) {
					potential_symbol_end = i - 1;
					break;
				}
			}
		}
	}

	// now find out where the symbol to the left of the opening brace ends
	// there cannot be a line-break between the opening brace, and the symbol name (or can there?), so no need to fetch further context
	while ((potential_symbol_end >= 0) && full_context.at (potential_symbol_end).isSpace ()) {
		--potential_symbol_end;
	}

	// now identify the symbol and object (if any)
	RObject *object = 0;
	call_opening = KTextEditor::Cursor (-1, -1);
	if (potential_symbol_end > 0) {
		QString effective_symbol = RKCommonFunctions::getCurrentSymbol (full_context, potential_symbol_end);
		if (!effective_symbol.isEmpty ()) {
			object = RObjectList::getObjectList ()->findObject (effective_symbol);
			call_opening = KTextEditor::Cursor (line, potential_symbol_end+1);
		}
	}

	callhint_model->setFunction (object);
}

void startModel (KTextEditor::CodeCompletionInterface* iface, KTextEditor::CodeCompletionModel *model, bool start, const KTextEditor::Range &range, QList<KTextEditor::CodeCompletionModel*> *active_models) {
	if (start && model->rowCount () > 0) {
		if (!active_models->contains (model)) {
			iface->startCompletion (range, model);
			active_models->append (model);
		}
	} else {
		active_models->removeAll (model);
	}
}

void RKCompletionManager::updateVisibility () {
	RK_TRACE (COMMANDEDITOR);

	if (!cc_iface->isCompletionActive ()) {
		active_models.clear ();
	}

	bool min_len = (currentCompletionWord ().length () >= RKSettingsModuleCommandEditor::completionMinChars ());
	startModel (cc_iface, completion_model, min_len, symbol_range, &active_models);
	startModel (cc_iface, file_completion_model, min_len, symbol_range, &active_models);
	if (kate_keyword_completion_model) {
		// Model needs to update, first, as we have not handled it in tryCompletion:
		if (min_len) kate_keyword_completion_model->completionInvoked (view (), symbol_range, KTextEditor::CodeCompletionModel::ManualInvocation);
		startModel (cc_iface, kate_keyword_completion_model, min_len, symbol_range, &active_models);
	}
// NOTE: Freaky bug in KF 5.44.0: Call hint will not show for the first time, if logically above the primary screen. TODO: provide patch for kateargumenthinttree.cpp:166pp
	startModel (cc_iface, callhint_model, true, currentCallRange (), &active_models);
	startModel (cc_iface, arghint_model, true, argname_range, &active_models);

	if (!active_models.isEmpty ()) {
		active = true;
	} else {
		cc_iface->abortCompletion ();
		active = false;
	}

	if (!active) update_call = true;
}

void RKCompletionManager::textInserted (KTextEditor::Document*, const KTextEditor::Cursor& position, const QString& text) {
	if (active) {
		if (position < call_opening) update_call = true;
		else if (text.contains (QChar ('(')) || text.contains (QChar (')'))) update_call = true;
	}
	tryCompletionProxy();
}

void RKCompletionManager::textRemoved (KTextEditor::Document*, const KTextEditor::Range& range, const QString& text) {
	if (active) {
		if (range.start () < call_opening) update_call = true;
		else if (text.contains (QChar ('(')) || text.contains (QChar (')'))) update_call = true;
	}
	tryCompletionProxy();
}

void RKCompletionManager::lineWrapped (KTextEditor::Document* , const KTextEditor::Cursor& ) {
// should already have been handled by textInserted()
	tryCompletionProxy ();
}

void RKCompletionManager::lineUnwrapped (KTextEditor::Document* , int ) {
// should already have been handled by textRemoved()
	tryCompletionProxy ();
}

void RKCompletionManager::cursorPositionChanged (KTextEditor::View* view, const KTextEditor::Cursor& newPosition) {
	if (active) {
		if (newPosition < call_opening) update_call = true;
		else {
			QString text = view->document ()->text (KTextEditor::Range (newPosition, cached_position));
			if (text.contains (QChar ('(')) || text.contains (QChar (')'))) update_call = true;
		}
	}
	tryCompletionProxy ();
}

KTextEditor::Range RKCompletionManager::currentCallRange () const {
	return KTextEditor::Range (call_opening, _view->cursorPosition ());
}

bool RKCompletionManager::eventFilter (QObject* watched, QEvent* event) {
	if (event->type () == QEvent::KeyPress || event->type () == QEvent::ShortcutOverride) {
		RK_TRACE (COMMANDEDITOR);	// avoid loads of empty traces, putting this here
		QKeyEvent *k = static_cast<QKeyEvent *> (event);

		if (k->key () == Qt::Key_Tab && (!k->modifiers ())) {
			// TODO: It is not quite clear, what behavior is desirable, in case more than one completion model is active at a time.
			//       For now, we use the simplest solution (implemenation-wise), and complete from the topmost-model, only
			// TODO: Handle the ktexteditor builtin models, too.
			bool exact = false;
			QString comp;
			bool handled = false;
			if (active_models.contains (arghint_model)) {
				comp = arghint_model->partialCompletion (&exact);
				handled = true;
			} else if (active_models.contains (completion_model)) {
				comp = completion_model->partialCompletion (&exact);
				handled = true;
			} else if (active_models.contains (file_completion_model)) {
				comp = file_completion_model->partialCompletion (&exact);
				handled = true;
			}

			if (handled) {
				RK_DEBUG(COMMANDEDITOR, DL_DEBUG, "Tab completion: %s", qPrintable (comp));
				view ()->document ()->insertText (view ()->cursorPosition (), comp);
				if (exact) cc_iface->abortCompletion ();
				else if (comp.isEmpty ()) {
					QApplication::beep (); // TODO: unfortunately, we catch *two* tab events, so this is not good, yet
				}
				return true;
			}
		} else if ((k->key () == Qt::Key_Up || k->key () == Qt::Key_Down) && cc_iface->isCompletionActive ()) {
			// Make up / down-keys (without alt) navigate in the document (aborting the completion)
			// Meke alt+up / alt+down naviate in the completion list
			if (k->modifiers () & Qt::AltModifier) {
				if (k->type() != QKeyEvent::KeyPress) return true;  // eat the release event

				// No, we cannot just send a fake key event, easily...
				KActionCollection *kate_edit_actions = view ()->findChild<KActionCollection*> ("edit_actions");
				QAction *action = kate_edit_actions ? (kate_edit_actions->action (k->key () == Qt::Key_Up ? "move_line_up" : "move_line_down")) : 0;
				if (!action) {
					kate_edit_actions = view ()->actionCollection ();
					action = kate_edit_actions ? (kate_edit_actions->action (k->key () == Qt::Key_Up ? "move_line_up" : "move_line_down")) : 0;
				}
				if (action) action->trigger ();
				else RK_ASSERT (action);
				return true;
			} else {
				cc_iface->abortCompletion ();
				return false;
			}
		}
	}

	return false;
}

//////////////////////// RKCompletionModelBase ////////////////////

RKCompletionModelBase::RKCompletionModelBase (RKCompletionManager *manager) : KTextEditor::CodeCompletionModel (manager) {
	RK_TRACE (COMMANDEDITOR);
	n_completions = 0;
	RKCompletionModelBase::manager = manager;
}

RKCompletionModelBase::~RKCompletionModelBase () {
	RK_TRACE (COMMANDEDITOR);
}

QModelIndex RKCompletionModelBase::index (int row, int column, const QModelIndex& parent) const {
	if (!parent.isValid ()) { // root
		if (row == 0) return createIndex (row, column, quintptr (HeaderItem));
	} else if (isHeaderItem (parent)) {
		return createIndex (row, column, quintptr (LeafItem));
	}
	return QModelIndex ();
}

QModelIndex RKCompletionModelBase::parent (const QModelIndex& index) const {
	if (index.isValid () && !isHeaderItem (index)) {
		return createIndex (0, 0, quintptr (HeaderItem));
	}
	return QModelIndex ();
}

int RKCompletionModelBase::rowCount (const QModelIndex& parent) const {
	if (!parent.isValid ()) return (n_completions ? 1 : 0); // header item, if list not empty
	if (isHeaderItem (parent)) return n_completions;
	return 0;  // no children on completion entries
}

//////////////////////// RKCodeCompletionModel ////////////////////

RKCodeCompletionModel::RKCodeCompletionModel (RKCompletionManager *manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);

	setHasGroups (true);
}

RKCodeCompletionModel::~RKCodeCompletionModel () {
	RK_TRACE (COMMANDEDITOR);
}

void RKCodeCompletionModel::updateCompletionList (const QString& symbol) {
	RK_TRACE (COMMANDEDITOR);

	if (current_symbol == symbol) return;	// already up to date
	beginResetModel ();

	RObject::ObjectList matches;
	QStringList objectpath = RObject::parseObjectPath (symbol);
	if (!objectpath.isEmpty () && !objectpath[0].isEmpty ()) {  // Skip completion, if the current symbol is '""' (or another empty quote), for instance
		matches = RObjectList::getObjectList ()->findObjectsMatching (symbol);
	}

	// copy the map to two lists. For one thing, we need an int indexable storage, for another, caching this information is safer
	// in case objects are removed while the completion mode is active.
	n_completions = matches.size ();
	icons.clear ();
	icons.reserve (n_completions);
	names = RObject::getFullNames (matches, RKSettingsModuleCommandEditor::completionOptions());
	for (int i = 0; i < n_completions; ++i) {
		icons.append (RKStandardIcons::iconForObject (matches[i]));
	}

	current_symbol = symbol;

	endResetModel ();
}

KTextEditor::Range RKCodeCompletionModel::completionRange (KTextEditor::View *, const KTextEditor::Cursor&c) {
	return manager->currentSymbolRange ();
}

QVariant RKCodeCompletionModel::data (const QModelIndex& index, int role) const {
	if (isHeaderItem (index)) {
		if (role == Qt::DisplayRole) return i18n ("Objects on search path");
		if (role == KTextEditor::CodeCompletionModel::GroupRole) return Qt::DisplayRole;
		if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) return 1;  // sort below arghint model
		return QVariant ();
	}

	int col = index.column ();
	int row = index.row ();

	if ((role == Qt::DisplayRole) || (role == KTextEditor::CodeCompletionModel::CompletionRole)) {
		if (col == KTextEditor::CodeCompletionModel::Name) {
			return (names.value (row));
		}
	} else if (role == Qt::DecorationRole) {
		if (col == KTextEditor::CodeCompletionModel::Icon) {
			return (icons.value (row));
		}
	} else if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) {
		return (row);  // disable sorting
	} else if (role == KTextEditor::CodeCompletionModel::MatchQuality) {
		return (10);
	}

	return QVariant ();
}

QString findCommonCompletion (const QStringList &list, const QString &lead, bool *exact_match) {
	RK_TRACE (COMMANDEDITOR);
	RK_DEBUG(COMMANDEDITOR, DL_DEBUG, "Looking for commong completion among set of %d, starting with %s", list.size (), qPrintable (lead));

	*exact_match = true;
	QString ret;
	bool first = true;
	int lead_size = lead.count ();
	for (int i = list.size () - 1; i >= 0; --i) {
		if (!list[i].startsWith (lead)) continue;

		QString candidate = list[i].mid (lead_size);
		if (first) {
			ret = candidate;
			first = false;
		} else {
			if (ret.length () > candidate.length ()) {
				ret = ret.left (candidate.length ());
				*exact_match = false;
			}

			for (int c = 0; c < ret.length(); ++c) {
				if (ret[c] != candidate[c]) {
					*exact_match = false;
					if (!c) return QString ();

					ret = ret.left (c);
					break;
				}
			}
		}
	}

	return ret;
}

QString RKCodeCompletionModel::partialCompletion (bool* exact_match) {
	RK_TRACE (COMMANDEDITOR);

	// Here, we need to do completion on the *last* portion of the object-path, only, so we will be able to complete "obj" to "object", even if "object" is present as
	// both packageA::object and packageB::object.
	// Thus as a first step, we look up the short names. We do this, lazily, as this function is only called on demand.
	QStringList objectpath = RObject::parseObjectPath (current_symbol);
	if (objectpath.isEmpty () || objectpath[0].isEmpty ()) return (QString ());
	RObject::ObjectList matches = RObjectList::getObjectList ()->findObjectsMatching (current_symbol);

	QStringList shortnames;
	for (int i = 0; i < matches.count (); ++i) {
		shortnames.append (matches[i]->getShortName ());
	}
	QString lead = objectpath.last ();
	if (!shortnames.value (0).startsWith (lead)) lead.clear ();  // This could happen if the current path ends with '$', for instance

	return (findCommonCompletion (shortnames, lead, exact_match));
}

//////////////////////// RKCallHintModel //////////////////////////
RKCallHintModel::RKCallHintModel (RKCompletionManager* manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);
	function = 0;
}

#include <QDesktopWidget>
// TODO: There could be more than one function by a certain name, and we could support this!
void RKCallHintModel::setFunction(RObject* _function) {
	RK_TRACE (COMMANDEDITOR);

	if (function == _function) return;
	function = _function;

	beginResetModel ();
	if (function && function->isType (RObject::Function)) {
		// initialize hint
		RFunctionObject *fo = static_cast<RFunctionObject*> (function);
		QStringList args = fo->argumentNames ();
		QStringList defs = fo->argumentDefaults ();

		name = function->getFullName ();

		formals = '(';
		formatting.clear ();
		KTextEditor::Attribute format;
//		format.setFontBold (); // NOTE: Not good. makes size mis-calculation worse.
		format.setForeground (QBrush (Qt::green));  // But turns out purple?!

		// NOTE: Unfortunately, adding new-lines within (long) formals does not work. If this issue turns out to be relevant, we'll have to resort to breaking the formals into
		// several (dummy) items.
		int pos = 1;
		for (int i = 0; i < args.size (); ++i) {
			QString pair = args[i];
			if (!defs.value(i).isEmpty ()) pair.append ('=' + defs[i]);
			formatting.append ({ pos + args[i].length (), pair.length ()-args[i].length (), format });

			if (i < (args.size () - 1)) pair.append (", ");
			formals.append (pair);

			pos = pos + pair.length ();
		}
		formals.append (')');
		n_completions = 1;
	} else {
		n_completions = 0;
	}
	endResetModel ();
}

QVariant RKCallHintModel::data (const QModelIndex& index, int role) const {
	if (isHeaderItem (index)) {
		if (role == Qt::DisplayRole) return i18n ("Function calls");  // NOTE: Header is not currently used as of KF5 5.44.0
		if (role == KTextEditor::CodeCompletionModel::GroupRole) return Qt::DisplayRole;
		return QVariant ();
	}

	int col = index.column ();
	if (role == Qt::DisplayRole) {
		if (col == KTextEditor::CodeCompletionModel::Prefix) return (name);
		if (col == KTextEditor::CodeCompletionModel::Arguments) return (formals);
		if (col == KTextEditor::CodeCompletionModel::Postfix) return ("        "); // Size is of a bit for KF5 5.44.0. Provide some padding to work around cut-off parts.
	} else if (role == KTextEditor::CodeCompletionModel::ArgumentHintDepth) {
		return 1;
	} else if (role == KTextEditor::CodeCompletionModel::CompletionRole) {
		return KTextEditor::CodeCompletionModel::Function;
	} else if (role == KTextEditor::CodeCompletionModel::HighlightingMethod) {
		if (col == KTextEditor::CodeCompletionModel::Arguments) return KTextEditor::CodeCompletionModel::CustomHighlighting;
	} else if (role == KTextEditor::CodeCompletionModel::CustomHighlight) {
		if (col == KTextEditor::CodeCompletionModel::Arguments)  return formatting;
	} else if (role == KTextEditor::CodeCompletionModel::MatchQuality) {
		return (10);
	}

	return QVariant ();
}

KTextEditor::Range RKCallHintModel::completionRange (KTextEditor::View *, const KTextEditor::Cursor&) {
	return manager->currentCallRange ();
}

//////////////////////// RKArgumentHintModel //////////////////////
RKArgumentHintModel::RKArgumentHintModel (RKCompletionManager* manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);

	function = 0;
}

void RKArgumentHintModel::updateCompletionList (RObject* _function, const QString &argument) {
	RK_TRACE (COMMANDEDITOR);

	bool changed = false;
	if (function != _function) {
		beginResetModel ();
		function = _function;
		if (function && function->isType (RObject::Function)) {
			// initialize hint
			RFunctionObject *fo = static_cast<RFunctionObject*> (function);
			args = fo->argumentNames ();
			defs = fo->argumentDefaults ();
		} else {
			args.clear ();
			defs.clear ();
		}
	}

	if (changed || (argument != fragment)) {
		if (!changed) {
			changed = true;
			beginResetModel ();
		}
		fragment = argument;
		matches.clear ();
		for (int i = 0; i < args.size (); ++i) {
			if (args[i].startsWith (fragment)) matches.append (i);
		}
	}

	if (changed) {
		n_completions = matches.size ();
		endResetModel ();
	}
}

QVariant RKArgumentHintModel::data (const QModelIndex& index, int role) const {
	if (isHeaderItem (index)) {
		if (role == Qt::DisplayRole) return i18n ("Function arguments");
		if (role == KTextEditor::CodeCompletionModel::GroupRole) return Qt::DisplayRole;
		if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) return 0; // Sort above other models (except calltip)
		return QVariant ();
	}

	int col = index.column ();
	int row = index.row ();
	if (role == Qt::DisplayRole) {
		if (col == KTextEditor::CodeCompletionModel::Name) return (args.value (matches.value (row)));
		if (col == KTextEditor::CodeCompletionModel::Postfix) {
			QString def = defs.value (matches.value (row));
			if (!def.isEmpty ()) return (QString ('=' + def));
		}
	} else if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) {
		return row;  // disable sorting
	} else if (role == KTextEditor::CodeCompletionModel::CompletionRole) {
		return KTextEditor::CodeCompletionModel::Function;
	} else if (role == KTextEditor::CodeCompletionModel::MatchQuality) {
		return (20);
	}

	return QVariant ();
}

KTextEditor::Range RKArgumentHintModel::completionRange (KTextEditor::View*, const KTextEditor::Cursor&) {
	return manager->currentArgnameRange ();
}

QString RKArgumentHintModel::partialCompletion (bool* exact) {
	RK_TRACE (COMMANDEDITOR);

	return (findCommonCompletion (args, fragment, exact));
}

//////////////////////// RKFileCompletionModel ////////////////////
#include <KUrlCompletion>
RKFileCompletionModelWorker::RKFileCompletionModelWorker (const QString &_string) : QThread () {
	RK_TRACE (COMMANDEDITOR);
	string = _string;
	connect (this, &QThread::finished, this, &QObject::deleteLater);
}

void RKFileCompletionModelWorker::run () {
	RK_TRACE (COMMANDEDITOR);

	KUrlCompletion comp;
	comp.setDir (QUrl::fromLocalFile (QDir::currentPath ()));
	comp.makeCompletion (string);
	QStringList files = comp.allMatches ();

	QStringList exes;
	if (!QDir (string).isAbsolute ()) {  // NOTE: KUrlCompletion does not handle this well, returns, e.g. "/home/me/firefox" when given "/home/me/f"; KF5 5.44.0
		comp.setMode (KUrlCompletion::ExeCompletion);
		comp.makeCompletion (string);
		exes = comp.allMatches ();
	}

	emit (completionsReady (string, exes, files));
}

RKFileCompletionModel::RKFileCompletionModel (RKCompletionManager* manager) : RKCompletionModelBase (manager) {
	RK_TRACE (COMMANDEDITOR);
	worker = 0;
}

RKFileCompletionModel::~RKFileCompletionModel () {
	RK_TRACE (COMMANDEDITOR);
}

void RKFileCompletionModel::updateCompletionList(const QString& fragment) {
	RK_TRACE (COMMANDEDITOR);

	if (current_fragment == fragment) return;

	current_fragment = fragment;
	launchThread ();
}

void RKFileCompletionModel::launchThread () {
	RK_TRACE (COMMANDEDITOR);

	if (current_fragment.isEmpty ()) {
		completionsReady (QString (), QStringList (), QStringList ());
	} else if (!worker) {
		RK_DEBUG (COMMANDEDITOR, DL_DEBUG, "Launching filename completion thread for '%s'", qPrintable (current_fragment));
		worker = new RKFileCompletionModelWorker (current_fragment);
		connect (worker, &RKFileCompletionModelWorker::completionsReady, this, &RKFileCompletionModel::completionsReady);
		worker->start ();
	}
}

void RKFileCompletionModel::completionsReady (const QString& string, const QStringList& exes, const QStringList& files) {
	RK_TRACE (COMMANDEDITOR);

	RK_DEBUG (COMMANDEDITOR, DL_DEBUG, "Filename completion finished for '%s': %d matches", qPrintable (string), files.size () + exes.size ());
	worker = 0;
	if (current_fragment == string) {
		beginResetModel ();
		names = files + exes; // TODO: This could be prettier
		n_completions = names.size ();
		endResetModel ();
	} else {
		launchThread ();
	}
}

QVariant RKFileCompletionModel::data (const QModelIndex& index, int role) const {
	if (isHeaderItem (index)) {
		if (role == Qt::DisplayRole) return i18n ("Local file names");
		if (role == KTextEditor::CodeCompletionModel::GroupRole) return Qt::DisplayRole;
		if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) return 1; // Sort below arghint model
		return QVariant ();
	}

	int col = index.column ();
	int row = index.row ();
	if ((role == Qt::DisplayRole) || (role == KTextEditor::CodeCompletionModel::CompletionRole)) {
		if (col == KTextEditor::CodeCompletionModel::Name) {
			return (names.value (row));
		}
	} else if (role == KTextEditor::CodeCompletionModel::InheritanceDepth) {
		return (row);  // disable sorting
	} else if (role == KTextEditor::CodeCompletionModel::MatchQuality) {
		return (10);
	}
	return QVariant ();
}

QString RKFileCompletionModel::partialCompletion (bool* exact) {
	RK_TRACE (COMMANDEDITOR);

	return (findCommonCompletion (names, current_fragment, exact));
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


