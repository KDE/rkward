/***************************************************************************
                          rkoutputdirectory  -  description
                             -------------------
    begin                : Mon Oct 12 2020
    copyright            : (C) 2020 by Thomas Friedrichsmeier
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

#ifndef RKOUTPUTDIRECTORY_H
#define RKOUTPUTDIRECTORY_H

#include <QString>
#include <QUrl>
#include <QVariant>
#include <QDateTime>
#include <QPointer>
#include <QObject>

class RKMDIWindow;
class RCommandChain;

class RKOutputDirectory : public QObject {
	Q_OBJECT
public:
#warning TODO: Move me to backend
	/** Standard wrapper for the result of a "generic" API call, such that we need less special-casing in the backend. The conventions are simple:
	 *  - If @param error is non-null, stop() will be called in the backend, with the given message.
	 *  - If @param warning is non-null, the message will be shown (as a warning), but no error will be raised.
	 *  - Unless an error was thrown, @param ret will be returned as a basic data type (possibly NULL). */
	struct GenericRCallResult {
		GenericRCallResult(const QVariant& ret=QVariant(), const QString& warning=QString(), const QString& error=QString()) :
			error(error), warning(warning), ret(ret) {};
		static GenericRCallResult makeError(const QString& error) {
			return GenericRCallResult(QVariant(), QString(), error);
		}
		QString error;
		QString warning;
		QVariant ret;
		bool failed() { return !error.isEmpty(); }
		bool toBool() { return ret.toBool(); }
	};
	enum OverwriteBehavior {
		Force,
		Ask,
		Fail
	};
	GenericRCallResult activate(RCommandChain* chain=0);
	GenericRCallResult revert(OverwriteBehavior discard=Ask);
	GenericRCallResult save(const QString& dest=QString(), OverwriteBehavior overwrite=Ask);
	GenericRCallResult exportAs(const QString& dest=QString(), OverwriteBehavior overwrite=Ask);
	GenericRCallResult clear(OverwriteBehavior discard=Ask);
	GenericRCallResult purge(OverwriteBehavior discard=Ask, RCommandChain* chain=0);
	QString getId() const { return id; };
	bool isEmpty() const;
	bool isActive() const;
	bool isModified() const;
	GenericRCallResult view(bool raise);
	QString filename() const { return save_dir; };
	QString workDir() const { return work_dir; }
	QString workPath() const;
	QString caption() const;
	static GenericRCallResult handleRCall(const QStringList& params, RCommandChain *chain);
	static RKOutputDirectory* getOutputById(const QString& id);
	static RKOutputDirectory* getOutputBySaveUrl(const QString& dest);
/** Return a list of all current output directories that have been modified. Used for asking for save during shutdown. */
	static QList<RKOutputDirectory*> modifiedOutputDirectories();

	static RKOutputDirectory::GenericRCallResult R_rk_Output(const QString& filename=QString(), bool create=false, bool all=false);
/** Returns the active output (in case there is one).
 *  If no output is active, find an activate the next output without a save url.
 *  If that does not exist, activate and return the next existing output.
 *  If that does not exist, create a new output, activate and return it. */
	static RKOutputDirectory* getCurrentOutput(RCommandChain *chain=0);
	static QList<RKOutputDirectory*> allOutputs();
private:
	RKOutputDirectory();
	~RKOutputDirectory();
	void updateSavedHash();

	QString saved_hash;
	QDateTime save_timestamp;
	QString work_dir;
	QString save_dir;
	QString id;
	bool initialized;

	QPointer<RKMDIWindow> window;
	/** map of outputs. */
	static QMap<QString, RKOutputDirectory*> outputs;

	GenericRCallResult import(const QString& from);
	static RKOutputDirectory* createOutputDirectoryInternal();
	static bool isRKWwardOutputDirectory (const QString &dir);
	GenericRCallResult importInternal(const QString &dir);
};

#endif

