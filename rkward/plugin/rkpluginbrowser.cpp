
#include "rkpluginbrowser.h"
#include <qdom.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qgrid.h>
#include <rkplugin.h>
#include <qpushbutton.h>
#include <kfiledialog.h>

#include <klocale.h>
#include "../rkglobals.h"
#include <qstring.h>
#include <kurl.h>

// /////////////////////







RKPluginBrowser::RKPluginBrowser(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {

	qDebug("creating note");
	vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	label = new QLabel (element.attribute ("label", "Enter your text"), this);
	textedit = new QTextEdit ( element.attribute ("intial") ,
	QString::null,this, element.attribute ("id")) ;
	button = new QPushButton ("Browser...",this);
//	size = element.attribute ("size", "small");
	vbox->addWidget (label);
	vbox->addWidget (textedit);
	vbox->addWidget (button);
	type = element.attribute("type","file") ;
	filter = element.attribute("filter","") ;
	connect(textedit,SIGNAL(textChanged()),SLOT(textChanged()));
	connect(button,SIGNAL(clicked()),SLOT(slotPopingup()));

	size = element.attribute ("size", "small");
	if (size == "small") {
	textedit->setMaximumSize (300,25) ; 
	textedit->setMinimumSize (300,25) ; 
	}
	else if (size == "big") {
	textedit->setMaximumSize (300,100) ; 
	textedit->setMinimumSize (300,100) ; 
	}
	
	
}


RKPluginBrowser::~RKPluginBrowser()
{
}

void RKPluginBrowser::setEnabled(bool checked){
label->setEnabled(checked);
textedit->setEnabled(checked);
button->setEnabled(checked);
}


void RKPluginBrowser::slotActive(bool isOk){
textedit->setEnabled(isOk) ;
button->setEnabled(isOk) ;
label->setEnabled(isOk) ;
}

  
void RKPluginBrowser::slotActive(){
bool isOk = textedit->isEnabled();
textedit->setEnabled(! isOk) ;
button->setEnabled(! isOk) ;
label->setEnabled(! isOk) ;
}

QString RKPluginBrowser::value (const QString &) {
	return textedit->text();
}


void RKPluginBrowser::textChanged(){
	emit(changed());
}

void RKPluginBrowser::slotPopingup(){
	if (type == "file") {
		QString filename= KFileDialog:: getOpenFileName("",filter,0,"") ; 
		textedit ->setText(filename);
		emit(changed());}
	else if (type=="dir"){
		QString filename= KFileDialog::getExistingDirectory(); 
		textedit ->setText(filename);
		emit(changed());}
	else if (type=="files"){
		QStringList filenames =KFileDialog:: getOpenFileNames("",filter,0,"") ; 
		QString filename = filenames.join( "\n" );
		textedit ->setText(filename);
		emit(changed());
		};
	
}


#include "rkpluginbrowser.moc"
