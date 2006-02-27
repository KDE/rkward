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
#include <qtimer.h>
#include <qapplication.h>

#include <klocale.h>

#include "../rkward.h"
#include "../rkeditormanager.h"
#include "../scriptbackends/phpbackend.h"
#include "../misc/rkerrordialog.h"
#include "../misc/xmlhelper.h"
#include "../rkcommandeditor.h"
#include "../settings/rksettingsmoduleplugins.h"
#include "../rbackend/rinterface.h"

// plugin-widgets
#include "rkpluginwidget.h"
#include "rkvarselector.h"
#include "rkvarslot.h"
#include "rktext.h"
#include "rkradio.h"
#include "rkcheckbox.h"
#include "rkpluginspinbox.h"
#include "rkformula.h"
#include "rknote.h"
#include "rkinput.h"
#include "rkpluginbrowser.h"

#include "../rkglobals.h"

#include "../debug.h"

#define R_FOR_PHP_FLAG 1

#define BACKEND_DONT_CARE 0
#define BACKEND_FOR_CODE_WINDOW 1
#define BACKEND_FOR_SUBMISSION 2

RKPlugin::RKPlugin(const QString &filename) : QWidget () {
	RK_TRACE (PLUGIN);

	backend = 0;
	script_backend_chain = 0;
	main_widget = 0;
	RKPlugin::filename = filename;

	// open the main description file for parsing
	XMLHelper* xml = XMLHelper::getStaticHelper ();
	QDomElement doc_element = xml->openXMLFile (filename, DL_ERROR);
	if (xml->highestError () >= DL_ERROR) {
		// TODO: inform user
		return;
	}

	// create an error-dialog
	error_dialog = new RKErrorDialog (i18n ("The R-backend has reported one or more error(s) while processing the plugin '%1'. This may lead to an incorrect ouput and is likely due to a bug in the plugin.\nA transcript of the error message(s) is shown below.").arg (filename), i18n ("R-Error"), false);
	
	// initialize the PHP-backend with the code-template
	should_updatecode=false;
	QDomElement element = xml->getChildElement (doc_element, "code", DL_WARNING);
	QString dummy = QFileInfo (filename).dirPath () + "/" + xml->getStringAttribute (element, "file", "code.php", DL_WARNING);
	backend = new PHPBackend ();
	connect (backend, SIGNAL (commandDone (int)), this, SLOT (backendCommandDone (int)));
	connect (backend, SIGNAL (idle ()), this, SLOT (backendIdle ()));
	connect (backend, SIGNAL (requestValue (const QString&)), this, SLOT (getValue (const QString&)));
	connect (backend, SIGNAL (requestRCall (const QString&)), this, SLOT (doRCall (const QString&)));
	connect (backend, SIGNAL (requestRVector (const QString&)), this, SLOT (getRVector (const QString&)));
	connect (backend, SIGNAL (haveError ()), this, SLOT (cancel ()));
	if (!backend->initialize (dummy)) return;

	// create the main gui
	sizer_grid = new QGridLayout (this, 1, 1);
	
	connect (qApp, SIGNAL (aboutToQuit ()), this, SLOT (cancel ()));
	
	update_timer = new QTimer (this);
	connect (update_timer, SIGNAL (timeout ()), this, SLOT (doChangeUpdate ()));
	buildGUI (&doc_element, 0);
}

RKPlugin::~RKPlugin(){
	RK_TRACE (PLUGIN);

	delete error_dialog;
	delete codeDisplay;
	delete backend;
	RKGlobals::rInterface ()->closeChain (script_backend_chain);
}

void RKPlugin::closeEvent (QCloseEvent *e) {
	RK_TRACE (PLUGIN);

	e->accept ();
	try_destruct ();
}

void RKPlugin::switchInterfaces () {
	RK_TRACE (PLUGIN);

	if (num_pages <= 1) {
		buildGUI (0, 1);
	} else {
		buildGUI (0, 2);
	}
}

void RKPlugin::buildGUI (QDomElement *doc_element, int type_override) {
	RK_TRACE (PLUGIN);

	QDomElement dummy;	// as this might become the document element (see below), its scope has to reach until the end of this function!
	XMLHelper* xml = XMLHelper::getStaticHelper ();
	if (!doc_element) {
		dummy = xml->openXMLFile (filename, DL_ERROR);
		// no error should have occured at this stage, as this has already been tried in the constructor at least once.
		doc_element = &dummy;
	}

	num_pages = 0;
	current_page = 0;
	
	if (main_widget) {
		hide ();
		sizer_grid->removeChild (main_widget);
		delete main_widget;
	}
	
	widgets.clear ();
	page_map.clear ();
	
	// find available interfaces
	QDomElement dialog_element;
	QDomElement wizard_element;
	bool wizard_recommended = false;
	
	QDomNode n = doc_element->firstChild ();
	while (!n.isNull ()) {
		QDomElement e = n.toElement ();
		if (e.tagName () == "dialog") {
			dialog_element = e;
		} else if (e.tagName () == "wizard") {
			if (e.attribute ("recommended") == "true") {
				wizard_recommended = true;
			}
			wizard_element = e;
		}
		n = n.nextSibling ();
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
	
	// initialize widgets
	WidgetsMap::Iterator it;
	for (it = widgets.begin(); it != widgets.end(); ++it) {
		it.data ()->initialize ();
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
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	setCaption (xml->getStringAttribute (dialog_element, "label", i18n ("No title"), DL_WARNING));

	QGridLayout *main_grid = new QGridLayout (main_widget, 1, 1);
	QSplitter *splitter = new QSplitter (QSplitter::Vertical, main_widget);
	main_grid->addWidget (splitter, 0, 0);
	QWidget *upper_widget = new QWidget (splitter);
	
	QHBoxLayout *hbox = new QHBoxLayout (upper_widget, RKGlobals::marginHint (), RKGlobals::spacingHint ());
	QVBoxLayout *vbox = new QVBoxLayout (hbox, RKGlobals::spacingHint ());

	// default layout is in vertical
	buildStructure (dialog_element, vbox, upper_widget);
	
	// build standard elements
	// lines
	QFrame *line;
	line = new QFrame (upper_widget);
	line->setFrameShape (QFrame::VLine);
	line->setFrameShadow (QFrame::Plain);	
	hbox->addWidget (line);

	// buttons
	vbox = new QVBoxLayout (hbox, RKGlobals::spacingHint ());
	okButton = new QPushButton ("Submit", upper_widget);
	connect (okButton, SIGNAL (clicked ()), this, SLOT (ok ()));
	vbox->addWidget (okButton);
	
	cancelButton = new QPushButton ("Close", upper_widget);
	connect (cancelButton, SIGNAL (clicked ()), this, SLOT (cancel ()));
	vbox->addWidget (cancelButton);
	vbox->addStretch (1);
	
	helpButton = new QPushButton ("Help", upper_widget);
	connect (helpButton, SIGNAL (clicked ()), this, SLOT (help ()));
	vbox->addWidget (helpButton);
	
	if (wizard_available) {
		switchButton = new QPushButton ("Use Wizard", upper_widget);
		connect (switchButton, SIGNAL (clicked ()), this, SLOT (switchInterfaces ()));
		vbox->addWidget (switchButton);
	}
	vbox->addStretch (2);
	
	toggleCodeButton = new QPushButton ("Code", upper_widget);
	toggleCodeButton->setToggleButton (true);
	toggleCodeButton->setOn (true);
	connect (toggleCodeButton, SIGNAL (clicked ()), this, SLOT (toggleCode ()));
	vbox->addWidget (toggleCodeButton);
	
	// text-fields
	QWidget *lower_widget = new QWidget (splitter);
	
	vbox = new QVBoxLayout (lower_widget, RKGlobals::spacingHint ());
	codeDisplay = new RKCommandEditor (lower_widget, true);
	vbox->addWidget (codeDisplay);

	num_pages = 1;


  // inserting connection for enabling widgets - or not...
  // work only if widget are on the same page
  // valid for every widget excet formula (too weird)
    Dependancies::Iterator it;
  for (it = dependant.begin(); it != dependant.end(); ++it) {
  RK_DO (qDebug ("id : %s", it.key().latin1 ()), PLUGIN, DL_DEBUG);
  if  (it.data() != "#free#"){
    WidgetsMap::iterator master ;
      for (master = widgets.begin(); master != widgets.end(); ++master) {
//        qDebug ("I depend on  : %s", it.data().latin1 ()) ;
//        qDebug ("Am I your master ? I am : %s", master.key().latin1 ()) ;
        if (master.data ()->type () == CHECKBOX_WIDGET && master.key() == it.data() ){
//            qDebug ("I am your master I am : %s", master.key().latin1 ()) ;
            WidgetsMap::iterator slave = widgets.find (it.key());
            if (slave != widgets.end ()  ) {
              if(  ! (RKCheckBox *) master.data()->isOk ){
                slave.data()->setEnabled(false );
                };
               RK_DO (qDebug ("Connecting %s to %s",slave.key().latin1(),master.key().latin1()), PLUGIN, DL_DEBUG);
               connect( (RKCheckBox *) master.data(), SIGNAL(clicked()) ,(RKCheckBox *)   slave.data(), SLOT(slotActive()));
               RK_DO (qDebug ("You are very right to choose me OK"), PLUGIN, DL_DEBUG);
               };
          };
          if (master.data ()->type () == RADIO_WIDGET  ){
//          qDebug ("Is your master a radio ?") ;
//          qDebug ("I look if %s is in %s", it.data().latin1() , master.key().latin1 ()) ;
            RKRadio  * temp = (RKRadio * ) master.data() ;
            QButton * sol = (QButton *) temp->findLabel(it.data()) ;
            if ( sol != 0){
            WidgetsMap::iterator slave = widgets.find (it.key());
            if (slave != widgets.end ()  ) {
//              qDebug ("Yes it is") ;
              if (! temp->isOk(it.data()) ) slave.data()->setEnabled(false );
              connect(  sol , SIGNAL(toggled(bool)) ,  slave.data(), SLOT(slotActive(bool)));
              };
            }
//            else  qDebug ("No it isn't") ;
         };
        };
      };
    };


}

void RKPlugin::buildWizard (const QDomElement &wizard_element, bool dialog_available) {
	RK_TRACE (PLUGIN);

	XMLHelper* xml = XMLHelper::getStaticHelper ();
	setCaption (xml->getStringAttribute (wizard_element, "label", i18n ("No title"), DL_WARNING));

	QGridLayout *main_grid = new QGridLayout (main_widget, 3, 4, RKGlobals::marginHint (), RKGlobals::spacingHint ());
	wizard_stack = new QWidgetStack (main_widget);
	main_grid->addMultiCellWidget (wizard_stack, 0, 0, 0, 3);

	XMLChildList pages = xml->getChildElements (wizard_element, "page", DL_ERROR);
	for (XMLChildList::const_iterator it = pages.begin (); it != pages.end (); ++it) {
		QWidget *page_widget = new QWidget (main_widget);
		QVBoxLayout *ilayout = new QVBoxLayout (page_widget);
		buildStructure ((*it), ilayout, page_widget);
		if (!num_pages) {
			if (dialog_available) {
				switchButton = new QPushButton ("Use Dialog", page_widget);
				ilayout->addWidget (switchButton);
				connect (switchButton, SIGNAL (clicked ()), this, SLOT (switchInterfaces ()));
			}
		}
		wizard_stack->addWidget (page_widget, num_pages++);
	}

	// build the last page
	QWidget *last_page = new QWidget (main_widget);
	QVBoxLayout *vbox = new QVBoxLayout (last_page, RKGlobals::spacingHint ());
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

  // inserting connection for enabling widgets - or not...
  // work only if widget are on the same page
  // valid for every widget excet formula (too weird)
  Dependancies::Iterator it;
  for (it = dependant.begin(); it != dependant.end(); ++it) {
  RK_DO (qDebug ("id : %s", it.key().latin1 ()), PLUGIN, DL_DEBUG);
  if  (it.data() != "#free#"){
    WidgetsMap::iterator master ;
      for (master = widgets.begin(); master != widgets.end(); ++master) {
//        qDebug ("I depend on  : %s", it.data().latin1 ()) ;
//        qDebug ("Am I your master ? I am : %s", master.key().latin1 ()) ;
        if (master.data ()->type () == CHECKBOX_WIDGET && master.key() == it.data() ){
//            qDebug ("I am your master I am : %s", master.key().latin1 ()) ;
            WidgetsMap::iterator slave = widgets.find (it.key());
            if (slave != widgets.end ()  ) {
              if(  ! (RKCheckBox *) master.data()->isOk ){
                slave.data()->setEnabled(false );
                };
               connect( (RKCheckBox *) master.data(), SIGNAL(clicked()) ,  slave.data(), SLOT(slotActive()));
               RK_DO (qDebug ("You are very right to choose me OK"), PLUGIN, DL_DEBUG);
               };
          };
          if (master.data ()->type () == RADIO_WIDGET  ){
//          qDebug ("Is your master a radio ?") ;
//          qDebug ("I look if %s is in %s", it.data().latin1() , master.key().latin1 ()) ;
            RKRadio  * temp = (RKRadio * ) master.data() ;
            QButton * sol = (QButton *) temp->findLabel(it.data()) ;
            if ( sol != 0){
            WidgetsMap::iterator slave = widgets.find (it.key());
            if (slave != widgets.end ()  ) {
//              qDebug ("Yes it is") ;
              if (! temp->isOk(it.data()) ) slave.data()->setEnabled(false );
              connect(  sol , SIGNAL(toggled(bool)) ,  slave.data(), SLOT(slotActive(bool)));
              };
            }
//            else  qDebug ("No it isn't") ;
         };
        };
      };
    };
}

void RKPlugin::buildStructure (const QDomElement &element, QBoxLayout *playout, QWidget *pwidget) {
	RK_TRACE (PLUGIN);

	QDomNodeList children = element.childNodes ();
	
	for (unsigned int i=0; i < children.count (); i++) {
		RKPluginWidget *widget = 0;
		QDomElement e = children.item (i).toElement ();
		
	    if (e.tagName () == "row") {
			buildStructure (e, new QHBoxLayout (playout, RKGlobals::spacingHint ()), pwidget);
		} else if (e.tagName () == "column") {
			buildStructure (e, new QVBoxLayout (playout, RKGlobals::spacingHint ()), pwidget);
		} else if (e.tagName () == "frame") {
			QVBoxLayout *layout = new QVBoxLayout (playout, RKGlobals::spacingHint ());	// just a container
			QGroupBox *box = new QGroupBox (1, Qt::Vertical, e.attribute ("label"), pwidget);
			layout->addWidget (box);
			QWidget *dummy = new QWidget (box);		// cumbersome workaround. Can this be done in a more straightforward way?
			QVBoxLayout *ilayout = new QVBoxLayout (dummy, RKGlobals::spacingHint ());
			buildStructure (e, ilayout, dummy);
		} else if (e.tagName () == "tabbook") {
			QTabWidget *tabbook = new QTabWidget (pwidget);
			QDomNodeList tabs = e.childNodes ();
			for (unsigned int t=0; t < tabs.count (); ++t) {
				QDomElement tab_e = tabs.item (t).toElement ();
				if (tab_e.tagName () == "tab") {
					QFrame *tabwidget = new QFrame (pwidget);
					QVBoxLayout *ilayout = new QVBoxLayout (tabwidget, RKGlobals::marginHint (), RKGlobals::spacingHint ());
					buildStructure (tab_e, ilayout, tabwidget);
					tabbook->addTab (tabwidget, tab_e.attribute ("label"));
				}
			}
			playout->addWidget (tabbook);
		} else if (e.tagName () == "varselector") {
			widget = new RKVarSelector (e, pwidget, this);
		} else if (e.tagName () == "radio") {
			widget = new RKRadio (e, pwidget, this);
		} else if (e.tagName () == "checkbox") {
			widget = new RKCheckBox (e, pwidget, this);
		} else if (e.tagName () == "spinbox") {
			widget = new RKPluginSpinBox (e, pwidget, this);
		} else if (e.tagName () == "varslot") {
			widget = new RKVarSlot (e, pwidget, this);
		} else if (e.tagName () == "formula") {
			widget = new RKFormula (e, pwidget, this);
		} else if (e.tagName () == "note") {
			widget = new RKNote (e, pwidget, this);
		} else if (e.tagName () == "browser") {
			widget = new RKPluginBrowser (e, pwidget, this);
		} else if (e.tagName () == "input") {
			widget = new RKInput (e, pwidget, this);
		} else {
			widget = new RKText (e, pwidget, this);
		}
		
		if (widget) {
			playout->addWidget (widget);
			registerWidget (widget, e.attribute ("id", "#noid#"), e.attribute("depend","#free#"),
      num_pages);
		}
	}
}

void RKPlugin::ok () {
	RK_TRACE (PLUGIN);

	if (current_page < (num_pages - 1)) {
		wizard_stack->raiseWidget (++current_page);
		backButton->setEnabled (true);
		if (current_page == (num_pages - 1)) {
			okButton->setText (i18n ("Submit"));
		}
		changed ();
	} else {
		RKGlobals::rInterface ()->issueCommand (new RCommand (current_code, RCommand::Plugin, QString::null, this));
		script_backend_chain = RKGlobals::rInterface ()->startChain ();
		backend->printout (BACKEND_DONT_CARE);
		backend->cleanup (BACKEND_FOR_SUBMISSION);
	}
}

void RKPlugin::back () {
	RK_TRACE (PLUGIN);

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
	RK_TRACE (PLUGIN);

	try_destruct ();
}

void RKPlugin::toggleCode () {
	RK_TRACE (PLUGIN);

	if (codeDisplay->isVisible ()) {
		codeDisplay->hide ();
	} else {
		codeDisplay->setText (current_code);
		codeDisplay->show ();
	}
}

void RKPlugin::try_destruct () {
	RK_TRACE (PLUGIN);

	if (!backend->isBusy ()) {
		delete this;
	} else {
		hide ();
		should_destruct = true;
	}
}

void RKPlugin::backendCommandDone (int flags) {
	RK_TRACE (PLUGIN);
	RK_DO (qDebug ("%d", flags), PLUGIN, DL_DEBUG);
	
	if (flags == BACKEND_DONT_CARE) {
		return;
	} else if (flags == BACKEND_FOR_CODE_WINDOW) {
		current_code = backend->retrieveOutput ();
		RK_DO (qDebug ("current_code %s", current_code.latin1 ()), PLUGIN, DL_DEBUG);
		backend->resetOutput ();
	} else if (flags == BACKEND_FOR_SUBMISSION) {
		RKGlobals::rInterface ()->issueCommand (new RCommand (backend->retrieveOutput (), RCommand::Plugin | RCommand::DirectToOutput, QString::null, this), script_backend_chain);
		backend->resetOutput ();
	}
}

void RKPlugin::backendIdle () {
	RK_TRACE (PLUGIN);
	script_backend_chain = RKGlobals::rInterface ()->closeChain (script_backend_chain);
	
	if (should_destruct) {
		try_destruct ();
		return;
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

	if (should_updatecode) {
		current_code = QString::null;
		codeDisplay->setText ("Processing. Please wait.");
		script_backend_chain = RKGlobals::rInterface ()->startChain ();
		backend->preprocess (BACKEND_DONT_CARE);
		backend->calculate (BACKEND_FOR_CODE_WINDOW);
		should_updatecode = false;
		okButton->setEnabled (false);
	} else {
		if (codeDisplay->isVisible () && (current_page == (num_pages - 1))) {
			codeDisplay->setText (current_code);
		}
	}
}

void RKPlugin::registerWidget (RKPluginWidget *widget, const QString &id , const QString &dep, int page) {
	RK_TRACE (PLUGIN);
	RK_DO (qDebug ("inserting widget %s",id.latin1()), PLUGIN, DL_DEBUG);

	widgets.insert (id, widget);
	page_map.insert (widget, page);
	dependant.insert (id,dep);
}

void RKPlugin::help () {
	RK_TRACE (PLUGIN);
	// TODO
}

void RKPlugin::doChangeUpdate () {
	RK_TRACE (PLUGIN);
	// trigger update for code-display
	if (current_page == (num_pages - 1)) {
		should_updatecode = true;
	}

	if (!backend->isBusy ()) {
		backendIdle ();
	} else {
		okButton->setEnabled (false);
	}
}

void RKPlugin::changed () {
	RK_TRACE (PLUGIN);
/* why don't we do the update right here? Two reasons:
	- several widgets may be updating in a chain, an each will emit a change signal. However, we only want to update once.
	- some widgets may be in a bad state, if the change-event was due to an RObject being deleted. These widgets should get a change to update before we try to get values from them */
	update_timer->start (0, true);
}

void RKPlugin::doRCall (const QString &call) {
	RK_TRACE (PLUGIN);
	RKGlobals::rInterface ()->issueCommand (new RCommand (call, RCommand::Plugin | RCommand::PluginCom, QString::null, this, R_FOR_PHP_FLAG), script_backend_chain);
}

void RKPlugin::getRVector (const QString &call) {
	RK_TRACE (PLUGIN);
	RKGlobals::rInterface ()->issueCommand (new RCommand (call, RCommand::Plugin | RCommand::PluginCom | RCommand::GetStringVector, QString::null, this, R_FOR_PHP_FLAG), script_backend_chain);
}

void RKPlugin::rCommandDone (RCommand *command) {
	RK_TRACE (PLUGIN);
	RK_DO (qDebug ("rCommandDone. Command-flags: %d", command->getFlags ()), PLUGIN, DL_DEBUG);

	if (command->hasError()) {
		error_dialog->newError (command->error());
	}
	if (command->getFlags() & R_FOR_PHP_FLAG) {
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
			backend->writeData (temp);
		} else {
			backend->writeData (command->output());
		}
	} else if (command->type () & RCommand::DirectToOutput) {
		RKGlobals::rkApp ()->newOutput ();
	}
}

void RKPlugin::getValue (const QString &id) {
	RK_TRACE (PLUGIN);
	backend->writeData (getVar (id));
}

QString RKPlugin::getVar (const QString &id) {
	RK_TRACE (PLUGIN);

	QString ident = id.section (".", 0, 0);
	RK_DO (qDebug("searching fo %s",ident.latin1()), PLUGIN, DL_DEBUG);
	
	RKPluginWidget *widget;
	if (widgets.contains (ident)) {
		widget = widgets[ident];
	} else {
		widget = 0;
		RK_DO (qDebug ("Couldn't find value for   $%s$    !", id.latin1 ()), PLUGIN, DL_ERROR);
		return ("#unavailable#");
	}

	return (widget->value (id.section (".", 1, 1)));
}

/** Returns a pointer to the varselector by that name (0 if not available) */
RKVarSelector *RKPlugin::getVarSelector (const QString &id) {
	RK_TRACE (PLUGIN);
	WidgetsMap::iterator it = widgets.find (id);
	if (it != widgets.end ()) {
		if (it.data ()->type () == VARSELECTOR_WIDGET) {
			return (RKVarSelector *) it.data ();
		}
	}

	RK_DO (qDebug ("%s", "failed to find varselector!"), PLUGIN, DL_ERROR);
	return 0;
}

RKVarSlot *RKPlugin::getVarSlot (const QString &id) {
	RK_TRACE (PLUGIN);
	WidgetsMap::iterator it = widgets.find (id);
	if (it != widgets.end ()) {
		if (it.data ()->type () == VARSLOT_WIDGET) {
			return (RKVarSlot *) it.data ();
		}
	}

	RK_DO (qDebug ("%s", "failed to find varselot!"), PLUGIN, DL_ERROR);
	return 0;
}

#include "rkplugin.moc"
