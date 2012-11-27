/***************************************************************************
                          rkabstractoptionselector  -  description
                             -------------------
    begin                : Tue Mar 20 2007
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

#ifndef RKABSTRACTOPTIONSELECTOR
#define RKABSTRACTOPTIONSELECTOR

#include "rkcomponent.h"
#include "rkcomponentproperties.h"

#include <qmap.h>

class QComboBox;
class QDomElement;

/** Base class for option selection plugin widgets, like RKRadio and RKDropDown. This can not be used directly, due to pure virtual functions.
@author Thomas Friedrichsmeier
*/
class RKAbstractOptionSelector : public RKComponent {
	Q_OBJECT
public: 
	RKAbstractOptionSelector (RKComponent *parent_component, QWidget *parent_widget);
	~RKAbstractOptionSelector ();
/** Find the option number with the corresponding string. If not found, returns -1
@param option_string the option string to search for
@returns the id (0, 1, 2...) of the corresponding option, or -1 if not found */
	int findOption (const QString &option_string);
	QVariant value (const QString &modifier=QString ()) { return (string->value (modifier)); };
/** reimplemented from RKComponent to add enabledness properties for the options, dynamically, if requested */
	RKComponentBase* lookupComponent (const QString &identifier, QString *remainder);
public slots:
	void itemSelected (int id);
	void propertyChanged (RKComponentPropertyBase *property);
	void ItemPropertyChanged (RKComponentPropertyBase *property);
protected:
	virtual void setItemInGUI (int id) = 0;
	virtual void addOptionToGUI (const QString &label, int id) = 0;
	virtual void setItemEnabledInGUI (int id, bool enabled)= 0;
	void addOptionsAndInit (const QDomElement &element);
private:
	RKComponentPropertyBase *string;
	RKComponentPropertyInt *number;

	struct Option {
		QString value;
		RKComponentPropertyBool *enabledness_prop;
	};

	bool updating;		// prevent recursion
	typedef QMap<int, Option*> OptionsMap;
	typedef QMap<QString, Option*> OptionsLookup;
	OptionsMap options;
	OptionsLookup named_options;
};

#endif
