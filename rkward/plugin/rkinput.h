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
#ifndef RKINPUT_H
#define RKINPUT_H

#include <rkpluginwidget.h>
class QTextEdit ; 
class QLabel ; 
class QVBoxLayout;


/**
@author Adrien d'Hardemare
*/
class RKInput : public RKPluginWidget
{
Q_OBJECT
public:
    	RKInput(const QDomElement &element, QWidget *parent, RKPlugin *plugin);
    	~RKInput();
    	void setEnabled(bool);
	QVBoxLayout *vbox	;
private:
	QString depend;
	QTextEdit * textedit ; 
	QLabel * label ;
	QString size ; 
public slots:
	void slotActive();
	void slotActive(bool);
	void textChanged();
protected:
/** Returns the value of the currently selected option. */
	QString value (const QString &modifier);	
};

#endif
