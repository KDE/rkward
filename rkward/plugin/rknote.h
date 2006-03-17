//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RKNOTE_H
#define RKNOTE_H

#include <rkpluginwidget.h>


class QLabel ; 

/**

TODO: remove. This is essentially a dupe of RKText

@author Thomas Friedrichsmeier
*/
class RKNote : public RKPluginWidget
{
Q_OBJECT
public:
    RKNote(const QDomElement &element, QWidget *parent, RKPlugin *plugin);
    ~RKNote();
    void setEnabled(bool);
private:
	QString depend;
	
	QLabel * label ;
public slots:
	void slotActive();
	void slotActive(bool);
	
};

#endif
