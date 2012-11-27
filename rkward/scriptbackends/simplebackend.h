/***************************************************************************
                          simplebackend  -  description
                             -------------------
    begin                : Thu May 10 2007
    copyright            : (C) 2007, 2012 by Thomas Friedrichsmeier
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

#ifndef SIMPLEBACKEND_H
#define SIMPLEBACKEND_H

#include "scriptbackend.h"

#include <qstringlist.h>

/** @brief A very simple script backend

This class provides a very simple alternative to the PHP backend. Right now it's basically just used as a hack to reduce the overhead of starting a PHP process for each color_chooser component (which is often embedded many times inside a single plugin).

This class is very hackish and NOT sure to stay! It might be obsoleted by another scripting solution.

@author Thomas Friedrichsmeier
*/
class SimpleBackend : public ScriptBackend {
public:
	SimpleBackend ();
	~SimpleBackend ();

	void setPreprocessTemplate (const QString &template_string) { preprocess_template = template_string; };
	void setCalculateTemplate (const QString &template_string) { calculate_template = template_string; };
	void setPrintoutTemplate (const QString &template_string) { printout_template = template_string; };
	void setPreviewTemplate (const QString &template_string) { preview_template = template_string; };

	bool initialize (RKComponentPropertyCode *code_property=0, bool add_headings=true);
	void destroy ();
	
	void preprocess (int flags);
	void calculate (int flags);
	void printout (int flags);
	void preview (int flags);
	
	void writeData (const QVariant &data);
	void tryNextFunction ();
private:
	QString preprocess_template;
	QString calculate_template;
	QString printout_template;
	QString preview_template;
	QString current_template;

	int template_sep;
	int template_pos;
	QList<QVariant> current_values;

	void processCall ();
	void finishCall (const QString &conditions);
};

#endif
