/*
rkcomponentscripting - This file is part of RKWard (https://rkward.kde.org). Created: Thu Jun 17 2010
SPDX-FileCopyrightText: 2010-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcomponentscripting.h"

#include <KLocalizedString>
#include <kmessagebox.h>
#include <QDir>

#include "../plugin/rkcomponent.h"
#include "../core/robjectlist.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/xmlhelper.h"
#include "../rbackend/rkrinterface.h"
#include "qtscriptbackend.h"
#include "qtscripti18n.h"

#include "../debug.h"

RKComponentScriptingProxy::RKComponentScriptingProxy (RKComponent *component) : QObject (component) {
	RK_TRACE (PHP);

	RK_ASSERT (component);
	RKComponentScriptingProxy::component = component;

	QJSValue backend_object = engine.newQObject(this);
	engine.globalObject ().setProperty ("_rkward", backend_object);
	RKMessageCatalogObject::addI18nToScriptEngine (&engine, component->xmlHelper ()->messageCatalog ());
}

RKComponentScriptingProxy::~RKComponentScriptingProxy () {
	RK_TRACE (PHP);

	for (int i = 0; i < outstanding_commands.size (); ++i) {
		RInterface::instance()->cancelCommand(outstanding_commands[i].command);
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

void RKComponentScriptingProxy::handleScriptError(const QJSValue &val, const QString& current_file) {
	RK_TRACE (PHP);

	QString file = current_file;
	if (file.isEmpty ()) file = _scriptfile;
	if (val.isError()) {
		QString message = i18n("Script Error at '%1' line %2: %3\n", file.isEmpty() ? i18n("inlined code") : file, val.property("lineNumber").toInt(), val.toString());
		KMessageBox::detailedError(nullptr, message, val.property("stack").toString());
		Q_EMIT haveError();
	}
}

void RKComponentScriptingProxy::include (const QString& filename) {
	RK_TRACE (PHP);

	QString _filename = filename;
	if (QFileInfo (filename).isRelative ()) {
		QUrl script_path = QUrl (QUrl::fromLocalFile (_scriptfile)).adjusted (QUrl::RemoveFilename).resolved (QUrl (filename));
		_filename = script_path.toLocalFile ();
	}

	QFile file (_filename);
	if (!file.open (QIODevice::ReadOnly | QIODevice::Text)) {
		evaluate (i18n ("error ('The file \"%1\" (needed by \"%2\") could not be found. Please check your installation.');\n", _filename, _scriptfile).toUtf8 ());
		return;
	}

	evaluate (QString::fromUtf8 (file.readAll()));
	handleScriptError (_filename);
}

void RKComponentScriptingProxy::evaluate (const QString &code) {
	RK_TRACE (PHP);

	QJSValue result = engine.evaluate (code, _scriptfile);

	handleScriptError(result);
}

void RKComponentScriptingProxy::addChangeCommand (const QString& changed_id, const QString& command) {
	RK_TRACE (PHP);

	QString remainder;
	RKComponentBase* base = component->lookupComponent (changed_id, &remainder);

	if (remainder.isEmpty ()) {
		component_commands.insert (base, command);
		if (base->isComponent()) {
			connect (static_cast<RKComponent*> (base), &RKComponent::componentChanged, this, &RKComponentScriptingProxy::componentChanged);
		} else {
			connect (static_cast<RKComponentPropertyBase*> (base), &RKComponentPropertyBase::valueChanged, this, &RKComponentScriptingProxy::propertyChanged);
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
			if (RInterface::instance()->softCancelCommand(oc.command)) {
				outstanding_commands.removeAt (i);
				--i;
				continue;
			}
		}
	}

	OutstandingCommand com;
	com.command = new RCommand (command, RCommand::PriorityCommand | RCommand::GetStructuredData | RCommand::Plugin);
	connect (com.command->notifier (), &RCommandNotifier::commandFinished, this, &RKComponentScriptingProxy::scriptRCommandFinished);
	com.callback = callback;
	outstanding_commands.append (com);

	RInterface::issueCommand (com.command);
	return (QVariant (com.command->id ()));
}

static QJSValue marshall (QJSEngine *engine, RData *data) {
	RK_TRACE (PHP);

	if (data->getDataType() == RData::StringVector) {
		return (rkJSMakeArray(engine, data->stringVector()));
	} else if (data->getDataType() == RData::IntVector) {
		return (rkJSMakeArray(engine, data->intVector()));
	} else if (data->getDataType() == RData::RealVector) {
		return (rkJSMakeArray(engine, data->realVector()));
	} else if (data->getDataType() == RData::StructureVector) {
		const RData::RDataStorage& rs = data->structureVector();
		QJSValue ret = engine->newArray (rs.size ());
		for (int i = 0; i < rs.size (); ++i) {
			ret.setProperty (i, marshall (engine, rs[i]));
		}
		return ret;
	} else {
		RK_ASSERT (false);
	}
	return QJSValue ();
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

	QJSValueList args;
	args.append (marshall (&engine, command));
	args.append (QJSValue (command->id ()));
	QJSValue callback_obj = engine.globalObject ().property (callback);
	auto res = callback_obj.call(args);
	handleScriptError(res);
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
		const auto objectDims = object->getDimensions ();
		for (int dim : objectDims) {
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

