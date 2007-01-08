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

#include "../rkglobals.h"
#include "../khelpdlg.h"
#include "../rkward.h"
#include "../settings/rksettingsmodulegeneral.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/xmlhelper.h"
#include "../plugin/rkcomponentmap.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkworkplaceview.h"
#include "../debug.h"

RKHTMLWindow::RKHTMLWindow (QWidget *parent) : RKMDIWindow (parent, RKMDIWindow::HelpWindow) {
	RK_TRACE (APP);
	scroll_position=0;
	
	khtmlpart = new KHTMLPart (this, 0, 0, 0, KHTMLPart::BrowserViewGUI);
	khtmlpart->setSelectable (true);
	setFocusProxy (khtmlpart->widget ());
	
	khtmlpart->widget ()->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *pLayout = new QHBoxLayout (this);
	pLayout->addWidget (khtmlpart->widget ());

	// We have to connect this in order to allow browsing.
	connect (khtmlpart->browserExtension (), SIGNAL(openURLRequest (const KURL &, const KParts::URLArgs &)), this, SLOT (slotOpenURLRequest (const KURL &, const KParts::URLArgs &)));
	connect (khtmlpart, SIGNAL (completed ()), this, SLOT (loadDone ()));

	url_history.setAutoDelete (true);
	back = forward = print = 0;		// initialization done in subclasses
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

KParts::Part *RKHTMLWindow::getPart () {
	RK_TRACE (APP);
	return khtmlpart;
}

void RKHTMLWindow::addCommonActions (KActionCollection *action_collection) {
	RK_TRACE (APP);

	// enable copy
	KStdAction::copy (khtmlpart->browserExtension (), SLOT (copy ()), action_collection, "copy");
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
	khtmlpart->view()->setContentsPos (0, scroll_position);
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

	outputFlush = new KAction (i18n ("&Flush Output"), 0, 0, this, SLOT (flushOutput ()), actionCollection (), "output_flush");
	outputRefresh = new KAction (i18n ("&Refresh Output"), 0, 0, this, SLOT (refreshOutput ()), actionCollection (), "output_refresh");
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
		out_file.remove ();
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

	// TODO: real error handling
	bool ok = true;
	qDebug ("here1 %s", url.prettyURL ().latin1 ());
	if (url.protocol () == "rkward") {
		if (url.host () == "component") {
			ok = renderRKHelp (url);
		} else if (url.host () == "rhelp") {
			// TODO: find a nice solution to render this in the current window
			RKGlobals::helpDialog ()->getFunctionHelp (url.path ().mid (1));
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

	while (true) {		// dirty hack to streamline exit code: breaking from this while, before success is set to true will cause the XMLHelpers to be deleted, and false returned.
		RKComponentHandle *chandle = 0;
		QString help_file_name;
		QDomElement element;
		QDomElement component_doc_element;
		QString help_base_dir;

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
			help_base_dir = RKCommonFunctions::getRKWardDataDir () + "pages/";

			help_file_name = help_base_dir + url.path () + ".rkh";
		}
		qDebug ("%s", help_file_name.latin1 ());

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
		khtmlpart->write ("<html><head><title>" + page_title + "</title></head>\n<body>\n<h1>" + page_title + "</h1>\n");

		// fix all elements containing an "src" attribute
		QDir base_path (QFileInfo (help_file_name).dirPath (true));
		XMLChildList src_elements = help_xml->findElementsWithAttribute (help_doc_element, "src", QString (), true, DL_DEBUG);
		for (XMLChildList::iterator it = src_elements.begin (); it != src_elements.end (); ++it) {
			QString src = (*it).attribute ("src");
			if (KURL::isRelativeURL (src)) {
				src = QDir::cleanDirPath (base_path.filePath (src));
				(*it).setAttribute ("src", src);
			}
		}

		// render the sections
		element = help_xml->getChildElement (help_doc_element, "summary", DL_INFO);
		if (!element.isNull ()) {
			khtmlpart->write ("<h2>" + i18n ("Summary") + "</h2>\n");
			khtmlpart->write (renderHelpFragment (element));
		}

		element = help_xml->getChildElement (help_doc_element, "usage", DL_INFO);
		if (!element.isNull ()) {
			khtmlpart->write ("<h2>" + i18n ("Usage") + "</h2>\n");
			khtmlpart->write (renderHelpFragment (element));
		}

		XMLChildList section_elements = help_xml->getChildElements (help_doc_element, "section", DL_INFO);
		for (XMLChildList::iterator it = section_elements.begin (); it != section_elements.end (); ++it) {
			khtmlpart->write ("<h2>" + help_xml->getStringAttribute (*it, "title", QString (), DL_WARNING) + "</h2>\n");
			khtmlpart->write (renderHelpFragment (*it));
		}

		if (for_component) {
			element = help_xml->getChildElement (help_doc_element, "settings", DL_INFO);
			if (!element.isNull ()) {
				khtmlpart->write ("<h2>" + i18n ("GUI settings") + "</h2>\n");
				XMLChildList setting_elements = help_xml->getChildElements (element, QString (), DL_WARNING);
				for (XMLChildList::iterator it = setting_elements.begin (); it != setting_elements.end (); ++it) {
					if ((*it).tagName () == "setting") {
						QString id = help_xml->getStringAttribute (*it, "id", QString (), DL_WARNING);
						QString title = help_xml->getStringAttribute (*it, "title", QString (), DL_WARNING);
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

		element = help_xml->getChildElement (help_doc_element, "related", DL_INFO);
		if (!element.isNull ()) {
			khtmlpart->write ("<h2>" + i18n ("Related functions and pages") + "</h2>\n");
			khtmlpart->write (renderHelpFragment (element));
		}

		khtmlpart->write ("</body></html>\n");
		khtmlpart->end ();
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

	qDebug ("%s", ret.latin1 ());
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
				else {
					text = i18n ("BROKEN REFERENCE");
					RK_DO (qDebug ("Broken component reference to %s", url.path ().latin1 ()), APP, DL_WARNING);
				}
			} else if (url.host () == "rhelp") {
				text = i18n ("R Reference on '%1'").arg (url.path ().mid (1));
			} else if (url.host () == "page") {
				text = "TODO: some help page";
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

#include "rkhtmlwindow.moc"
