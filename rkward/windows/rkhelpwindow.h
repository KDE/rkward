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
#ifndef RKHELPWINDOW_H
#define RKHELPWINDOW_H

#include <kmdichildview.h>

class KHTMLPart;

/**
@author Pierre Ecochard
*/
class RKHelpWindow : public KMdiChildView
{
Q_OBJECT
public:
    RKHelpWindow(QWidget *parent = 0, const char *name = 0);

    ~RKHelpWindow();
    bool openURL(KURL url);
private:
    KHTMLPart * khtmlpart;
};

#endif
