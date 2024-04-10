/*
simplebackend - This file is part of the RKWard project. Created: Thu May 10 2007
SPDX-FileCopyrightText: 2007-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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

	bool initialize (RKComponentPropertyCode *code_property=nullptr, bool add_headings=true) override;
	void destroy () override;
	
	void preprocess (int flags) override;
	void calculate (int flags) override;
	void printout (int flags) override;
	void preview (int flags) override;
	
	void writeData (const QVariant &data) override;
	void tryNextFunction () override;
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
