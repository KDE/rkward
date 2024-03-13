/*
rkabstractoptionselector - This file is part of the RKWard project. Created: Tue Mar 20 2007
SPDX-FileCopyrightText: 2007-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	QVariant value (const QString &modifier=QString ()) override { return (string->value (modifier)); };
/** reimplemented from RKComponent to add enabledness properties for the options, dynamically, if requested */
	RKComponentBase* lookupComponent (const QString &identifier, QString *remainder) override;
public Q_SLOTS:
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
