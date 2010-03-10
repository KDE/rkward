/***************************************************************************
                          rkhtmlwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
    copyright            : (C) 2005, 2006, 2007, 2009, 2010 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "rkhtmlwindow.h"

#include <khtmlview.h>
#include <khtml_part.h>
#include <klibloader.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/partmanager.h>
#include <kactioncollection.h>
#include <kdirwatch.h>
#include <kmimetype.h>

#include <qfileinfo.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qdir.h>
#include <QHBoxLayout>
#include <QHostInfo>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "rkhelpsearchwindow.h"
#include "../rkward.h"
#include "../rkconsole.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettingsmoduler.h"
#include "../settings/rksettingsmoduleoutput.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardactions.h"
#include "../misc/rkstandardicons.h"
#include "../misc/xmlhelper.h"
#include "../misc/rkxmlguisyncer.h"
#include "../plugin/rkcomponentmap.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkworkplaceview.h"
#include "../debug.h"

RKHTMLWindow::RKHTMLWindow (QWidget *parent, WindowMode mode) : RKMDIWindow (parent, RKMDIWindow::HelpWindow) {
	RK_TRACE (APP);
	scroll_position=-1;

	setComponentData (KGlobal::mainComponent ());

	khtmlpart = new KHTMLPart (this, 0, KHTMLPart::BrowserViewGUI);
	setPart (khtmlpart);
	fixupPartGUI ();
	initializeActivationSignals ();
	RKXMLGUISyncer::self()->registerChangeListener (khtmlpart, this, SLOT (fixupPartGUI()));
	khtmlpart->setSelectable (true);
	setFocusProxy (khtmlpart->widget ());
	
	khtmlpart->widget ()->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *pLayout = new QHBoxLayout (this);
	pLayout->setContentsMargins (0, 0, 0, 0);
	pLayout->addWidget (khtmlpart->widget ());

	// We have to connect this in order to allow browsing.
	connect (khtmlpart->browserExtension (), SIGNAL (openUrlRequestDelayed (const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)), this, SLOT (slotOpenUrl (const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)));
	connect (khtmlpart, SIGNAL (completed ()), this, SLOT (loadDone ()));

	current_history_position = -1;
	url_change_is_from_history = false;

	initActions ();
	window_mode = Undefined;
	useMode (mode);
}

RKHTMLWindow::~RKHTMLWindow () {
	RK_TRACE (APP);

	delete khtmlpart;
}

void RKHTMLWindow::fixupPartGUI () {
	RK_TRACE (APP);

	// strip down the khtmlpart's GUI. remove some stuff we definitely don't need.
	RKCommonFunctions::removeContainers (khtmlpart, QString ("tools,security,extraToolBar,saveBackground,saveFrame,printFrame,kget_menu").split (','), true);
}

QString RKHTMLWindow::getDescription () {
	RK_TRACE (APP);

	if (window_mode == HTMLOutputWindow) {
		return ("output:" + current_url.url ());

	} else {
		QString fixed_url = current_url.url ().replace (RKSettingsModuleR::helpBaseUrl(), "rkward://RHELPBASE");
		return ("help:" + fixed_url);
	}
}

bool RKHTMLWindow::isModified () {
	RK_TRACE (APP);
	return false;
}

void RKHTMLWindow::initActions () {
	RK_TRACE (APP);

	// common actions
	actionCollection ()->addAction (KStandardAction::Copy, "copy", khtmlpart->browserExtension (), SLOT (copy ()));

	print = actionCollection ()->addAction (KStandardAction::Print, "print_html", this, SLOT (slotPrint()));

	run_selection = RKStandardActions::runSelection (this, "run_selection", this, SLOT (runSelection()));

		// needed to enable / disable the run selection action
	connect (khtmlpart, SIGNAL (selectionChanged()), this, SLOT (selectionChanged()));
	selectionChanged ();

	// help window actions
	back = actionCollection ()->addAction (KStandardAction::Back, "help_back", this, SLOT (slotBack()));
	back->setEnabled (false);

	forward = actionCollection ()->addAction (KStandardAction::Forward, "help_forward", this, SLOT (slotForward()));
	forward->setEnabled (false);

	// output window actions
	outputFlush = actionCollection ()->addAction ("output_flush", this, SLOT (flushOutput()));
	outputFlush->setText (i18n ("&Flush Output"));
	outputFlush->setIcon (KIcon ("edit-delete"));

	outputRefresh = actionCollection ()->addAction ("output_refresh", this, SLOT (refresh()));
	outputRefresh->setText (i18n ("&Refresh Output"));
	outputRefresh->setIcon (KIcon ("view-refresh"));
}

void RKHTMLWindow::selectionChanged () {
	RK_TRACE (APP);

	if (!run_selection) {
		RK_ASSERT (false);
		return;
	}

	run_selection->setEnabled (khtmlpart->hasSelection ());
}

void RKHTMLWindow::runSelection () {
	RK_TRACE (APP);

	RKConsole::pipeUserCommand (khtmlpart->selectedText ());
}

void RKHTMLWindow::doGotoAnchor (const QString &anchor_name) {
	RK_TRACE (APP);

	goto_anchor_name = anchor_name;
	QTimer::singleShot (0, this, SLOT (doGotoAnchorNow ()));
}

void RKHTMLWindow::doGotoAnchorNow () {
	RK_TRACE (APP);

	khtmlpart->gotoAnchor (goto_anchor_name);
}

void RKHTMLWindow::slotPrint () {
	RK_TRACE (APP);

	khtmlpart->view ()->print ();
}

void RKHTMLWindow::slotForward () {
	RK_TRACE (APP);
	RK_ASSERT (window_mode == HTMLHelpWindow);

	url_change_is_from_history = true;

	++current_history_position;
	int history_last = url_history.count () - 1;
	RK_ASSERT (current_history_position > 0);
	RK_ASSERT (current_history_position <= history_last);
	openURL (url_history[current_history_position]);

	back->setEnabled (true);
	forward->setEnabled (current_history_position < history_last);
	url_change_is_from_history = false;
}

void RKHTMLWindow::slotBack () {
	RK_TRACE (APP);
	RK_ASSERT (window_mode == HTMLHelpWindow);

	url_change_is_from_history = true;
	--current_history_position;
	RK_ASSERT (current_history_position >= 0);
	openURL (url_history[current_history_position]);

	forward->setEnabled (true);
	back->setEnabled (current_history_position > 0);
	url_change_is_from_history = false;
}

bool RKHTMLWindow::handleRKWardURL (const KUrl &url) {
	RK_TRACE (APP);

	if (url.protocol () == "rkward") {
		if (url.host () == "runplugin") {
			QString path = url.path ();
			if (path.startsWith ('/')) path = path.mid (1);
			int sep = path.indexOf ('/');
			RKComponentMap::invokeComponent (path.left (sep), path.mid (sep+1).split ('\n', QString::SkipEmptyParts));
			return true;
		} else {
			bool ok = false;
			if (url.host () == "component") {
				ok = renderRKHelp (url);
			} else if (url.host () == "rhelp") {
				// TODO: find a nice solution to render this in the current window
				RKHelpSearchWindow::mainHelpSearch ()->getFunctionHelp (url.path ().mid (1));
				return true;
			} else if (url.host () == "page") {
				ok = renderRKHelp (url);
			} else if (url.host ().toUpper () == "RHELPBASE") {	// NOTE: QUrl () may lowercase the host part, internally
				KUrl fixed_url = KUrl (RKSettingsModuleR::helpBaseUrl ());
				fixed_url.setPath (url.path ());
				if (url.hasQuery ()) fixed_url.setQuery (url.query ());
				if (url.hasFragment ()) fixed_url.setFragment (url.fragment ());
				ok = openURL (fixed_url);
			}
		
			if (!ok) {
				fileDoesNotExistMessage ();
			}
		
			changeURL (url);
			return true;
		}
	}
	return false;
}

bool RKHTMLWindow::openURL (const KUrl &url) {
	RK_TRACE (APP);

	if (handleRKWardURL (url)) return true;
	if (window_mode == HTMLOutputWindow) {
		if (url != current_url) {
			// output window should not change url after initialization
			if (!current_url.isEmpty ()) {
				RK_ASSERT (false);
				return false;
			}

			current_url = url;	// needs to be set before registering
			RKOutputWindowManager::self ()->registerWindow (this);
		}
	} else {
		if (!(url.isLocalFile ())) {
			if (window_mode == HTMLHelpWindow) {
				// since R 2.10.0, help urls may be on local ports
				if (url.protocol ().toLower ().startsWith ("http")) {
					QString host = url.host ();
					if ((host == "127.0.0.1") || (host == "localhost") || host == QHostInfo::localHostName ()) {
						khtmlpart->openUrl (url);
						changeURL (url);
						return true;
					}
				}
			}
		}
		if (!(url.isLocalFile () && KMimeType::findByUrl (url)->is ("text/html"))) {
			RKWorkplace::mainWorkplace ()->openAnyUrl (url);
			return true;
		}
	}

	QFileInfo out_file (url.toLocalFile ());
	bool ok = out_file.exists();
	if (ok)  {
		khtmlpart->openUrl (url);
	} else {
		fileDoesNotExistMessage ();
	}
	changeURL (url);
	return ok;
}

KUrl RKHTMLWindow::url () {
	return current_url;
}

void RKHTMLWindow::changeURL (const KUrl &url) {
	current_url = url;
	updateCaption (url);

	if (!url_change_is_from_history) {
		if (window_mode == HTMLHelpWindow) {
			int history_last = url_history.count () - 1;
			for (int i = history_last; i > current_history_position; --i) {
				url_history.removeLast ();
			}

			url_history.append (url);
			++current_history_position;
			back->setEnabled (current_history_position > 0);	// may be false, if this is the very first page to be added to the history
			forward->setEnabled (false);
		}
	}
}

void RKHTMLWindow::updateCaption (const KUrl &url) {
	RK_TRACE (APP);

	if (window_mode == HTMLOutputWindow) setCaption (i18n ("Output %1").arg (url.fileName ()));
	else setCaption (url.fileName ());
}

void RKHTMLWindow::slotOpenUrl (const KUrl & url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &) {
	RK_TRACE (APP);
	openURL (url);
}

void RKHTMLWindow::refresh () {
	RK_TRACE (APP);

	KParts::OpenUrlArguments args;
	args.setReload (true);		// this forces the next openURL to reload all images
	khtmlpart->setArguments (args);

	scroll_position = khtmlpart->view ()->contentsY ();

	openURL (current_url);
}

void RKHTMLWindow::loadDone () {
	RK_TRACE (APP);

	if (window_mode == HTMLOutputWindow) {	// scroll to bottom
		khtmlpart->view ()->setContentsPos (0, khtmlpart->view ()->contentsHeight ());
	} else {	// scroll to previous pos
		if (scroll_position >= 0) khtmlpart->view()->setContentsPos (0, scroll_position);
	}
}

void RKHTMLWindow::useMode (WindowMode new_mode) {
	RK_TRACE (APP);

	RK_ASSERT (new_mode != Undefined);
	if (window_mode == new_mode) return;

	if (new_mode == HTMLOutputWindow) {
		type = RKMDIWindow::OutputWindow;
		setWindowIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowOutput));

		print->setText (i18n ("Print output"));
		QAction *action = khtmlpart->action ("saveDocument");
		if (action) action->setText (i18n ("Export page as HTML"));
		else RK_ASSERT (false);		// we should know about this

		setXMLFile ("rkoutputwindow.rc");
		run_selection->setVisible (false);
	} else {
		RK_ASSERT (new_mode == HTMLHelpWindow);

		type = RKMDIWindow::HelpWindow;
		setWindowIcon (RKStandardIcons::getIcon (RKStandardIcons::WindowHelp));

		print->setText (i18n ("Print page"));
		QAction *action = khtmlpart->action ("saveDocument");
		if (action) action->setText (i18n ("Save Output as HTML"));
		else RK_ASSERT (false);		// we should know about this

		setXMLFile ("rkhelpwindow.rc");
		run_selection->setVisible (true);
	}

	if (parentClient ()) khtmlpart->removeChildClient (this);
	khtmlpart->insertChildClient (this);

	updateCaption (current_url);
	window_mode = new_mode;
}

void RKHTMLWindow::fileDoesNotExistMessage () {
	RK_TRACE (APP);

	khtmlpart->begin();
	if (window_mode == HTMLOutputWindow) {
		khtmlpart->write (i18n ("<HTML><BODY><H1>RKWard output file could not be found</H1>\n</BODY></HTML>"));
	} else {
		khtmlpart->write ("<html><body><h1>" + i18n ("Page does not exist or is broken") + "</h1></body></html>");
	}
	khtmlpart->end();
}

void RKHTMLWindow::flushOutput () {
	RK_TRACE (APP);

	int res = KMessageBox::questionYesNo (this, i18n ("Do you really want to flush the output? It will not be possible to restore it."), i18n ("Flush output?"));
	if (res==KMessageBox::Yes) {
		QFile out_file (current_url.toLocalFile ());
		QDir out_dir = QFileInfo (out_file).absoluteDir ();
		out_file.remove ();

		// remove all the graphs
		out_dir.setNameFilters (QStringList ("graph*.png"));
		QStringList graph_files = out_dir.entryList ();
		for (QStringList::const_iterator it = graph_files.constBegin (); it != graph_files.constEnd (); ++it) {
			QFile file (out_dir.absoluteFilePath (*it));
			file.remove ();
		}

		// initialize the now empty file
		RKGlobals::rInterface ()->issueCommand ("rk.set.output.html.file (\"" + out_file.fileName () + "\")\n", RCommand::App);
		refresh ();
	}
}

bool RKHTMLWindow::renderRKHelp (const KUrl &url) {
	RK_TRACE (APP);

	if (url.protocol () != "rkward") {
		RK_ASSERT (false);
		return (false);
	}

	useMode (HTMLHelpWindow);

	bool for_component = false;		// is this a help page for a component, or a top-level help page?
	if (url.host () == "component") for_component = true;

	bool success = false;
	XMLHelper *component_xml = new XMLHelper ();
	XMLHelper *help_xml = new XMLHelper ();
	QStringList anchors, anchornames;

	while (true) {		// dirty hack to streamline exit code: breaking from this while, before success is set to true will cause the XMLHelpers to be deleted, and false returned.
		RKComponentHandle *chandle = 0;
		QString help_file_name;
		QDomElement element;
		QDomElement component_doc_element;
		QString help_base_dir = RKCommonFunctions::getRKWardDataDir () + "pages/";
		QString css_filename = "file://" + help_base_dir + "rkward_help.css";

		// determine help file, and prepare
		if (for_component) {
			chandle = componentPathToHandle (url.path ());
			if (!chandle) break;

			component_doc_element = component_xml->openXMLFile (chandle->getFilename (), DL_ERROR);
			if (component_doc_element.isNull ()) break;
			element = component_xml->getChildElement (component_doc_element, "help", DL_ERROR);
			if (!element.isNull ()) {
				help_file_name = component_xml->getStringAttribute (element, "file", QString::null, DL_ERROR);
				if (!help_file_name.isEmpty ()) help_file_name = QFileInfo (chandle->getFilename ()).absoluteDir ().filePath (help_file_name);
			}
		} else {
			help_file_name = help_base_dir + url.path () + ".rkh";
		}
		RK_DO (qDebug ("rendering help page for local file %s", help_file_name.toLatin1().data()), APP, DL_DEBUG);

		// open help file
		QDomElement help_doc_element = help_xml->openXMLFile (help_file_name, DL_ERROR);
		if (help_doc_element.isNull () && (!for_component)) break;

		// initialize output, and set title
		khtmlpart->begin (url);
		QString page_title (i18n ("No Title"));
		if (for_component) {
			page_title = chandle->getLabel ();
		} else {
			element = help_xml->getChildElement (help_doc_element, "title", DL_WARNING);
			if (!element.isNull ()) {
				page_title = element.text ();
			}
		}
		khtmlpart->write ("<html><head><title>" + page_title + "</title><link rel=\"stylesheet\" type=\"text/css\" href=\"" + css_filename + "\"></head>\n<body><div id=\"main\">\n<h1>" + page_title + "</h1>\n");

		if (help_doc_element.isNull ()) {
			RK_ASSERT (for_component);
			khtmlpart->write (i18n ("<h1>Help page missing</h1>\n<p>The help page for this component has not yet been written (or is broken). Please consider contributing it.</p>\n"));
		}
		if (for_component) {
			QString component_id = componentPathToId (url.path());
			RKComponentHandle *handle = componentPathToHandle (url.path());
			if (handle && handle->isAccessible ()) khtmlpart->write ("<a href=\"rkward://runplugin/" + component_id + "/\">" + i18n ("Use %1 now", page_title) + "</a>");
		}

		// fix all elements containing an "src" attribute
		QDir base_path (QFileInfo (help_file_name).absolutePath());
		XMLChildList src_elements = help_xml->findElementsWithAttribute (help_doc_element, "src", QString (), true, DL_DEBUG);
		for (XMLChildList::iterator it = src_elements.begin (); it != src_elements.end (); ++it) {
			QString src = (*it).attribute ("src");
			if (KUrl::isRelativeUrl (src)) {
				src = "file://" + QDir::cleanPath (base_path.filePath (src));
				(*it).setAttribute ("src", src);
			}
		}

		// render the sections
		element = help_xml->getChildElement (help_doc_element, "summary", DL_INFO);
		if (!element.isNull ()) {
			khtmlpart->write (startSection ("summary", i18n ("Summary"), QString (), &anchors, &anchornames));
			khtmlpart->write (renderHelpFragment (element));
		}

		element = help_xml->getChildElement (help_doc_element, "usage", DL_INFO);
		if (!element.isNull ()) {
			khtmlpart->write (startSection ("usage", i18n ("Usage"), QString (), &anchors, &anchornames));
			khtmlpart->write (renderHelpFragment (element));
		}

		XMLChildList section_elements = help_xml->getChildElements (help_doc_element, "section", DL_INFO);
		for (XMLChildList::iterator it = section_elements.begin (); it != section_elements.end (); ++it) {
			QString title = help_xml->getStringAttribute (*it, "title", QString (), DL_WARNING);
			QString shorttitle = help_xml->getStringAttribute (*it, "shorttitle", QString (), DL_DEBUG);
			QString id = help_xml->getStringAttribute (*it, "id", QString (), DL_WARNING);
			khtmlpart->write (startSection (id, title, shorttitle, &anchors, &anchornames));
			khtmlpart->write (renderHelpFragment (*it));
		}

		// the section "settings" is the most complicated, as the labels of the individual GUI items has to be fetched from the component description. Of course it is only meaningful for component help, and not rendered for top level help pages.
		if (for_component) {
			element = help_xml->getChildElement (help_doc_element, "settings", DL_INFO);
			if (!element.isNull ()) {
				khtmlpart->write (startSection ("settings", i18n ("GUI settings"), QString (), &anchors, &anchornames));
				XMLChildList setting_elements = help_xml->getChildElements (element, QString (), DL_WARNING);
				for (XMLChildList::iterator it = setting_elements.begin (); it != setting_elements.end (); ++it) {
					if ((*it).tagName () == "setting") {
						QString id = help_xml->getStringAttribute (*it, "id", QString (), DL_WARNING);
						QString title = help_xml->getStringAttribute (*it, "title", QString (), DL_INFO);
						if (title.isEmpty ()) {
							QDomElement source_element = component_xml->findElementWithAttribute (component_doc_element, "id", id, true, DL_WARNING);
							title = component_xml->getStringAttribute (source_element, "label", i18n ("Unnamed GUI element"), DL_WARNING);
						}
						khtmlpart->write ("<h4>" + title + "</h4>");
						khtmlpart->write (renderHelpFragment (*it));
					} else if ((*it).tagName () == "caption") {
						QString id = help_xml->getStringAttribute (*it, "id", QString (), DL_WARNING);
						QString title = help_xml->getStringAttribute (*it, "title", QString (), DL_INFO);
						QDomElement source_element = component_xml->findElementWithAttribute (component_doc_element, "id", id, true, DL_WARNING);
						title = component_xml->getStringAttribute (source_element, "label", title, DL_WARNING);
						khtmlpart->write ("<h3>" + title + "</h3>");
					} else {
						help_xml->displayError (&(*it), "Tag not allowed, here", DL_WARNING);
					}
				}
			}
		}

		// "related" section
		element = help_xml->getChildElement (help_doc_element, "related", DL_INFO);
		if (!element.isNull ()) {
			khtmlpart->write (startSection ("related", i18n ("Related functions and pages"), QString (), &anchors, &anchornames));
			khtmlpart->write (renderHelpFragment (element));
		}

		// "technical" section
		element = help_xml->getChildElement (help_doc_element, "technical", DL_INFO);
		if (!element.isNull ()) {
			khtmlpart->write (startSection ("technical", i18n ("Technical details"), QString (), &anchors, &anchornames));
			khtmlpart->write (renderHelpFragment (element));
		}

		// create a navigation bar
		KUrl url_copy = url;
		QString navigation;
		QStringList::const_iterator names_it = anchornames.constBegin ();
		for (QStringList::const_iterator it = anchors.constBegin (); it != anchors.constEnd (); ++it) {
			if (!((*it).isNull () || (*names_it).isNull ())) {
				url_copy.setRef (*it);
				navigation.append ("<p><a href=\"" + url_copy.url () + "\">" + *names_it + "</a></p>\n");
			}

			if (names_it != anchornames.constEnd ()) {
				++names_it;
			} else {
				RK_ASSERT (false);
			}
		}
		khtmlpart->write ("</div><div id=\"navigation\">" + navigation + "</div>");
		khtmlpart->write ("</body></html>\n");
		khtmlpart->end ();

		QString ref = url.ref ();
		if (!ref.isEmpty ()) {
			doGotoAnchor (ref);
		}

		success = true;
		break;
	}

	delete (component_xml);
	delete (help_xml);
	return (success);
}

QString RKHTMLWindow::renderHelpFragment (QDomElement &fragment) {
	RK_TRACE (APP);

	// prepare all internal links
	QDomNodeList link_nodes = fragment.elementsByTagName ("link");
	for (int i=link_nodes.count (); i >= 0; --i) {
		QDomElement element = link_nodes.item (i).toElement ();
		if (element.isNull ()) continue;

		prepareHelpLink (&element);
	}

	// render to string
	QString ret;
	QTextStream stream (&ret, QIODevice::WriteOnly);
	for (QDomNode node = fragment.firstChild (); !node.isNull (); node = node.nextSibling ()) {
		node.save (stream, 0);
	}

	ret.prepend ("<p>");
	ret.append ("</p>");
	ret.replace ("\n\n", "</p>\n<p>");

	RK_DO (qDebug ("%s", ret.toLatin1 ().data ()), APP, DL_DEBUG);
	return ret;
}

void RKHTMLWindow::prepareHelpLink (QDomElement *link_element) {
	RK_TRACE (APP);

	link_element->setTagName ("a");
	if (link_element->text ().isEmpty ()) {
		QString text;
		KUrl url = link_element->attribute ("href");
		if (url.protocol () == "rkward") {
			if (url.host () == "component") {
				RKComponentHandle *chandle = componentPathToHandle (url.path ());
				if (chandle) text = chandle->getLabel ();
			} else if (url.host () == "rhelp") {
				text = i18n ("R Reference on '%1'", url.path ().mid (1));
			} else if (url.host () == "page") {
				QString help_base_dir = RKCommonFunctions::getRKWardDataDir () + "pages/";
		
				QString help_file_name = help_base_dir + url.path () + ".rkh";
				XMLHelper *xml = new XMLHelper ();

				QDomElement doc_element = xml->openXMLFile (help_file_name, DL_WARNING);
				QDomElement title_element = xml->getChildElement (doc_element, "title", DL_WARNING);
				text = title_element.text ();

				delete xml;
			}

			if (text.isEmpty ()) {
				text = i18n ("BROKEN REFERENCE");
				RK_DO (qDebug ("Broken reference to %s", url.path ().toLatin1 ().data ()), APP, DL_WARNING);
			}

			link_element->appendChild (link_element->ownerDocument ().createTextNode (text));
		}
	}
}

QString RKHTMLWindow::componentPathToId (QString path) {
	RK_TRACE (APP);

	QStringList path_segments = path.split ('/', QString::SkipEmptyParts);
	if (path_segments.count () > 2) return 0;
	if (path_segments.count () < 1) return 0;
	if (path_segments.count () == 1) path_segments.push_front ("rkward");
	RK_ASSERT (path_segments.count () == 2);

	return (path_segments.join ("::"));
}

RKComponentHandle *RKHTMLWindow::componentPathToHandle (QString path) {
	RK_TRACE (APP);

	return (RKComponentMap::getComponentHandle (componentPathToId (path)));
}

QString RKHTMLWindow::startSection (const QString &name, const QString &title, const QString &shorttitle, QStringList *anchors, QStringList *anchor_names) {
	QString ret = "<a name=\"" + name + "\">";
	ret.append ("<h2>" + title + "</h2>\n");
	anchors->append (name);
	if (!shorttitle.isNull ()) anchor_names->append (shorttitle);
	else anchor_names->append (title);
	return (ret);
}


/////////////////////////////////////
/////////////////////////////////////


// static
RKOutputWindowManager* RKOutputWindowManager::_self = 0;

RKOutputWindowManager* RKOutputWindowManager::self () {
	if (!_self) {
		RK_TRACE (APP);

		_self = new RKOutputWindowManager ();
	}
	return _self;
}

RKOutputWindowManager::RKOutputWindowManager () : QObject () {
	RK_TRACE (APP);

	file_watcher = new KDirWatch (this);
	connect (file_watcher, SIGNAL (dirty(const QString&)), this, SLOT (fileChanged(const QString&)));
	connect (file_watcher, SIGNAL (created(const QString&)), this, SLOT (fileChanged(const QString&)));
}

RKOutputWindowManager::~RKOutputWindowManager () {
	RK_TRACE (APP);

	file_watcher->removeFile (current_default_path);
	delete (file_watcher);
}

void RKOutputWindowManager::registerWindow (RKHTMLWindow *window) {
	RK_TRACE (APP);

	RK_ASSERT (window->mode () == RKHTMLWindow::HTMLOutputWindow);
	KUrl url = window->url ();

	if (!url.isLocalFile ()) {
		RK_ASSERT (false);		// should not happen right now, but might be an ok condition in the future. We can't monitor non-local files, though.
		return;
	}

	url.cleanPath ();
	QString file = url.toLocalFile ();
	if (!windows.contains (file, window)) {
		if (!windows.contains (file)) {
			if (file != current_default_path) file_watcher->addFile (file);
		}
	
		windows.insertMulti (file, window);
		connect (window, SIGNAL (destroyed(QObject*)), this, SLOT (windowDestroyed(QObject*)));
	} else {
		RK_ASSERT (false);
	}
}

void RKOutputWindowManager::setCurrentOutputPath (const QString &_path) {
	RK_TRACE (APP);

	KUrl url = KUrl::fromLocalFile (_path);
	url.cleanPath ();
	QString path = url.toLocalFile ();

	if (path == current_default_path) return;

	if (!windows.contains (path)) {
		file_watcher->addFile (path);
	}
	if (!windows.contains (current_default_path)) {
		file_watcher->removeFile (current_default_path);
	}

	current_default_path = path;
}

RKHTMLWindow* RKOutputWindowManager::getCurrentOutputWindow () {
	RK_TRACE (APP);

	RKHTMLWindow *current_output = windows.value (current_default_path);

	if (!current_output) {
		current_output = new RKHTMLWindow (RKWorkplace::mainWorkplace ()->view (), RKHTMLWindow::HTMLOutputWindow);

		current_output->openURL (KUrl::fromLocalFile (current_default_path));

		RK_ASSERT (current_output->url ().toLocalFile () == current_default_path);
	}

	return current_output;
}

void RKOutputWindowManager::fileChanged (const QString &path) {
	RK_TRACE (APP);

	RKHTMLWindow *w = 0;
	QList<RKHTMLWindow *> window_list = windows.values (path);
	for (int i = 0; i < window_list.size (); ++i) {
		window_list[i]->refresh ();
		w = window_list[i];
	}

	if (w) {
		if (RKSettingsModuleOutput::autoRaise ()) w->activate ();
	} else {
		RK_ASSERT (path == current_default_path);
		if (RKSettingsModuleOutput::autoShow ()) RKWorkplace::mainWorkplace ()->openOutputWindow (path);
	}
}

void RKOutputWindowManager::windowDestroyed (QObject *window) {
	RK_TRACE (APP);

	// warning: Do not call any methods on the window. It is half-destroyed, already.
	RKHTMLWindow *w = static_cast<RKHTMLWindow*> (window);

	QString path = windows.key (w);
	windows.remove (path, w);

	// if there are no further windows for this file, stop listening
	if ((path != current_default_path) && (!windows.contains (path))) {
		file_watcher->removeFile (path);
	}
}

#include "rkhtmlwindow.moc"
