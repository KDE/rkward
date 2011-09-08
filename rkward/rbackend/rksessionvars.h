/***************************************************************************
                          rksessionvars  -  description
                             -------------------
    begin                : Thu Sep 08 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#ifndef RKSESSIONVARS_H
#define RKSESSIONVARS_H

#include <QObject>
#include <QStringList>

class RInterface;
class RCommand;

/** Singleton for storing information about the running R session, and - for some of the info - notifying about changes. */
class RKSessionVars : public QObject {
	Q_OBJECT
public:
	static RKSessionVars* instance () { return _instance; };
	QStringList installedPackages () const { return installed_packages; };
	void setInstalledPackages (const QStringList &new_list);
signals:
	void installedPackagesChanged ();
protected:
friend class RInterface;
	RKSessionVars (RInterface *parent);
	~RKSessionVars ();
private:
	static RKSessionVars* _instance;

	QStringList installed_packages;
private slots:
	void installedPackagesCommandFinished (RCommand *command);
};

#endif
