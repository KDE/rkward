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
#ifndef RKKONSOLE_H
#define RKKONSOLE_H
#include <qwidget.h>
#include <kde_terminal_interface.h>
#include "misc/rktogglewidget.h"

/**
@author Thomas Friedrichsmeier
*/
class TerminalInterface ; 
class QString;
class RKKonsole : public RKToggleWidget {
	Q_OBJECT
public:
    RKKonsole(QWidget* parent = 0);
    ~RKKonsole();
    void sendInput(const QString & text);
   
private:
	TerminalInterface * interface ; 


};

#endif
