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
#include "rkkonsole.h"
#include <kde_terminal_interface.h>
#include "windows/rkcommandeditorwindow.h"
#include "debug.h"
#include <qstringlist.h>
#include <qlayout.h>
#include <qstring.h>


RKKonsole::RKKonsole(QWidget *parent) :RKToggleWidget (parent){
  
  QVBoxLayout *layout = new QVBoxLayout (this);
  KLibFactory* factory = KLibLoader::self()->factory( "libkonsolepart" );
  if ( factory == 0L )
  {
    // inform the user that he should install konsole..
    return;
  };
  KParts::Part* p = static_cast<KParts::Part*>(
      factory->create( this, "tralala", "QObject","KParts::ReadOnlyPart" ) );  
  // fetch the part..
	RK_ASSERT (p);
  layout -> addWidget( p->widget() );
	
  // cast the part to the TerminalInterface..
  interface = static_cast<TerminalInterface*>( p->qt_cast( "TerminalInterface" ) );
  
  	RK_ASSERT (interface) ;
  if( ! interface )
  {
    // This probably happens because the konsole that is installed
    // comes from before KDE 3.2 , and the TerminalInterface is not
    // available..  What you can do here is either inform the user
    // that he needs a more recent konsole, or try to deliver the
    // functionality in some other way...
    return;
  };
  // now use the interface in all sorts of ways, e.g.
//      t->showShellInDir( QDir::home().path() );
  // or:
     QStrList l;
     l.append( "python" );
     interface->startProgram( QString::fromUtf8( "/usr/bin/R" ), l);
  // or connect to one of the signals.  Connect to the Part object,
  // not to the TerminalInterface, since the latter is no QObject,
  // and as such cannot have signals..:
  //    connect( p, SIGNAL( processExited( int ) ),
  //             this, SLOT( shellExited( int ) ) );
  // etc.*/
}


RKKonsole::~RKKonsole()
{
}



/*!
    \fn RKKonsole::sendInput(const QString & text)
 */
void RKKonsole::sendInput(const QString & text)
{
interface->sendInput(text);
    /// @todo implement me
}
