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
	virtual ~REmbedInternal();
protected:
	bool startR (const char* r_home, int argc, char **argv);
	void shutdown ();
	void runCommandInternal (const char *command, bool *error, bool print_result=false);
	
	char **getCommandAsStringVector (const char *command, int *count, bool *error);
	double *getCommandAsRealVector (const char *command, int *count, bool *error);
	int *getCommandAsIntVector (const char *command, int *count, bool *error);

public:
// these will need QStrings and stuff and hence are handled in REmbed
	virtual void handleSubstackCall (char **call, int call_length) = 0;
	//virtual char **handleGetValueCall (char **call, int call_length, int *reply_length) = 0;

// only one instance of this class may be around. This pointer keeps the reference to it, for interfacing to C++
	static REmbedInternal *this_pointer;
private:
// can't declare this as part of the class, as it would confuse REmbed
//	SEXPREC *runCommandInternalBase (const char *command, bool *error);
};
 
#endif
