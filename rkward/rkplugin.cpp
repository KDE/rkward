/***************************************************************************
                          rkplugin.cpp  -  description
                             -------------------
    begin                : Wed Nov 6 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#include "rkplugin.h"

#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qmap.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtabwidget.h>
#include <qsplitter.h>
#include <qwidgetstack.h>
#include <qlabel.h>

#include <klocale.h>

#include "rkward.h"
#include "rkwarddoc.h"
#include "rcommand.h"
#include "phpbackend.h"
#include "rkerrordialog.h"
#include "rkcommandeditor.h"
#include "rksettingsmoduleplugins.h"

// plugin-widgets
#include "rkpluginwidget.h"
#include "rkvarselector.h"
#include "rkvarslot.h"
#include "rktext.h"
#include "rkradio.h"
#include "rkcheckbox.h"
#include "rkpluginspinbox.h"

#define FOR_PHP_FLAG 1

RKPlugin::RKPlugin(RKwardApp *parent, const QString &filename) : QWidget () {
	app = parent;
	backend = 0;
	php_backend_chain = 0;
	main_widget = 0;
	RKPlugin::filename = filename;
	
	// create an error-dialog
	error_dialog = new RKErrorDialog (i18n ("The R-backend has reported one or more error(s) while processing the plugin ") + caption () + i18n (". This may lead to an incorrect ouput and is likely due to a bug in the plugin.\nA transcript of the error message(s) is shown below."), i18n ("R-Error"), false);
	
	// initialize the PHP-backend with the code-template
	QString dummy = QFileInfo (QFile (filename)).dirPath () + "/code.php";
	backend = new PHPBackend ();
	if (!backend->initTemplate (dummy, this)) return;

	// create the main gui
	sizer_grid = new QGridLayout (this, 1, 1);
	buildGUI (0);
}

RKPlugin::~RKPlugin(){
	delete error_dialog;
	delete codeDisplay;
	delete backend;
	getApp ()->r_inter->closeChain (php_backend_chain);
}

void RKPlugin::closeEvent (QCloseEvent *e) {
	e->accept ();
	try_destruct ();
}

void RKPlugin::switchInterfaces () {
	if (num_pages <= 1) {
		buildGUI (1);
	} else {
		buildGUI (2);
	}
}

void RKPlugin::buildGUI (int type_override) {
	num_pages = 0;
	current_page = 0;
	
	if (main_widget) {
		hide ();
		sizer_grid->removeChild (main_widget);
		delete main_widget;
	}
	
	widgets.clear ();
	page_map.clear ();
	
	// open XML-file (TODO: remove code-duplication)
	int error_line, error_column;
	QString error_message, dummy;
	QDomDocument doc;
	QFile f(filename);
	if (!f.open(IO_ReadOnly))
		qDebug ("Could not open file for reading: " + filename);
	if (!doc.setContent(&f, false, &error_message, &error_line, &error_column)) {
		f.close();
		qDebug ("parsing-error in: " + filename);
		qDebug ("Message: " + error_message);
		qDebug ("Line: %d", error_line);
		qDebug ("Column: %d", error_column);
		return;
	}
	f.close();

	// find layout-section
	QDomElement element = doc.documentElement ();
	QDomNodeList children = element.elementsByTagName("entry");
	element = children.item (0).toElement ();
	setCaption (element.attribute ("label", "untitled"));

	// find available interfaces
	QDomElement dialog_element;
	QDomElement wizard_element;
	bool wizard_recommended = false;
	
	children = doc.documentElement ().childNodes ();;
	for (unsigned int n=0; n < children.count (); ++n) {
		QDomElement e = children.item (n).toElement ();
		if (e.tagName () == "dialog") {
			dialog_element = e;
		} else if (e.tagName () == "wizard") {
			if (e.attribute ("recommended") == "true") {
				wizard_recommended = true;
			}
			wizard_element = e;
		}
	}

	main_widget = new QWidget (this);
	sizer_grid->addWidget (main_widget, 0, 0);
	
	// select best interface, construct and show
	if (type_override == 1) {
		buildWizard (wizard_element, !dialog_element.isNull ());
	} else if (type_override == 2) {
		buildDialog (dialog_element, !wizard_element.isNull ());
	} else {
		if (RKSettingsModulePlugins::getInterfacePreference () == RKSettingsModulePlugins::PreferRecommended) {
			if (wizard_recommended || dialog_element.isNull ()) {
				buildWizard (wizard_element, !dialog_element.isNull ());
			} else {
				buildDialog (dialog_element, !wizard_element.isNull ());
			}
		} else if (RKSettingsModulePlugins::getInterfacePreference () == RKSettingsModulePlugins::PreferDialog) {
			if (!dialog_element.isNull ()) {
				buildDialog (dialog_element, !wizard_element.isNull ());
			} else {
				buildWizard (wizard_element, !dialog_element.isNull ());
			}
		} else {
			if (!wizard_element.isNull ()) {
				buildWizard (wizard_element, !dialog_element.isNull ());
			} else {
				buildDialog (dialog_element, !wizard_element.isNull ());
			}
		}
	}
	show ();
	resize (sizeHint ());

	// keep it alive
	should_destruct = false;
	should_updatecode = true;

	// initialize code/warn-views
	changed ();
}

void RKPlugin::buildDialog (const QDomElement &dialog_element, bool wizard_available) {
	qDebug ("%s", "buildDialog");
		
	QGridLayout *main_grid = new QGridLayout (main_widget, 1, 1);
	QSplitter *splitter = new QSplitter (QSplitter::Vertical, main_widget);
	main_grid->addWidget (splitter, 0, 0);
	QWidget *upper_widget = new QWidget (splitter);
	
	QGridLayout *grid = new QGridLayout (upper_widget, 1, 3, 6);
	QVBoxLayout *vbox = new QVBoxLayout (grid);

	// default layout is in vertical	
	buildStructure (dialog_element, vbox, upper_widget);

	// build standard elements
	// lines
	QFrame *line;
	line = new QFrame (upper_widget);
	line->setFrameShape (QFrame::VLine);
	line->setFrameShadow (QFrame::Plain);	
	grid->addWidget (line, 0, 1);

	// buttons
	vbox = new QVBoxLayout (0, 0, 6);
	okButton = new QPushButton ("Submit", upper_widget);
	connect (okButton, SIGNAL (clicked ()), this, SLOT (ok ()));
	cancelButton = new QPushButton ("Close", upper_widget);
	connect (cancelButton, SIGNAL (clicked ()), this, SLOT (cancel ()));
	helpButton = new QPushButton ("Help", upper_widget);
	connect (helpButton, SIGNAL (clicked ()), this, SLOT (help ()));
	if (wizard_available) {
		switchButton = new QPushButton ("Use Wizard", upper_widget);
		connect (switchButton, SIGNAL (clicked ()), this, SLOT (switchInterfaces ()));
	}
	toggleCodeButton = new QPushButton ("Code", upper_widget);
	toggleCodeButton->setToggleButton (true);
	toggleCodeButton->setOn (true);
	connect (toggleCodeButton, SIGNAL (clicked ()), this, SLOT (toggleCode ()));
	vbox->addWidget (okButton);
	vbox->addWidget (cancelButton);
	vbox->addStretch (1);
	vbox->addWidget (helpButton);
	vbox->addWidget (switchButton);
	vbox->addStretch (2);
	vbox->addWidget (toggleCodeButton);
	grid->addLayout (vbox, 0, 2);
	
	// text-fields
	QWidget *lower_widget = new QWidget (splitter);
	
	vbox = new QVBoxLayout (lower_widget, 6);
	codeDisplay = new RKCommandEditor (lower_widget, true);
	vbox->addWidget (codeDisplay);

	num_pages = 1;
}

void RKPlugin::buildWizard (const QDomElement &wizard_element, bool dialog_available) {
	qDebug ("%s", "buildWizard");

	QGridLayout *main_grid = new QGridLayout (main_widget, 3, 4);
	wizard_stack = new QWidgetStack (main_widget);
	main_grid->addMultiCellWidget (wizard_stack, 0, 0, 0, 3);
	
	QDomNodeList pages = wizard_element.childNodes ();
	for (unsigned int p=0; p < pages.count (); ++p) {
		QDomElement page = pages.item (p).toElement ();
		if (page.tagName () == "page") {
			QWidget *page_widget = new QWidget (main_widget);
			QVBoxLayout *ilayout = new QVBoxLayout (page_widget);
			buildStructure (page, ilayout, page_widget);
			if (!num_pages) {
				if (dialog_available) {
					switchButton = new QPushButton ("Use Dialog", page_widget);
					ilayout->addWidget (switchButton);
					connect (switchButton, SIGNAL (clicked ()), this, SLOT (switchInterfaces ()));
				}
			}
			wizard_stack->addWidget (page_widget, num_pages++);
		}
	}

	// build the last page
	QWidget *last_page = new QWidget (main_widget);
	QVBoxLayout *vbox = new QVBoxLayout (last_page, 6);
	QLabel *label = new QLabel (i18n ("Below you can see the command(s) corresponding to the settings you made. Click 'Submit' to run the command(s)."), last_page);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	codeDisplay = new RKCommandEditor (last_page, true);
	vbox->addWidget (label);
	vbox->addWidget (codeDisplay);
	wizard_stack->addWidget (last_page, num_pages++);

	// build standard elements
	// lines
	QFrame *line;
	line = new QFrame (main_widget);
	line->setFrameShape (QFrame::HLine);
	line->setFrameShadow (QFrame::Plain);	
	main_grid->addMultiCellWidget (line, 1, 1, 0, 3);

	// buttons
	cancelButton = new QPushButton ("Cancel", main_widget);
	main_grid->addWidget (cancelButton, 2, 0, Qt::AlignLeft);
	helpButton = new QPushButton ("Help", main_widget);
	main_grid->addWidget (helpButton, 2, 1, Qt::AlignLeft);
	backButton = new QPushButton ("< Back", main_widget);
	backButton->setEnabled (false);
	main_grid->addWidget (backButton, 2, 2, Qt::AlignRight);
	okButton = new QPushButton ("Next >", main_widget);
	main_grid->addWidget (okButton, 2, 3, Qt::AlignRight);
	connect (okButton, SIGNAL (clicked ()), this, SLOT (ok ()));
	connect (backButton, SIGNAL (clicked ()), this, SLOT (back ()));
	connect (cancelButton, SIGNAL (clicked ()), this, SLOT (cancel ()));
	connect (helpButton, SIGNAL (clicked ()), this, SLOT (help ()));
	
	wizard_stack->raiseWidget (0);
}

void RKPlugin::buildStructure (const QDomElement &element, QBoxLayout *playout, QWidget *pwidget) {
	RKPluginWidget *widget = 0;

	QDomNodeList children = element.childNodes ();
	
	for (unsigned int i=0; i < children.count (); i++) {
		QDomElement e = children.item (i).toElement ();
		
	    if (e.tagName () == "row") {
			buildStructure (e, new QHBoxLayout (playout, 6), pwidget);
		} else if (e.tagName () == "column") {
			buildStructure (e, new QVBoxLayout (playout, 6), pwidget);
		} else if (e.tagName () == "frame") {
			QVBoxLayout *layout = new QVBoxLayout (playout, 6);	// just a container
			QGroupBox *box = new QGroupBox (1, Qt::Vertical, e.attribute ("label"), pwidget);
			layout->addWidget (box);
			QWidget *dummy = new QWidget (box);		// cumbersome workaround. Can this be done in a more straightforward way?
			QVBoxLayout *ilayout = new QVBoxLayout (dummy, 6);
			buildStructure (e, ilayout, dummy);
		} else if (e.tagName () == "tabbook") {
			QTabWidget *tabbook = new QTabWidget (pwidget);
			playout->addWidget (tabbook);
			QDomNodeList tabs = e.childNodes ();
			for (unsigned int t=0; t < tabs.count (); ++t) {
				QDomElement tab_e = tabs.item (t).toElement ();
				if (tab_e.tagName () == "tab") {
					QWidget *tabwidget = new QWidget (tabbook);
					QVBoxLayout *ilayout = new QVBoxLayout (tabwidget);
					buildStructure (tab_e, ilayout, tabwidget);
					tabbook->addTab (tabwidget, tab_e.attribute ("label"));
				}
			}
		} else if (e.tagName () == "varselector") {
			widget = new RKVarSelector (e, pwidget, this, playout);
		} else if (e.tagName () == "radio") {
			widget = new RKRadio (e, pwidget, this, playout);
		} else if (e.tagName () == "checkbox") {
			widget = new RKCheckBox (e, pwidget, this, playout);
		} else if (e.tagName () == "spinbox") {
			widget = new RKPluginSpinBox (e, pwidget, this, playout);
		} else if (e.tagName () == "varslot") {
			widget = new RKVarSlot (e, pwidget, this, playout);
		} else {
			widget = new RKText (e, pwidget, this, playout);
		}
		
		if (widget) {
			registerWidget (widget, e.attribute ("id", "#noid#"), num_pages);
		}
	}
}

void RKPlugin::ok () {
	if (current_page < (num_pages - 1)) {
		wizard_stack->raiseWidget (++current_page);
		backButton->setEnabled (true);
		if (current_page == (num_pages - 1)) {
			okButton->setText (i18n ("Submit"));
		}
		changed ();
	} else {
		getApp ()->getDocument ()->syncToR ();
		getApp ()->r_inter->issueCommand (new RCommand (current_code, RCommand::Plugin, "", this, SLOT (gotRResult (RCommand *))));
		php_backend_chain = getApp ()->r_inter->startChain ();
		backend->callFunction ("printout (); cleanup ();", true);
	}
}

void RKPlugin::back () {
	if (current_page > 0) {
		wizard_stack->raiseWidget (--current_page);
		if (!current_page) {
			backButton->setEnabled (false);
		}
		okButton->setText (i18n ("Next >"));
		okButton->setEnabled (true);
	}
}

void RKPlugin::cancel () {
	try_destruct ();
}

void RKPlugin::toggleCode () {
	if (codeDisplay->isVisible ()) {
		codeDisplay->hide ();
	} else {
		codeDisplay->setText (current_code);
		codeDisplay->show ();
	}
}

void RKPlugin::newOutput () {
	app->newOutput();
}

void RKPlugin::try_destruct () {
	qDebug ("try_destruct");
	if (!backend->isBusy ()) {
		delete this;
	} else {
		hide ();
		should_destruct = true;
	}
}

bool RKPlugin::backendIdle () {
	qDebug ("backendIdle");
	getApp ()->r_inter->closeChain (php_backend_chain);
	php_backend_chain = 0;
	
	if (should_destruct) {
		try_destruct ();
		return true;
	}

	if (should_updatecode) {
		current_code = "";
		codeDisplay->setText ("Processing. Please wait.");
		php_backend_chain = getApp ()->r_inter->startChain ();
		backend->callFunction ("preprocess (); calculate ();");
		should_updatecode = false;
		okButton->setEnabled (false);
		return false;	// backend is not idle anymore, now
	} else {
		if (codeDisplay->isVisible () && (current_page == (num_pages - 1))) {
			codeDisplay->setText (current_code);
		}
	}
	
	return false;
}

void RKPlugin::registerWidget (RKPluginWidget *widget, const QString &id, int page) {
	widgets.insert (id, widget);
	page_map.insert (widget, page);
}

void RKPlugin::help () {
	// TODO
}

void RKPlugin::changed () {
	// trigger update for code-display
	if (current_page == (num_pages - 1)) {
		should_updatecode = true;
	}

	// check whether everything is satisfied and fetch any complaints
	bool ok = true;
	QString warn;
	PageMap::Iterator it;
	for (it = page_map.begin(); it != page_map.end(); ++it) {
		if (it.data () <= current_page) {
			if (!it.key ()->isSatisfied ()) {
				ok = false;
			}
		}
		//warn.append (it.key ()->complaints ());
	}
	okButton->setEnabled (ok);
	//warn.truncate (warn.length () -1);

	if (!backend->isBusy ()) {
		backendIdle ();
	} else {
		okButton->setEnabled (false);
	}
}

void RKPlugin::updateCode (const QString &text) {
	current_code.append (text);
}

void RKPlugin::doRCall (const QString &call) {
	getApp ()->r_inter->issueCommand (new RCommand (call, RCommand::Plugin | RCommand::PluginCom, "", this, SLOT (gotRResult (RCommand *)), FOR_PHP_FLAG), php_backend_chain);
}

void RKPlugin::getRVector (const QString &call) {
	getApp ()->r_inter->issueCommand (new RCommand (call, RCommand::Plugin | RCommand::PluginCom | RCommand::GetStringVector, "", this, SLOT (gotRResult (RCommand *)), FOR_PHP_FLAG), php_backend_chain);
}

void RKPlugin::gotRResult (RCommand *command) {
	if (command->hasError()) {
		error_dialog->newError (command->error());
	}
	if (command->getFlags() & FOR_PHP_FLAG) {
		// since R-calls are asynchronous, we need to expect incoming data after the backend has been torn down
		if (!backend) return;
		if ((command->type () & RCommand::GetStringVector)) {
			QString temp;
// TODO: can this painful process be optimized?
			for (int i = 0; i < command->stringVectorLength (); ++i) {
				if (i) {
					temp.append ("\t");
				}
				temp.append (command->getStringVector ()[i]);
			}
			backend->gotRCallResult (temp);
		} else {
			backend->gotRCallResult (command->output());
		}
	}
}

QString RKPlugin::getVar (const QString &id) {
	QString ident = id.section (".", 0, 0);
	RKPluginWidget *widget;
	if (widgets.contains (ident)) {
		widget = widgets[ident];
	} else {
		widget = 0;
		qDebug ("Couldn't find value for $" + id +"$!");
		return ("#unavailable#");
	}

	return (widget->value (id.section (".", 1, 1)));
}

/** Returns a pointer to the varselector by that name (0 if not available) */
RKVarSelector *RKPlugin::getVarSelector (const QString &id) {
	RKPluginWidget *selector = widgets[id];
	if (selector->isVarSelector ()) {
		return (RKVarSelector *) selector;
	}

	qDebug ("failed to find varselector!");
	return 0;
}
