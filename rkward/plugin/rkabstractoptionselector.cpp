/*
rkabstractoptionselector - This file is part of RKWard (https://rkward.kde.org). Created: Tue Mar 20 2007
SPDX-FileCopyrightText: 2007-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkabstractoptionselector.h"

#include <qdom.h>

#include "../misc/xmlhelper.h"
#include "../debug.h"

RKAbstractOptionSelector::RKAbstractOptionSelector (RKComponent *parent_component, QWidget *parent_widget) : RKComponent (parent_component, parent_widget) {
	RK_TRACE (PLUGIN);

	// create and register properties
	addChild ("string", string = new RKComponentPropertyBase (this, false));
	connect (string, &RKComponentPropertyBase::valueChanged, this, &RKAbstractOptionSelector::propertyChanged);
	addChild ("number", number = new RKComponentPropertyInt (this, true, -1));
	connect (number, &RKComponentPropertyBase::valueChanged, this, &RKAbstractOptionSelector::propertyChanged);
	number->setInternal (true);
}

RKAbstractOptionSelector::~RKAbstractOptionSelector(){
	RK_TRACE (PLUGIN);

	for (OptionsMap::const_iterator it = options.cbegin(); it != options.cend(); ++it) {
		delete (it.value ());
	}
}

void RKAbstractOptionSelector::addOptionsAndInit (const QDomElement &element) {
	RK_TRACE (PLUGIN);

	// get xml-helper
	XMLHelper *xml = parentComponent ()->xmlHelper ();

	// create all the options
	XMLChildList option_elements = xml->getChildElements (element, "option", DL_ERROR);	
	int selected = 0;
	int i = 0;
	for (XMLChildList::const_iterator it = option_elements.cbegin (); it != option_elements.cend (); ++it) {
		QString label = xml->i18nStringAttribute (*it, "label", QString (), DL_ERROR);
		QString value = xml->getStringAttribute (*it, "value", QString (), DL_WARNING);
		QString name = xml->getStringAttribute (*it, "id", QString (), DL_INFO);

		Option *opt = new Option;
		opt->value = value;
		opt->enabledness_prop = nullptr;

		options.insert (i, opt);
		if (!name.isNull ()) named_options.insert (name, opt);

		addOptionToGUI (label, i);

		if (xml->getBoolAttribute (*it, "checked", false, DL_INFO)) {
			selected = i;
		}

		++i;
	}

	updating = false;
	number->setMin (0);
	number->setMax (i-1);
	number->setIntValue (selected);		// will also take care of activating the correct item
}

RKComponentBase* RKAbstractOptionSelector::lookupComponent (const QString &identifier, QString *remainder) {
	RK_TRACE (PLUGIN);

	if (identifier.isEmpty ()) return this;

	QString name = identifier.section ('.', 0, 0);
	if (named_options.contains (name)) {
		Option *opt = named_options[name];

		QString mod = identifier.section ('.', 1);
		if (mod != "enabled") {
			RK_DEBUG (PLUGIN, DL_DEBUG, "options do not have property '%s'", mod.toLatin1().data ());
			return this;
		}

		if (!(opt->enabledness_prop)) {		// requested for the first time
			opt->enabledness_prop = new RKComponentPropertyBool (this, false);
			connect (opt->enabledness_prop, &RKComponentPropertyBase::valueChanged, this, &RKAbstractOptionSelector::ItemPropertyChanged);
		}

		return (opt->enabledness_prop);
	}

	return RKComponent::lookupComponent (identifier, remainder);
}

void RKAbstractOptionSelector::propertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	if (updating) return;

	int new_id = -1;
	if (property == string) {
		new_id = findOption (fetchStringValue (string));
	} else if (property == number) {
		new_id = number->intValue ();
	} else {
		RK_ASSERT (false);
	}
	if (new_id < 0) {
		RK_DEBUG (PLUGIN, DL_ERROR, "option selector '%s' has no such option:", qPrintable (getIdInParent ()));
/*		RKComponent *p = this; 
		while (p) {
			qDebug ("p: %s", qPrintable (p->getIdInParent ()));
			p = p->parentComponent();
		} */
		if (property == string) {
			RK_DEBUG (PLUGIN, DL_ERROR, "'%s'", qPrintable (fetchStringValue (string)));
		} else {
			RK_DEBUG (PLUGIN, DL_ERROR, "index %d", number->intValue ());
		}
		return;
	}

	updating = true;
	setItemInGUI (new_id);
	itemSelected (new_id);		// slot not called automatically on programmed changes!
	updating = false;

	changed ();
}

void RKAbstractOptionSelector::ItemPropertyChanged (RKComponentPropertyBase *property) {
	RK_TRACE (PLUGIN);

	Option *opt = nullptr;
	int id = -1;
	for (OptionsMap::const_iterator it = options.cbegin(); it != options.cend(); ++it) {
		RK_ASSERT (it.value ());
		if (it.value ()->enabledness_prop == property) {
			opt = it.value ();
			id = it.key ();
			break;
		}
	}

	if ((!opt) || (id < 0)) {
		RK_ASSERT (false);
		return;
	}

	RK_ASSERT (property->type () == RKComponent::PropertyBool);
	bool enabled = static_cast<RKComponentPropertyBool*> (property)->boolValue ();

	if (!enabled) {
		if (id == number->intValue ()) {	// current item was disabled
			int settable_opt = -1;
			for (OptionsMap::const_iterator it = options.cbegin(); it != options.cend(); ++it) {
				RK_ASSERT (it.value ());

				if ((!(it.value ()->enabledness_prop)) || (it.value ()->enabledness_prop->boolValue ())) {
					settable_opt = it.key();
					break;
				}
			}
			if (settable_opt >= 0) itemSelected (settable_opt);
			else RK_DEBUG (PLUGIN, DL_ERROR, "No option left enabled. Disable the entire component '%s', instead!", qPrintable (getIdInParent ()));
			// TODO: Error message below can also trigger, if items first get disabled, then others enabled. Delay it, instead.
		}
	}

	setItemEnabledInGUI (id, enabled);
}

void RKAbstractOptionSelector::itemSelected (int id) {
	RK_TRACE (PLUGIN);

	Option *opt = options[id];
	RK_ASSERT (opt);

	string->setValue (opt->value);
	number->setIntValue (id);
}

int RKAbstractOptionSelector::findOption (const QString &option_string) {
	RK_TRACE (PLUGIN);

	for (OptionsMap::const_iterator it = options.cbegin(); it != options.cend(); ++it) {
		RK_ASSERT (it.value ());
		if (it.value ()->value == option_string) return (it.key ());
	}
	return -1;
}

