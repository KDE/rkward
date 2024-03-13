/*
rkcomponentscripting - This file is part of the RKWard project. Created: Thu Jun 17 2010
SPDX-FileCopyrightText: 2010-2023 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKCOMPONENTSCRIPTING_H
#define RKCOMPONENTSCRIPTING_H

#include <QObject>
#include <QHash>

#include "qtscriptbackend.h"  // TODO: For RKJSEngine, only

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
	explicit RKComponentScriptingProxy (RKComponent *component);
	~RKComponentScriptingProxy ();

	void initialize (const QString& file, const QString& command);
public Q_SLOTS:
	void componentChanged (RKComponent* changed);
	void propertyChanged (RKComponentPropertyBase* changed);

public:
// these are meant to be called from the script
	Q_INVOKABLE void include (const QString& filename);
	Q_INVOKABLE void addChangeCommand (const QString& changed_id, const QString& command);
/** @returns id of the command issued. */
	Q_INVOKABLE QVariant doRCommand (const QString& command, const QString& callback);

	Q_INVOKABLE QVariant getValue (const QString &id) const;
	Q_INVOKABLE QVariant getString (const QString &id) const;
	Q_INVOKABLE QVariant getBoolean (const QString &id) const;
	Q_INVOKABLE QVariant getList (const QString &id) const;
	Q_INVOKABLE void setValue (const QString &value, const QString &id);
	Q_INVOKABLE void setListValue (const QStringList &value, const QString &id);

	Q_INVOKABLE QVariantList getObjectInfo (const QString &name);
	Q_INVOKABLE QString getObjectParent (const QString &name);
	Q_INVOKABLE QString getObjectChild (const QString &name);
Q_SIGNALS:
	void haveError ();
private Q_SLOTS:
	void scriptRCommandFinished (RCommand* command);
private:
	RKComponent* component;
	QJSEngine engine;
	struct OutstandingCommand {
		RCommand *command;
		QString callback;
	};
	QList<OutstandingCommand> outstanding_commands;
	QString _scriptfile;
	void evaluate (const QString &code);

	void handleChange (RKComponentBase* changed);
	QHash<RKComponentBase*, QString> component_commands;

	void handleScriptError (const QJSValue &val, const QString& current_file=QString ());
};

#endif
