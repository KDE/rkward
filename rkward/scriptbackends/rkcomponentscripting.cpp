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

#include "../plugin/rkcomponent.h"
#include "../core/robjectlist.h"
#include "../misc/rkcommonfunctions.h"

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
}

void RKComponentScriptingProxy::initialize (const QString& file, const QString& command) {
	RK_TRACE (PHP);

	QString _command = command;
	if (!file.isEmpty ()) {
		_command.prepend ("include('" + file + "');\n");
		_scriptfile = file;
	}
	QDir files_path (RKCommonFunctions::getRKWardDataDir () + "phpfiles/");
	_command.prepend ("_rkward.include('" + files_path.absoluteFilePath ("rkcomponentscripting.js") + "');\n");
	script->setCode (_command.toUtf8 ());

	script->trigger ();
	handleScriptError ();
}

void RKComponentScriptingProxy::handleScriptError (const QString& current_file) {
	RK_TRACE (PHP);

	QString file = current_file;
	if (file.isEmpty ()) file = _scriptfile;
	if (script->hadError ()) {
#warning TODO: refine error messages (file/context), and display them in a dialog
qDebug ("line %d: %s", script->errorLineNo (), qPrintable (script->errorMessage ()));
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
		script->evaluate (i18n ("error ('The file \"%1\" (needed by \"%2\") could not be found. Please check your installation.');\n", _filename, _scriptfile).toUtf8 ());
		return;
	}

	script->evaluate (file.readAll());
	handleScriptError (_filename);
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
	script->evaluate (command.toUtf8());
}

QString RKComponentScriptingProxy::getValue (const QString &id) const {
	RK_TRACE (PHP);

	return component->fetchStringValue (id);
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
		for (unsigned int i = 0; i < object->numDimensions (); ++i) {
			dims.append (object->getDimension (i));
		}
		ret.append (QVariant (dims));

		QStringList classes;
		for (unsigned int i = 0; i < object->numClasses (); ++i) {
			classes.append (object->getClassName (i));
		}
		ret.append (QVariant (classes));

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
		if (object->getContainer ()) return (object->getContainer ()->getFullName ());
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
