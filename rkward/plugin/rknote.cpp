//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "rknote.h"
#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>

#include "../rkglobals.h"


RKNote::RKNote(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {

	qDebug("creating note");
	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	label = new QLabel (element.attribute ("label", "Select one:"), this);
	vbox->addWidget (label);
	depend = element.attribute ("depend", "");


}


RKNote::~RKNote()
{
}

void RKNote::setEnabled(bool checked){
  label->setEnabled(checked);
  }
void RKNote::slotActive(bool isOk){
label->setEnabled(isOk) ;
}

  
void RKNote::slotActive(){
bool isOk = label->isEnabled();
label->setEnabled(! isOk) ;
}




#include "rknote.moc"
