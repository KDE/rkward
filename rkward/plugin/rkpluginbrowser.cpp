
#include "rkpluginbrowser.h"

#include <qdom.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qstring.h>
#include <qpushbutton.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kurl.h>

#include "rkplugin.h"
#include "../rkglobals.h"
#include "../debug.h"

// /////////////////////

RKPluginBrowser::RKPluginBrowser(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	RK_TRACE (PLUGIN);

	vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	label = new QLabel (element.attribute ("label", "Enter your text"), this);
	textedit = new QTextEdit ( element.attribute ("initial", QString::null), QString::null,this, element.attribute ("id")) ;
	button = new QPushButton (i18n("Browse..."),this);
//	size = element.attribute ("size", "small");
	vbox->addWidget (label);
	vbox->addWidget (textedit);
	vbox->addWidget (button);
	type = element.attribute("type","file") ;
	filter = element.attribute("filter", QString::null) ;
	connect(textedit,SIGNAL(textChanged()),SLOT(textChanged()));
	connect(button,SIGNAL(clicked()),SLOT(slotPopingup()));

	size = element.attribute ("size", "small");
	if (size == "small") {
	textedit->setMinimumSize (300,25) ; 
	button ->setMinimumSize (300,25) ; 
	textedit->setMaximumSize (300,25) ; 
	button ->setMaximumSize (300,25) ; 
	}
	else if (size == "big") {
	textedit->setMinimumSize (300,100) ; 
	button->setMinimumSize (300,100) ; 
	}
	
	
}


RKPluginBrowser::~RKPluginBrowser()
{
	RK_TRACE (PLUGIN);
}

void RKPluginBrowser::setEnabled(bool checked){
	RK_TRACE (PLUGIN);
label->setEnabled(checked);
textedit->setEnabled(checked);
button->setEnabled(checked);
}


void RKPluginBrowser::slotActive(bool isOk){
	RK_TRACE (PLUGIN);
textedit->setEnabled(isOk) ;
button->setEnabled(isOk) ;
label->setEnabled(isOk) ;
}

  
void RKPluginBrowser::slotActive(){
	RK_TRACE (PLUGIN);
bool isOk = textedit->isEnabled();
textedit->setEnabled(! isOk) ;
button->setEnabled(! isOk) ;
label->setEnabled(! isOk) ;
}

QString RKPluginBrowser::value (const QString &) {
	RK_TRACE (PLUGIN);
	return textedit->text();
}


void RKPluginBrowser::textChanged(){
	RK_TRACE (PLUGIN);
	emit(changed());
}

void RKPluginBrowser::slotPopingup(){
	RK_TRACE (PLUGIN);
	if (type == "file") {
		QString filename= KFileDialog:: getOpenFileName(QString::null,filter,0, QString::null) ; 
		textedit ->setText(filename);
		emit(changed());}
	else if (type=="dir"){
		QString filename= KFileDialog::getExistingDirectory(); 
		textedit ->setText(filename);
		emit(changed());}
	else if (type=="files"){
		QStringList filenames =KFileDialog:: getOpenFileNames(QString::null,filter,0,QString::null);
		QString filename = filenames.join( "\n" );
		textedit ->setText(filename);
		emit(changed());
		};
	
}


#include "rkpluginbrowser.moc"
