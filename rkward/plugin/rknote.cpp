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
#include "../debug.h"

RKNote::RKNote(const QDomElement &element, QWidget *parent, RKPlugin *plugin) : RKPluginWidget (element, parent, plugin) {
	RK_TRACE (PLUGIN);

	QVBoxLayout *vbox = new QVBoxLayout (this, RKGlobals::spacingHint ());
	label = new QLabel (element.attribute ("label", "Select one:"), this);
	vbox->addWidget (label);
	depend = element.attribute ("depend", QString::null);


}


RKNote::~RKNote()
{
	RK_TRACE (PLUGIN);
}

void RKNote::setEnabled(bool checked){
	RK_TRACE (PLUGIN);
  label->setEnabled(checked);
  }
void RKNote::slotActive(bool isOk){
	RK_TRACE (PLUGIN);
label->setEnabled(isOk) ;
}

  
void RKNote::slotActive(){
	RK_TRACE (PLUGIN);
bool isOk = label->isEnabled();
label->setEnabled(! isOk) ;
}

#include "rknote.moc"
