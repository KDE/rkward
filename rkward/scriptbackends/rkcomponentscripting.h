/***************************************************************************
                          rkcomponentscripting  -  description
                             -------------------
    begin                : Thu Jun 17 2010
    copyright            : (C) 2010, 2013 by Thomas Friedrichsmeier
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

#ifndef RKCOMPONENTSCRIPTING_H
#define RKCOMPONENTSCRIPTING_H

#include <QObject>
#include <QHash>

#include <kross/core/action.h>
#include "../rbackend/rcommand.h"

class RKComponent;
class RKComponentBase;
class RKComponentPropertyBase;

/** This class basically provides the API that is available to scripts running within rkward plugins.
The slots are meant to be called from the script.

NOTE: This contains some duplication of ScriptBackend and derived classes. Perhaps this can be merged, better.
The key technical difference between this, and ScriptBackend, is that this operates in the main thread, while
ScriptBackend is designed to operate in a separate thread, and may merge a bunch of changes into a single update. */
class RKComponentScriptingProxy : public QObject {
Q_OBJECT
public:
	RKComponentScriptingProxy (RKComponent *component);
	~RKComponentScriptingProxy ();

	void initialize (const QString& file, const QString& command);
	void addScriptableWidget (const QString& name, QWidget *widget);
public slots:
	void componentChanged (RKComponent* changed);
	void propertyChanged (RKComponentPropertyBase* changed);

// these are meant to be called from the script
	void include (const QString& filename);
	void addChangeCommand (const QString& changed_id, const QString& command);
/** @returns id of the command issued. */
	QVariant doRCommand (const QString& command, const QString& callback);

	QVariant getValue (const QString &id) const;
	QVariant getString (const QString &id) const;
	QVariant getBoolean (const QString &id) const;
	QVariant getList (const QString &id) const;
	void setValue (const QString &value, const QString &id);

	QVariantList getObjectInfo (const QString &name);
	QString getObjectParent (const QString &name);
	QString getObjectChild (const QString &name);
signals:
	void haveError ();
private slots:
	void scriptRCommandFinished (RCommand* command);
private:
	RKComponent* component;
	Kross::Action* script;
	struct OutstandingCommand {
		RCommand *command;
		QString callback;
	};
	QList<OutstandingCommand> outstanding_commands;
	QString _scriptfile;
/** helper function for compatibility with KDE < 4.3 */
	void evaluate (const QByteArray &code);

	void handleChange (RKComponentBase* changed);
	QHash<RKComponentBase*, QString> component_commands;

	void handleScriptError (const QString& current_file=QString ());
};

#endif
