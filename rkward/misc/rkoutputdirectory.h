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

class RKOutputDirectory {
public:
	RKOutputDirectory();
	~RKOutputDirectory();
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
		bool failed() { !error.isEmpty(); }
		bool toBool() { return ret.toBool(); }
	};
	enum OverwriteBehavior {
		Force,
		Ask,
		Fail
	};
	void activate(RCommandChain* chain=0);
	bool isEmpty();
	GenericRCallResult revert(bool ask);
	GenericRCallResult createOrImport(const QUrl from);
	GenericRCallResult save(const QString& dest=QString(), OverwriteBehavior overwrite=Ask);
	GenericRCallResult exportAs(const QString& dest=QString(), OverwriteBehavior overwrite=Ask);
	GenericRCallResult clear(OverwriteBehavior discard=Ask);
	GenericRCallResult purge(OverwriteBehavior discard=Ask);
	QString getId() const { return id; };
	void view(bool raise);
	QString filename();
	QString workDir();
	static GenericRCallResult handleRCall(const QStringList& params);
	static RKOutputDirectory* getOpenDirectory(const QString id, GenericCallResult* result);
private:
	QString saved_hash;
	QDateTime save_timestamp;
	QString work_dir;
	QString save_dir;
	QString id;
	bool initialized;
	/** map of outputs. */
	static QMap<QString, RKOutputDirectory> outputs;

	void backendActivateOutputDirectory (const QString& dir, RCommandChain* chain);
	QString createOutputDirectoryInternal ();
	static bool isRKWwardOutputDirectory (const QString &dir);
	static bool isOutputDirectoryModified (const QString &dir);
	QString dropOutputDirectoryInternal (const QString& dir);

};

#endif

