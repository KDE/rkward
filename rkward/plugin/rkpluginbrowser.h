//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{Adrien d'Hardemare} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RKPLUGINBROWSER_H
#define RKPLUGINBROWSER_H

#include <rkpluginwidget.h>
class QTextEdit ; 
class QLabel ; 
class QVBoxLayout;


/**
@author Adrien d'Hardemare
*/
class QPushButton ; 

class RKPluginBrowser : public RKPluginWidget
{
Q_OBJECT
public:
    	RKPluginBrowser(const QDomElement &element, QWidget *parent, RKPlugin *plugin);
    	~RKPluginBrowser();
    	void setEnabled(bool);
	QVBoxLayout *vbox	;
private:
	QString depend;
	QTextEdit * textedit ; 
	QLabel * label ;
	QPushButton * button ;
	QString type;		
	QString size;		
	QString filter;
public slots:
	void slotActive();
	void slotActive(bool);
	void textChanged();
private slots : 
	void slotPopingup();
protected:
/** Returns the value of the currently selected option. */
	QString value (const QString &modifier);	
};

#endif
