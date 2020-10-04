/***************************************************************************
                          rksettingsmodule  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004-2018 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RKSETTINGSMODULE_H
#define RKSETTINGSMODULE_H

#include <qstring.h>
#include <qwidget.h>
#include <QUrl>
#include <KConfigGroup>

class KConfig;
class RKSettings;
class RCommandChain;

/** Base class for RKWard config settings.
 *
 *  Meant to reduce the amount of boilerplate code involved in defining, loading, saving config settings (and in the future probably also creating widgets and applying changes). */
class RKConfigBase {
public:
	virtual void loadConfig(KConfigGroup &cg) = 0;
	virtual void saveConfig(KConfigGroup &cg) const = 0;
protected:
	RKConfigBase(const char* name) : name(name) {};
	virtual ~RKConfigBase() {};
	const char* name;
};

/** A single value stored in the RKWard config file.
 *
 *  The value set initially (in the constructor or via setDefaultValue() represents the default value. */
template<typename T> class RKConfigValue : public RKConfigBase {
public:
	RKConfigValue(const char* name, const T &default_value) : RKConfigBase(name), value(default_value) {};
	~RKConfigValue() {};

	void loadConfig(KConfigGroup &cg) override {
		value = cg.readEntry(name, value);
	}
	void saveConfig(KConfigGroup &cg) const override {
		cg.writeEntry(name, value);
	}
	void setDefaultValue(const T& value) { RKConfigValue<T>::value = value; }
	operator T() const { return(value); }
	RKConfigValue& operator= (const T v) { value = v; return *this; };
private:
	T value;
};

#include <vector>
#include <initializer_list>
/** A group of values stored in the RKWard config file.
 *
 *  If a name is given and non-empty, loadConfig()/saveConfig() will load from/save to a sub config-group by that name. */
class RKConfigGroup : public RKConfigBase {
public:
	RKConfigGroup(const char *name, std::initializer_list<RKConfigBase*> values) : RKConfigBase(name),
		values(values) {};
	template<typename T> RKConfigGroup(const char *name, size_t count, RKConfigValue<T>* _values) : RKConfigBase(name),
		values(count) { for (size_t i = 0; i < count; ++i) values[i] = (_values + i); };
	~RKConfigGroup() {};
	void loadConfig(KConfigGroup &cg) override {
		KConfigGroup lcg = cg;
		if (name && name[0]) {
			lcg = cg.group(name);
		}
		for (auto it = values.begin(); it != values.end(); ++it) (*it)->loadConfig(lcg);
	}
	void saveConfig(KConfigGroup &cg) const override {
		KConfigGroup lcg = cg;
		if (name && name[0]) {
			lcg = cg.group(name);
		}
		for (auto it = values.begin(); it != values.end(); ++it) (*it)->saveConfig(lcg);
	}
private:
	std::vector<RKConfigBase*> values;
};

/**
Base class for settings modules. Provides some pure virtual calls.

@author Thomas Friedrichsmeier
*/
class RKSettingsModule : public QWidget {
public:
	RKSettingsModule (RKSettings *gui, QWidget *parent);
	virtual ~RKSettingsModule ();

	bool hasChanges () { return changed; };
	virtual void applyChanges () = 0;
	virtual void save (KConfig *config) = 0;
	
	virtual QString caption () = 0;
/** Some settings modules execute R commands on "apply". If an RCommandChain is specified for the RKSettings-dialog, those commands should
be inserted into this chain. It's safe to use this unconditionally, as if there is no chain, this will return 0, which corresponds to using the top-level chain */
	RCommandChain *commandChain () { return chain; };

	virtual QUrl helpURL () { return QUrl (); };
protected:
friend class RKSettingsModuleWidget;
	void change ();

	bool changed;
private:
	RKSettings *gui;
friend class RKSettings;
	static RCommandChain *chain;
};

/** Base class for UI widgets operating on an RKSettingsModule. For now this is used, only where similar settings are shared across modules (e.g. code completion). Eventually, this could be used to disentangle RKSettingsModule from QWidget. */
class RKSettingsModuleWidget : public QWidget {
public:
	RKSettingsModuleWidget(QWidget *parent, RKSettingsModule *_module) : QWidget(parent), module(_module) {};
	~RKSettingsModuleWidget() {};
	virtual void applyChanges() = 0;
protected:
	void change() { module->change(); }
private:
	RKSettingsModule *module;
};

class RKSetupWizardItem;

#endif
