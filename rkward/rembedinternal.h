/***************************************************************************
                          rembedinternal  -  description
                             -------------------
    begin                : Sun Jul 25 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef R_EMBED_H
#define R_EMBED_H

 /** The main purpose of separating this class from REmbed is
	that R- and Qt-includes don't like each other. Hence this class is Qt-agnostic while
	REmbed is essentially R-agnostic.
	*@author Thomas Friedrichsmeier
  */
  
class REmbedInternal {
public: 
	REmbedInternal();
	~REmbedInternal();
protected:
	bool startR (const char* r_home, int argc, char **argv);
	void shutdown ();
	bool runCommandInternal (const char *command);
};
 
#endif
