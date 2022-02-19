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

#include "../rbackend/rcommand.h"

class RKMDIWindow;
class RCommandChain;
class RKOutputDirectory;

// convenience struct to avoid defining separate functions for R API and C++ API;
// this struct encapsulates the relevant results for both
struct RKOutputDirectoryCallResult : public GenericRRequestResult {
	RKOutputDirectoryCallResult() : GenericRRequestResult(), _dir(nullptr) {};
	RKOutputDirectoryCallResult(const GenericRRequestResult &other) : GenericRRequestResult(other), _dir(nullptr) {};
	void setDir(RKOutputDirectory *d);
	RKOutputDirectory* dir() const { return _dir; }
private:
	RKOutputDirectory* _dir;
};

class RKOutputDirectory : public QObject {
	Q_OBJECT
public:
	enum OverwriteBehavior {
		Force,
		Ask,
		Fail
	};
	GenericRRequestResult activate(RCommandChain* chain=0);
	GenericRRequestResult revert(OverwriteBehavior discard=Ask);
	GenericRRequestResult save(const QString& dest=QString(), OverwriteBehavior overwrite=Ask);
	GenericRRequestResult exportAs(const QString& dest=QString(), OverwriteBehavior overwrite=Ask);
	GenericRRequestResult clear(OverwriteBehavior discard=Ask);
	GenericRRequestResult purge(OverwriteBehavior discard=Ask, RCommandChain* chain=0, bool activate_other=true);
	QString getId() const { return id; };
	bool isEmpty() const;
	bool isActive() const;
	bool isModified() const;
	GenericRRequestResult view(bool raise);
	QString filename() const { return save_filename; };
	QString workDir() const { return work_dir; }
	QString workPath() const;
	QString caption() const;
	static GenericRRequestResult handleRCall(const QStringList& params, RCommandChain *chain);
	static RKOutputDirectory* getOutputById(const QString& id);
	static RKOutputDirectory* getOutputBySaveUrl(const QString& dest);
	static RKOutputDirectory* getOutputByWindow(const RKMDIWindow* window);
/** Return a list of all current output directories that have been modified. Used for asking for save during shutdown. */
	static QList<RKOutputDirectory*> modifiedOutputDirectories();

/** Returns the active output (in case there is one).
 *  If no output is active, find and activate the next output without a save url.
 *  If that does not exist, activate and return the next existing output.
 *  If that does not exist, create a new output, activate and return it. */
	static RKOutputDirectoryCallResult getCurrentOutput(RCommandChain *chain=0);
	static RKOutputDirectoryCallResult get(const QString &filename=QString(), bool create=false, RCommandChain *chain=0);
	static QList<RKOutputDirectory*> allOutputs();
	static void purgeAllNoAsk();
private:
	RKOutputDirectory();
	~RKOutputDirectory();
	void updateSavedHash();

	QString saved_hash;
	QDateTime save_timestamp;
	QString work_dir;
	QString save_filename;
	QString id;
	bool initialized;

	/** map of outputs. */
	static QMap<QString, RKOutputDirectory*> outputs;

	GenericRRequestResult import(const QString& from);
	static RKOutputDirectory* createOutputDirectoryInternal();
	GenericRRequestResult importZipInternal(const QString &from);
	GenericRRequestResult exportZipInternal(const QString &dest);
};

#endif

