/*
rksettingsmodule - This file is part of the RKWard project. Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULE_H
#define RKSETTINGSMODULE_H

#include <KConfigGroup>
#include <QIcon>
#include <QUrl>
#include <qstring.h>
#include <qwidget.h>

class KConfig;
class RKSettings;
class RCommandChain;
class QCheckBox;
class QComboBox;
class QGroupBox;
class RKSettingsModule;
class RKSettingsModuleWidget;
class RKSetupWizardItem;
class RKSpinBox;
class QAction;
class QLineEdit;

using namespace Qt::Literals::StringLiterals;

/** Base class for RKWard config settings.
 *
 *  Meant to reduce the amount of boilerplate code involved in defining, loading, saving config settings (and in the future probably also creating widgets and applying changes). */
class RKConfigBase {
  public:
	virtual void loadConfig(KConfigGroup &cg) = 0;
	virtual void saveConfig(KConfigGroup &cg) const = 0;
	enum ConfigSyncAction {
		SaveConfig,
		LoadConfig
	};
	/** Save or load config value (a somewhat dirty trick that saves a lot of tying... */
	void syncConfig(KConfigGroup &cg, ConfigSyncAction a) {
		if (a == SaveConfig) saveConfig(cg);
		else loadConfig(cg);
	};
	const char *key() { return name; }

	struct ValueLabel {
		int key;
		QString label;
	};
	typedef QList<ValueLabel> LabelList;

  protected:
	RKConfigBase(const char *name) : name(name){};
	virtual ~RKConfigBase(){};
	const char *name;

	static QComboBox *makeDropDownHelper(const LabelList &entries, RKSettingsModuleWidget *module, int initial, std::function<void(int)> setter);
};

/** A single value stored in the RKWard config file.
 *
 *  The value set initially (in the constructor or via setDefaultValue() represents the default value. */
template <typename T, typename STORAGE_T = T>
class RKConfigValue : public RKConfigBase {
  public:
	RKConfigValue(const char *name, const T &default_value) : RKConfigBase(name), value(default_value){};
	~RKConfigValue(){};

	void loadConfig(KConfigGroup &cg) override {
		value = (T)cg.readEntry(name, (STORAGE_T)value);
	}
	void saveConfig(KConfigGroup &cg) const override {
		cg.writeEntry(name, (STORAGE_T)value);
	}
	void setDefaultValue(const T &value) { RKConfigValue<T>::value = value; }
	operator T() const { return (value); }
	T &get() { return (value); }
	RKConfigValue &operator=(const T v) {
		value = v;
		return *this;
	};

	/** Only for bool values: convenience function to create a fully connected checkbox for this option */
	QCheckBox *makeCheckbox(const QString &label, RKSettingsModuleWidget *module);
	/** Only for bool values: convenience function to create a fully connected checkable groupbox for this option */
	QGroupBox *makeCheckableGroupBox(const QString &label, RKSettingsModuleWidget *module);
	/** Currently only for boolean or int options: Make a dropdown selector (QComboBox). If bit_flag_mask is set, the selector operates on (part of) an OR-able set of flags, instead of
	 *  plain values. */
	QComboBox *makeDropDown(const LabelList &entries, RKSettingsModuleWidget *_module, int bit_flag_mask = 0) {
		static_assert(std::is_same<STORAGE_T, int>::value || std::is_same<STORAGE_T, bool>::value, "makeDropDown can only be used for int or bool");
		if (bit_flag_mask) {
			return makeDropDownHelper(entries, _module, ((int)value) & bit_flag_mask, [this, bit_flag_mask](int val) { this->value = (T)((((int)this->value) & ~bit_flag_mask) + val); });
		} else {
			return makeDropDownHelper(entries, _module, (std::is_same<STORAGE_T, bool>::value && value) ? 1 : (int)value, [this](int val) { this->value = (T)val; });
		}
	}
	/** Only for QString values: Create a fully connected QLineEdit for this option */
	QLineEdit *makeLineEdit(RKSettingsModuleWidget *module);
	RKSpinBox *makeSpinBox(T min, T max, RKSettingsModuleWidget *_module);
	/** For settings to be used outside the settings dialog: Create an appropriate action connected to this value. Call @param handler with the new value, whenever the action is triggered,
	 *  and - for convenience - once while creating the action (for intialization) */
	QAction *makeAction(QObject *parent, const QString &label, std::function<void(T)> handler);

  private:
	T value;
};

#include <initializer_list>
#include <vector>
/** A group of values stored in the RKWard config file.
 *
 *  If a name is given and non-empty, loadConfig()/saveConfig() will load from/save to a sub config-group by that name. */
class RKConfigGroup : public RKConfigBase {
  public:
	RKConfigGroup(const char *name, std::initializer_list<RKConfigBase *> values) : RKConfigBase(name),
	                                                                                values(values){};
	template <typename T>
	RKConfigGroup(const char *name, size_t count, RKConfigValue<T> *_values) : RKConfigBase(name),
	                                                                           values(count) {
		for (size_t i = 0; i < count; ++i)
			values[i] = (_values + i);
	}
	~RKConfigGroup(){};
	void loadConfig(KConfigGroup &cg) override {
		KConfigGroup lcg = cg;
		if (name && name[0]) {
			lcg = cg.group(QString::fromLatin1(name));
		}
		for (auto it = values.begin(); it != values.end(); ++it)
			(*it)->loadConfig(lcg);
	}
	void saveConfig(KConfigGroup &cg) const override {
		KConfigGroup lcg = cg;
		if (name && name[0]) {
			lcg = cg.group(QString::fromLatin1(name));
		}
		for (auto it = values.begin(); it != values.end(); ++it)
			(*it)->saveConfig(lcg);
	}

  private:
	std::vector<RKConfigBase *> values;
};

class RKSettingsModuleWidget;

/**
Base class for settings modules. This provides a wrapper around the stored settings.

UI pages (for the settings dialog) are created via pages().

@author Thomas Friedrichsmeier
*/
class RKSettingsModule : public QObject {
	Q_OBJECT
  public:
	RKSettingsModule(QObject *parent);
	virtual ~RKSettingsModule();
	/** Some settings modules execute R commands on "apply". If an RCommandChain is specified for the RKSettings-dialog, those commands should
	be inserted into this chain. It's safe to use this unconditionally, as if there is no chain, this will return 0, which corresponds to using the top-level chain */
	RCommandChain *commandChain() { return chain; };
	typedef QLatin1StringView PageId;
	static constexpr PageId no_page_id = QLatin1String("");
  Q_SIGNALS:
	void settingsChanged();

  protected:
	virtual void createPages(RKSettings *parent) = 0;

  private:
	friend class RKSettings;
	static RCommandChain *chain;
	virtual void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction) = 0;
	virtual void validateSettingsInteractive(QList<RKSetupWizardItem *> *){};
};

/** Base class for UI widgets operating on an RKSettingsModule. For now this is used, only where similar settings are shared across modules (e.g. code completion). Eventually, this could be used to disentangle RKSettingsModule from QWidget. */
class RKSettingsModuleWidget : public QWidget {
	Q_OBJECT
  public:
	RKSettingsModuleWidget(QWidget *parent, RKSettingsModule *parent_module, const RKSettingsModule::PageId pageid, const RKSettingsModule::PageId superpageid = RKSettingsModule::no_page_id);
	~RKSettingsModuleWidget(){};
	virtual void applyChanges() = 0;
	/** Mark this module as "changed" (propagates to parent module) */
	void change();
	bool hasChanges() const { return changed; };
	virtual QString longCaption() const { return windowTitle(); };
	RKSettingsModule *parentModule() const { return parent_module; };
	void doApply() {
		Q_EMIT apply();
		applyChanges();
		changed = false;
	}
  Q_SIGNALS:
	void settingsChanged();
	void apply();

  protected:
	QUrl helpURL() { return help_url; };
	bool changed;
	/** temporary indirection until applyChanges() has been obsolete, everywhere */
	friend class RKSettings;
	const RKSettingsModule::PageId pageid;
	const RKSettingsModule::PageId superpageid;
	RKSettingsModule *parent_module;
	QUrl help_url;
};

#endif
