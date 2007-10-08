/***************************************************************************
                          rkhtmlwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
    copyright            : (C) 2005, 2006, 2007 by Thomas Friedrichsmeier
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
#include <krun.h>
#include <kparts/partmanager.h>

#include <qfileinfo.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3HBoxLayout>

#include "../rkglobals.h"
#include "rkhelpsearchwindow.h"
#include "../rkward.h"
#include "../rkconsole.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/xmlhelper.h"
#include "../plugin/rkcomponentmap.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkworkplaceview.h"
#include "../debug.h"

RKHTMLWindow::RKHTMLWindow (QWidget *parent) : RKMDIWindow (parent, RKMDIWindow::HelpWindow) {
	RK_TRACE (APP);
	scroll_position=-1;
	
	khtmlpart = new KHTMLPart (this, 0, 0, 0, KHTMLPart::BrowserViewGUI);
	setPart (khtmlpart);
	initializeActivationSignals ();
	khtmlpart->setSelectable (true);
	setFocusProxy (khtmlpart->widget ());
	
	khtmlpart->widget ()->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	Q3HBoxLayout *pLayout = new Q3HBoxLayout (this);
	pLayout->addWidget (khtmlpart->widget ());

	// We have to connect this in order to allow browsing.
	connect (khtmlpart->browserExtension (), SIGNAL (openURLRequestDelayed (const KURL &, const KParts::URLArgs &)), this, SLOT (slotOpenURLRequest (const KURL &, const KParts::URLArgs &)));
	connect (khtmlpart, SIGNAL (completed ()), this, SLOT (loadDone ()));

	url_history.setAutoDelete (true);
	back = forward = print = run_selection = 0;		// initialization done in subclasses
	url_change_is_from_history = false;
}

RKHTMLWindow::~RKHTMLWindow () {
	RK_TRACE (APP);

	delete khtmlpart;
	khtmlpart = 0;	// in case we try to redelete in a parent class
}

QString RKHTMLWindow::getDescription () {
	RK_TRACE (APP);

	return ("help:" + khtmlpart->url ().url ());
}

bool RKHTMLWindow::isModified () {
	RK_TRACE (APP);
	return false;
}

void RKHTMLWindow::addCommonActions (KActionCollection *action_collection) {
	RK_TRACE (APP);

	// enable copy
	KStdAction::copy (khtmlpart->browserExtension (), SLOT (copy ()), action_collection, "copy");

	// run selection
	run_selection = new KAction (i18n ("Run selection"), QIcon (RKCommonFunctions::getRKWardDataDir () + "icons/run_selection.png"), KShortcut ("F8"), this, SLOT (runSelection ()), action_collection, "run_selection");

	// needed to enable / disable the run selection action
	connect (khtmlpart, SIGNAL (selectionChanged()), this, SLOT (selectionChanged()));
	selectionChanged ();
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

	url_change_is_from_history = true;
	url_history.next ();
	RK_ASSERT (url_history.current ());
	openURL (*(url_history.current ()));

	back->setEnabled (true);
	forward->setEnabled (url_history.current () != url_history.getLast ());
	url_change_is_from_history = false;
}

void RKHTMLWindow::slotBack () {
	RK_TRACE (APP);

	url_change_is_from_history = true;
	url_history.prev ();
	RK_ASSERT (url_history.current ());
	openURL (*(url_history.current ()));

	forward->setEnabled (true);
	back->setEnabled (url_history.current () != url_history.getFirst ());
	url_change_is_from_history = false;
}

bool RKHTMLWindow::openURL (const KURL &url) {
	RK_TRACE (APP);

	// asyncrhonously dealing with non-local files would be quite a task. We chose the simple answer instead...
	if (!url.isLocalFile ()) {
		if (KMessageBox::questionYesNo (this, i18n ("The url you are trying to open ('%1') is not a local file. Do you want to open the url in the default application?").arg (url.prettyURL ()), i18n ("Open in default application?")) != KMessageBox::Yes) {
			return false;
		}
		KRun *runner = new KRun (url);		// according to KRun-documentation, KRun will self-destruct when done.
		runner->setRunExecutables (false);
		return false;
	}

	khtmlpart->openURL (url);
	changeURL (url);

	return true;
}

KURL RKHTMLWindow::url () {
	if (khtmlpart) {
		return khtmlpart->url ();
	} else {
		RK_ASSERT (false);
		return KURL ();
	}
}

void RKHTMLWindow::changeURL (const KURL &url) {
	updateCaption (url);

	if (!url_change_is_from_history) {
		if (back && forward) {
			KURL *current_url = url_history.current ();
			while (current_url != url_history.getLast ()) {
				url_history.removeLast ();
			}
			KURL *url_copy = new KURL (url);
			url_history.append (url_copy);
			back->setEnabled (url_history.count () > 1);
			forward->setEnabled (false);
		}
	}
}

void RKHTMLWindow::updateCaption (const KURL &url) {
	RK_TRACE (APP);
	setCaption (url.filename ());
}

void RKHTMLWindow::slotOpenURLRequest(const KURL &url, const KParts::URLArgs & ) {
	RK_TRACE (APP);
	openURL (url);
}

void RKHTMLWindow::refresh () {
	RK_TRACE (APP);
	scroll_position = khtmlpart->view ()->contentsY ();
	openURL (khtmlpart->url ());
}

void RKHTMLWindow::loadDone () {
	RK_TRACE (APP);
	if (scroll_position >= 0) khtmlpart->view()->setContentsPos (0, scroll_position);
}

//##################### BEGIN RKOutputWindow #####################
//static 
RKOutputWindow* RKOutputWindow::current_output = 0;
QDateTime RKOutputWindow::last_refresh_time;

RKOutputWindow::RKOutputWindow (QWidget *parent) : RKHTMLWindow (parent), KXMLGUIClient () {
	RK_TRACE (APP);

	type = RKMDIWindow::OutputWindow;
	// strip down the khtmlpart's GUI. remove some stuff we definitely don't need.
	RKCommonFunctions::removeContainers (khtmlpart, QStringList::split (',', "tools,security,extraToolBar,saveBackground,saveFrame,printFrame,kget_menu"), true);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);
	setXMLFile ("rkoutputwindow.rc");

	khtmlpart->insertChildClient (this);

	setIcon (SmallIcon ("text_block"));
	setCaption (i18n ("Output"));

	outputFlush = new KAction (i18n ("&Flush Output"), "editdelete", 0, this, SLOT (flushOutput ()), actionCollection (), "output_flush");
	outputRefresh = new KAction (i18n ("&Refresh Output"), "reload", 0, this, SLOT (refreshOutput ()), actionCollection (), "output_refresh");
	print = KStdAction::print (this, SLOT (slotPrint ()), actionCollection (), "print_output");
	print->setText (i18n ("Print Output"));
	addCommonActions (actionCollection ());

	KAction *action = khtmlpart->action ("saveDocument");
	if (action) action->setText (i18n ("Save Output as HTML"));
}

//static
void RKOutputWindow::initialize () {
	RK_TRACE (APP);

	QFileInfo out_file (RKSettingsModuleGeneral::filesPath () + "/rk_out.html");
	last_refresh_time = out_file.lastModified ();
}

RKOutputWindow::~RKOutputWindow () {
	RK_TRACE (APP);

	if (this == current_output) {
		current_output = 0;
	}

	delete khtmlpart;
	khtmlpart = 0;	// in case we try to redelete in a parent class
}

QString RKOutputWindow::getDescription () {
	RK_TRACE (APP);

	return ("output:" + output_url.url ());
}

bool RKOutputWindow::openURL (const KURL &url) {
	RK_TRACE (APP);

	output_url = url;
	QFileInfo out_file (url.path ());
	bool ok = out_file.exists();
	if (ok)  {
		khtmlpart->browserExtension ()->setURLArgs (KParts::URLArgs (true, 0, 0));	// this forces the next openURL to reload all images
		RKHTMLWindow::openURL (url);
		last_refresh_time = out_file.lastModified ();
	} else {
		showOutputEmptyMessage ();
	}
	return ok;
}

void RKOutputWindow::updateCaption (const KURL &) {
}

void RKOutputWindow::refresh () {
	scroll_position = khtmlpart->view ()->contentsHeight ();
	openURL (output_url);
}

//static
RKOutputWindow* RKOutputWindow::refreshOutput (bool show, bool raise, bool only_if_modified) {
	RK_TRACE (APP);

	if (only_if_modified) {
		QFileInfo out_file (RKSettingsModuleGeneral::filesPath () + "/rk_out.html");
		if (out_file.lastModified () <= last_refresh_time) return current_output;
	}

	if (current_output) {
		if (raise) {
			current_output->activate ();
		}
		current_output->refresh ();
	} else {
		if (show) {
			// getCurrentOutput creates an output window
			getCurrentOutput ();
		}
	}

	return current_output;
}

//static
RKOutputWindow* RKOutputWindow::getCurrentOutput () {
	RK_TRACE (APP);
	
	if (!current_output) {
		current_output = new RKOutputWindow (RKWorkplace::mainWorkplace ()->view ());

		KURL url (RKSettingsModuleGeneral::filesPath () + "/rk_out.html");
		current_output->openURL (url);
	}

	return current_output;
}

void RKOutputWindow::flushOutput () {
	RK_TRACE (APP);

	int res = KMessageBox::questionYesNo (this, i18n ("Do you really want to flush the output? It will not be possible to restore it."), i18n ("Flush output?"));
	if (res==KMessageBox::Yes) {
		QFile out_file (RKSettingsModuleGeneral::filesPath () + "/rk_out.html");
		QDir out_dir = QFileInfo (out_file).dir (true);
		out_file.remove ();

		// remove all the graphs
		out_dir.setNameFilter ("graph*.png");
		QStringList graph_files = out_dir.entryList ();
		for (QStringList::const_iterator it = graph_files.constBegin (); it != graph_files.constEnd (); ++it) {
			QFile file (out_dir.absFilePath (*it, false));
			file.remove ();
		}
		refreshOutput (false, false, false);
	}
}

void RKOutputWindow::refreshOutput () {
	RK_TRACE (APP);

	refreshOutput (true, true, false);
}

void RKOutputWindow::showOutputEmptyMessage () {
	khtmlpart->begin();
	khtmlpart->write (i18n ("<HTML><BODY><H1>RKWard output</H1>\n<P>The output is empty.</P>\n</BODY></HTML>"));
	khtmlpart->end();
}

//#################### END RKOutputWindow ##########################
//#################### BEGIN RKHelpWindow ##########################

RKHelpWindow::RKHelpWindow (QWidget *parent) : RKHTMLWindow (parent), KXMLGUIClient () {
	RK_TRACE (APP);

	// strip down the khtmlpart's GUI. remove some stuff we definitely don't need.
	RKCommonFunctions::removeContainers (khtmlpart, QStringList::split (',', "tools,security,extraToolBar,saveBackground,saveDocument,saveFrame,printFrame,kget_menu"), true);

	back = KStdAction::back (this, SLOT (slotBack ()), actionCollection (), "help_back");
	back->setEnabled (false);
	forward = KStdAction::forward (this, SLOT (slotForward ()), actionCollection (), "help_forward");
	forward->setEnabled (false);
	print = KStdAction::print (this, SLOT (slotPrint ()), actionCollection (), "print_help");
	print->setText (i18n ("Print Help"));
	addCommonActions (actionCollection ());

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);
	setXMLFile ("rkhelpwindow.rc");

	khtmlpart->insertChildClient (this);

	setIcon (SmallIcon ("help"));
	setCaption (i18n ("R Help"));
}

RKHelpWindow::~RKHelpWindow () {
	RK_TRACE (APP);
	delete khtmlpart;
	khtmlpart = 0;	// in case we try to redelete in a parent class
}

bool RKHelpWindow::openURL (const KURL &url) {
	RK_TRACE (APP);

	bool ok = true;
	if (url.protocol () == "rkward") {
		if (url.host () == "component") {
			ok = renderRKHelp (url);
		} else if (url.host () == "rhelp") {
			// TODO: find a nice solution to render this in the current window
			RKHelpSearchWindow::mainHelpSearch ()->getFunctionHelp (url.path ().mid (1));
			return true;
		} else if (url.host () == "page") {
			ok = renderRKHelp (url);
		}

		if (!ok) {
			khtmlpart->begin (url);
			khtmlpart->write ("<html><body><h1>" + i18n ("Page does not exist or is broken") + "</h1></body></html>");
			khtmlpart->end ();
		}
	
		changeURL (url);
		return ok;
	} else {
		return (RKHTMLWindow::openURL (url));
	}
}

bool RKHelpWindow::renderRKHelp (const KURL &url) {
	RK_TRACE (APP);

	if (url.protocol () != "rkward") {
		RK_ASSERT (false);
		return (false);
	}

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
			if (element.isNull ()) break;
			help_file_name = component_xml->getStringAttribute (element, "file", QString::null, DL_ERROR);
			if (help_file_name.isNull ()) break;
			help_file_name = QFileInfo (chandle->getFilename ()).dir (true).filePath (help_file_name);
		} else {
			help_file_name = help_base_dir + url.path () + ".rkh";
		}
		RK_DO (qDebug ("rendering help page for local file %s", help_file_name.latin1 ()), APP, DL_DEBUG);

		// open help file
		QDomElement help_doc_element = help_xml->openXMLFile (help_file_name, DL_ERROR);
		if (help_doc_element.isNull ()) break;

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

		// fix all elements containing an "src" attribute
		QDir base_path (QFileInfo (help_file_name).dirPath (true));
		XMLChildList src_elements = help_xml->findElementsWithAttribute (help_doc_element, "src", QString (), true, DL_DEBUG);
		for (XMLChildList::iterator it = src_elements.begin (); it != src_elements.end (); ++it) {
			QString src = (*it).attribute ("src");
			if (KURL::isRelativeURL (src)) {
				src = "file://" + QDir::cleanDirPath (base_path.filePath (src));
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
		KURL url_copy = url;
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

QString RKHelpWindow::renderHelpFragment (QDomElement &fragment) {
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
	QTextOStream stream (&ret);
	for (QDomNode node = fragment.firstChild (); !node.isNull (); node = node.nextSibling ()) {
		node.save (stream, 0);
	}

	ret.prepend ("<p>");
	ret.append ("</p>");
	ret.replace ("\n\n", "</p>\n<p>");

	RK_DO (qDebug ("%s", ret.latin1 ()), APP, DL_DEBUG);
	return ret;
}

void RKHelpWindow::prepareHelpLink (QDomElement *link_element) {
	RK_TRACE (APP);

	link_element->setTagName ("a");
	if (link_element->text ().isEmpty ()) {
		QString text;
		KURL url = link_element->attribute ("href");
		if (url.protocol () == "rkward") {
			if (url.host () == "component") {
				RKComponentHandle *chandle = componentPathToHandle (url.path ());
				if (chandle) text = chandle->getLabel ();
			} else if (url.host () == "rhelp") {
				text = i18n ("R Reference on '%1'").arg (url.path ().mid (1));
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
				RK_DO (qDebug ("Broken reference to %s", url.path ().latin1 ()), APP, DL_WARNING);
			}

			link_element->appendChild (link_element->ownerDocument ().createTextNode (text));
		}
	}
}

RKComponentHandle *RKHelpWindow::componentPathToHandle (QString path) {
	RK_TRACE (APP);

	QStringList path_segments = QStringList::split ('/', path);
	if (path_segments.count () > 2) return 0;
	if (path_segments.count () < 1) return 0;
	if (path_segments.count () == 1) path_segments.push_front ("rkward");
	RK_ASSERT (path_segments.count () == 2);
	return (RKComponentMap::getComponentHandle (path_segments.join ("::")));
}

QString RKHelpWindow::startSection (const QString &name, const QString &title, const QString &shorttitle, QStringList *anchors, QStringList *anchor_names) {
	QString ret = "<a name=\"" + name + "\">";
	ret.append ("<h2>" + title + "</h2>\n");
	anchors->append (name);
	if (!shorttitle.isNull ()) anchor_names->append (shorttitle);
	else anchor_names->append (title);
	return (ret);
}


#include "rkhtmlwindow.moc"
