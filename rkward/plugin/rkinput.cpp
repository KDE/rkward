
#include "rkinput.h"

#include <qdom.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qgrid.h>

#include "rkplugin.h"
#include "../rkglobals.h"
#include "../debug.h"

RKInput::RKInput(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	RK_TRACE (PLUGIN);

	vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	label = new QLabel (element.attribute ("label", "Enter your text"), this);
	QString initial = element.attribute ("initial","") ;
	textedit = new QTextEdit ( initial ,
	QString::null,this, element.attribute ("id")) ;
	vbox->addWidget (label);
	vbox->addWidget (textedit);
	connect(textedit,SIGNAL(textChanged()),SLOT(textChanged()));
	
	size = element.attribute ("size", "medium");
	if (size == "small") {
	textedit->setMaximumSize (100,25) ; 
	textedit->setMinimumSize (100,25) ; 
	} else if (size == "medium") {
	textedit->setMaximumSize (250,25) ; 
	textedit->setMinimumSize (250,25) ; 
	} else if (size == "big") {
	textedit->setMinimumSize (250,100) ; 
	}

	
	//textedit->setMinimumSize (50 , 300) ; 

}


RKInput::~RKInput()
{
	RK_TRACE (PLUGIN);
}

void RKInput::setEnabled(bool checked){
	RK_TRACE (PLUGIN);
label->setEnabled(checked);
textedit->setEnabled(checked);
}


void RKInput::slotActive(bool isOk){
	RK_TRACE (PLUGIN);
textedit->setEnabled(isOk) ;
label->setEnabled(isOk) ;
}

  
void RKInput::slotActive(){
	RK_TRACE (PLUGIN);
bool isOk = textedit->isEnabled();
textedit->setEnabled(! isOk) ;
label->setEnabled(! isOk) ;
}

QString RKInput::value (const QString &) {
	RK_TRACE (PLUGIN);
	return textedit->text();
}


void RKInput::textChanged(){
	RK_TRACE (PLUGIN);
	emit(changed());
}


#include "rkinput.moc"
