/***************************************************************************
                          rkcomponentscripting  -  description
                             -------------------
    begin                : Thu Jun 17 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#include "rkcomponentscripting.h"

#include <klocale.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <QDir>

#include "../plugin/rkcomponent.h"
#include "../core/robjectlist.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/xmlhelper.h"
#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "qtscriptbackend.h"
#include "qtscripti18n.h"

#include "../debug.h"

RKComponentScriptingProxy::RKComponentScriptingProxy (RKComponent *component) : QObject (component) {
	RK_TRACE (PHP);

	RK_ASSERT (component);
	RKComponentScriptingProxy::component = component;

	QScriptValue backend_object = engine.newQObject (this);
	engine.globalObject ().setProperty ("_rkward", backend_object);
	RKMessageCatalogObject::addI18nToScriptEngine (&engine, component->xmlHelper ()->messageCatalog ());
}

RKComponentScriptingProxy::~RKComponentScriptingProxy () {
	RK_TRACE (PHP);

	for (int i = 0; i < outstanding_commands.size (); ++i) {
		RKGlobals::rInterface ()->cancelCommand (outstanding_commands[i].command);
	}
}

void RKComponentScriptingProxy::initialize (const QString& file, const QString& command) {
	RK_TRACE (PHP);

	QString _command = command;
	if (!file.isEmpty ()) {
		_command.prepend ("_rkward.include('" + file + "');\n");
		_scriptfile = file;
	}
	QDir files_path (RKCommonFunctions::getRKWardDataDir () + "phpfiles/");
#ifdef USE_Q_SCRIPT_PROGRAM
	if (!RKPrecompiledQtScripts::loadCommonScript (&engine, files_path.absoluteFilePath ("rkcomponentscripting.js"))) {
		engine.evaluate (i18n ("Error opening script file %1", files_path.absoluteFilePath ("rkcomponentscripting.js")));
	} else if (!RKPrecompiledQtScripts::loadCommonScript (&engine, files_path.absoluteFilePath ("common.js"))) {
		engine.evaluate (i18n ("Error opening script file %1", files_path.absoluteFilePath ("common.js")));
	}
#else
	_command.prepend ("_rkward.include('" + files_path.absoluteFilePath ("rkcomponentscripting.js") + "');\n");
	_command.prepend ("_rkward.include('" + files_path.absoluteFilePath ("common.js") + "');\n");
#endif
	evaluate (_command);
}

void RKComponentScriptingProxy::handleScriptError (const QString& current_file) {
	RK_TRACE (PHP);

	QString file = current_file;
	if (file.isEmpty ()) file = _scriptfile;
	if (engine.hasUncaughtException ()) {
		QString message = i18n ("Script Error: %1\n", engine.uncaughtException ().toString ());
		KMessageBox::detailedError (0, message, engine.uncaughtExceptionBacktrace ().join ("\n"));
		engine.clearExceptions ();
		emit (haveError());
	}
}

void RKComponentScriptingProxy::include (const QString& filename) {
	RK_TRACE (PHP);

	QString _filename = filename;
	if (QFileInfo (filename).isRelative ()) {
		KUrl script_path = KUrl (QUrl::fromLocalFile (_scriptfile)).upUrl ();
		script_path.addPath (filename);
		_filename = script_path.toLocalFile ();
	}

	QFile file (_filename);
	if (!file.open (QIODevice::ReadOnly | QIODevice::Text)) {
		evaluate (i18n ("error ('The file \"%1\" (needed by \"%2\") could not be found. Please check your installation.');\n", _filename, _scriptfile).toUtf8 ());
		return;
	}

	evaluate (file.readAll());
	handleScriptError (_filename);
}

void RKComponentScriptingProxy::evaluate (const QString &code) {
	RK_TRACE (PHP);

	// evaluate in global context
	engine.currentContext ()->setActivationObject (engine.globalObject ());
	QScriptValue result = engine.evaluate (code, _scriptfile);

	handleScriptError ();
}

void RKComponentScriptingProxy::addChangeCommand (const QString& changed_id, const QString& command) {
	RK_TRACE (PHP);

	QString remainder;
	RKComponentBase* base = component->lookupComponent (changed_id, &remainder);

	if (remainder.isEmpty ()) {
		component_commands.insert (base, command);
		if (base->isComponent()) {
			connect (static_cast<RKComponent*> (base), SIGNAL (componentChanged(RKComponent*)), this, SLOT (componentChanged(RKComponent*)));
		} else {
			connect (static_cast<RKComponentPropertyBase*> (base), SIGNAL (valueChanged(RKComponentPropertyBase*)), this, SLOT (propertyChanged(RKComponentPropertyBase*)));
		}
	} else {
		evaluate (QString ("error ('No such property %1 (failed portion was %2)');\n").arg (changed_id, remainder));
	}
}

QVariant RKComponentScriptingProxy::doRCommand (const QString& command, const QString& callback) {
	RK_TRACE (PHP);

	// purge duplicate commands
	for (int i = 0; i < outstanding_commands.size (); ++i) {
		const OutstandingCommand &oc = outstanding_commands[i];
		if (oc.callback == callback) {
			if (RKGlobals::rInterface ()->softCancelCommand (oc.command)) {
				outstanding_commands.removeAt (i);
				--i;
				continue;
			}
		}
	}

	OutstandingCommand com;
	com.command = new RCommand (command, RCommand::PriorityCommand | RCommand::GetStructuredData | RCommand::Plugin);
	connect (com.command->notifier (), SIGNAL (commandFinished(RCommand*)), this, SLOT (scriptRCommandFinished(RCommand*)));
	com.callback = callback;
	outstanding_commands.append (com);

	RKGlobals::rInterface ()->issueCommand (com.command);
	return (QVariant (com.command->id ()));
}

static QScriptValue marshall (QScriptEngine *engine, RData *data) {
	RK_TRACE (PHP);

	if (data->getDataType () == RData::StringVector) {
		return (qScriptValueFromSequence (engine, data->stringVector()));
	} else if (data->getDataType () == RData::IntVector) {
		return (qScriptValueFromSequence (engine, data->intVector()));
	} else if (data->getDataType () == RData::RealVector) {
		return (qScriptValueFromSequence (engine, data->realVector()));
	} else if (data->getDataType () == RData::StructureVector) {
		const RData::RDataStorage& rs = data->structureVector ();
		QScriptValue ret = engine->newArray (rs.size ());
		for (int i = 0; i < rs.size (); ++i) {
			ret.setProperty (i, marshall (engine, rs[i]));
		}
		return ret;
	} else {
		RK_ASSERT (false);
	}
	return QScriptValue ();
}

void RKComponentScriptingProxy::scriptRCommandFinished (RCommand* command) {
	RK_TRACE (PHP);

	QString callback;
	for (int i = 0; i < outstanding_commands.size (); ++i) {
		const OutstandingCommand& oc = outstanding_commands[i];
		if (oc.command == command) {
			callback = oc.callback;
			outstanding_commands.removeAt (i);
			break;
		}
	}
	RK_ASSERT (!callback.isNull ());

	if (command->wasCanceled ()) return;
	if (command->failed ()) RK_DEBUG (PHP, DL_ERROR, "Plugin script R command %s failed. Full output wsa %s", qPrintable (command->command ()), qPrintable (command->fullOutput ()));

	QScriptValueList args;
	args.append (marshall (&engine, command));
	args.append (QScriptValue (command->id ()));
	QScriptValue callback_obj = engine.globalObject ().property (callback);
	callback_obj.call (engine.globalObject (), args);
	handleScriptError ();
}

void RKComponentScriptingProxy::componentChanged (RKComponent* changed) {
	RK_TRACE (PHP);
	handleChange (changed);
}

void RKComponentScriptingProxy::propertyChanged (RKComponentPropertyBase* changed) {
	RK_TRACE (PHP);
	handleChange (changed);
}

void RKComponentScriptingProxy::handleChange (RKComponentBase* changed) {
	RK_TRACE (PHP);

	QString command = component_commands.value (changed);
	evaluate (command.toUtf8());
}

QVariant RKComponentScriptingProxy::getValue (const QString &id) const {
	RK_TRACE (PHP);
	return (component->fetchValue (id, RKComponent::TraditionalValue));
}

QVariant RKComponentScriptingProxy::getString (const QString &id) const {
	RK_TRACE (PHP);
	return (component->fetchValue (id, RKComponent::StringValue));
}

QVariant RKComponentScriptingProxy::getBoolean (const QString &id) const {
	RK_TRACE (PHP);
	return (component->fetchValue (id, RKComponent::BooleanValue));
}

QVariant RKComponentScriptingProxy::getList (const QString &id) const {
	RK_TRACE (PHP);
	return (component->fetchValue (id, RKComponent::StringlistValue));
}

void RKComponentScriptingProxy::setValue (const QString &value, const QString &id) {
	RK_TRACE (PHP);

	QString modifier;
	RKComponentBase* resolved = component->lookupComponent (id, &modifier);
	if (resolved && modifier.isEmpty () && resolved->isProperty ()) {
		static_cast<RKComponentPropertyBase*> (resolved)->setValue (value);
	} else {
		evaluate (QString ("error ('No such property %1 (failed portion was %2)');\n").arg (id, modifier));
	}
}

void RKComponentScriptingProxy::setListValue (const QStringList& value, const QString& id) {
	RK_TRACE (PHP);

	QString modifier;
	RKComponentBase* resolved = component->lookupComponent (id, &modifier);
	if (resolved && modifier.isEmpty () && resolved->isProperty ()) {
		RKComponentPropertyAbstractList *l = dynamic_cast<RKComponentPropertyAbstractList*> (resolved);
		if (l) {
			l->setValueList (value);
			return;
		}
		static_cast<RKComponentPropertyBase*> (resolved)->setValue (value.join ("\n"));
	} else {
		evaluate (QString ("error ('No such property %1 (failed portion was %2)');\n").arg (id, modifier));
	}
}

QVariantList RKComponentScriptingProxy::getObjectInfo (const QString &name) {
	RK_TRACE (PHP);

	RObject* object = RObjectList::getObjectList ()->findObject (name);
	if (object) {
		QVariantList ret;

		QVariantList dims;
		foreach (int dim, object->getDimensions ()) {
			dims.append (dim);
		}
		ret.append (QVariant (dims));

		ret.append (QVariant (object->classNames ()));

		ret.append (object->isType (RObject::DataFrame));
		ret.append (object->isType (RObject::Matrix));
		ret.append (object->isType (RObject::List));
		ret.append (object->isType (RObject::Function));
		ret.append (object->isType (RObject::Environment));

		if (object->getDataType () == RObject::DataNumeric) ret.append ("numeric");
		else if (object->getDataType () == RObject::DataFactor) ret.append ("factor");
		else if (object->getDataType () == RObject::DataCharacter) ret.append ("character");
		else if (object->getDataType () == RObject::DataLogical) ret.append ("logical");
		else ret.append ("unknown");

		return (ret);
	}
	return (QVariantList ());
}

QString RKComponentScriptingProxy::getObjectParent (const QString &name) {
	RK_TRACE (PHP);

	RObject* object = RObjectList::getObjectList ()->findObject (name);
	if (object) {
		if (object->parentObject ()) return (object->parentObject ()->getFullName ());
	}
	return (QString ());
}

QString RKComponentScriptingProxy::getObjectChild (const QString &name) {
	RK_TRACE (PHP);
	RObject* object = RObjectList::getObjectList ()->findObject (name);

	if (object) {
		if (object->isContainer ()) {
			RObject* child = static_cast<RContainerObject*> (object)->findChildByName (name);
			if (child) return (child->getFullName ());
		}
	}
	return (QString ());
}

#include "rkcomponentscripting.moc"
