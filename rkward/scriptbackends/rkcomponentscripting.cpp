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

#include "../plugin/rkcomponent.h"
#include "../core/robjectlist.h"
#include "../misc/rkcommonfunctions.h"
#include "../rkglobals.h"
#include "../rbackend/rinterface.h"

#include "../debug.h"

RKComponentScriptingProxy::RKComponentScriptingProxy (RKComponent *component) : QObject (component) {
	RK_TRACE (PHP);

	RK_ASSERT (component);
	RKComponentScriptingProxy::component = component;

	script = new Kross::Action (this, QString ());
	script->setInterpreter ("qtscript");
	script->addObject (this, "_rkward");
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
	_command.prepend ("_rkward.include('" + files_path.absoluteFilePath ("rkcomponentscripting.js") + "');\n");
	_command.prepend ("_rkward.include('" + files_path.absoluteFilePath ("common.js") + "');\n");
#if not KDE_IS_VERSION(4,3,0)
	_command.prepend ("_rk_eval = function (x) { eval (x); }\n");
#endif
	script->setCode (_command.toUtf8 ());

	script->trigger ();
	handleScriptError ();
}

void RKComponentScriptingProxy::handleScriptError (const QString& current_file) {
	RK_TRACE (PHP);

	QString file = current_file;
	if (file.isEmpty ()) file = _scriptfile;
	if (script->hadError ()) {
		QString message = i18n ("There was an error while evaluating script code.\nFile: %1\nLine: %2\nMessage: %3.", file, script->errorLineNo(), script->errorMessage());
		KMessageBox::detailedError (0, message, script->errorTrace ());
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

void RKComponentScriptingProxy::evaluate (const QByteArray &code) {
	RK_TRACE (PHP);

#if KDE_IS_VERSION(4,3,0)
	script->evaluate (code);
#else
	script->callFunction ("_rk_eval", QVariantList() << QString (code));
#endif
	handleScriptError ();
}

void RKComponentScriptingProxy::addScriptableWidget (const QString& name, QWidget *widget) {
	RK_TRACE (PHP);

	script->addObject (widget, name);
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
		script->setError (QString ("error ('No such property %1 (failed portion was %2)');\n").arg (changed_id, remainder));
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

static QVariant marshall (RData *data) {
	RK_TRACE (PHP);

	if (data->getDataType () == RData::StringVector) {
		return (QVariant (data->stringVector()));
	} else if (data->getDataType () == RData::IntVector) {
		const RData::IntStorage& is = data->intVector ();
		QVariantList ret;
		for (int i = 0; i < is.size (); ++i) {
			ret.append (QVariant (is[i]));
		}
		return ret;
	} else if (data->getDataType () == RData::RealVector) {
		const RData::RealStorage& rs = data->realVector ();
		QVariantList ret;
		for (int i = 0; i < rs.size (); ++i) {
			ret.append (QVariant (rs[i]));
		}
		return ret;
	} else if (data->getDataType () == RData::StructureVector) {
		const RData::RDataStorage& rs = data->structureVector ();
		QVariantList ret;
		for (int i = 0; i < rs.size (); ++i) {
			ret.append (marshall (rs[i]));
		}
		return ret;
	} else {
		RK_ASSERT (false);
	}
	return QVariant ();
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

	QVariantList args;
	args.append (marshall (command));
	args.append (QVariant (command->id ()));
	script->callFunction (callback, args);
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
		script->setError (QString ("error ('No such property %1 (failed portion was %2)');\n").arg (id, modifier));
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
