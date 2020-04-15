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

#include <functional>
/** Simple helper class to formalize the API of widgets used for the interactive validation of settings.
 *  (For quering about settings that may need adjusting on startup. Possibly to be expanded to a "first-run-wizard", in the future). */
class RKSettingsWizardPage : public QWidget {
public:
	RKSettingsWizardPage (QWidget* parent=0) : QWidget (parent) {
		apply_callback = 0;
	}
	~RKSettingsWizardPage () {};
	void apply () {
		if (apply_callback) apply_callback();
	}
	void setApplyCallback (std::function<void()> callback) {
		apply_callback = callback;
	};
private:
	std::function<void()> apply_callback;
};

#endif
