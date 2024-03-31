/*
rkoutputdirectory - This file is part of the RKWard project. Created: Mon Oct 12 2020
SPDX-FileCopyrightText: 2020-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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

/** Convenience struct to avoid defining separate functions for R API and C++ API in RKOutputDirectory.
 *  This struct encapsulates the relevant results for both */
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
	GenericRRequestResult activate(RCommandChain* chain=nullptr);
	GenericRRequestResult revert(OverwriteBehavior discard=Ask);
	GenericRRequestResult save(const QString& dest=QString(), OverwriteBehavior overwrite=Ask);
	GenericRRequestResult exportAs(const QString& dest=QString(), OverwriteBehavior overwrite=Ask);
	GenericRRequestResult clear(OverwriteBehavior discard=Ask);
	GenericRRequestResult purge(OverwriteBehavior discard=Ask, RCommandChain* chain=nullptr, bool activate_other=true);
	QString getId() const { return id; };
	bool isEmpty() const;
	bool isActive() const;
	/** This function is guaranteed to be accurate, but relatively slow. At least too slow to be called on every update of caption. */
	bool isModifiedAccurate() const;
	/** This function may not always be accurate, but is fast. It is fairly reliable as long as there is an active view, but should not be used when there is not.  */
	bool isModifiedFast() const;
	GenericRRequestResult view(bool raise, RCommandChain* chain=nullptr);
	RKMDIWindow* getOrCreateView(bool raise, RCommandChain* chain=nullptr);
	QString filename() const { return save_filename; };
	QString workDir() const { return work_dir; }
	QString workPath() const;
	QString caption() const;
	static GenericRRequestResult handleRCall(const QStringList& params, RCommandChain *chain);
	static RKOutputDirectory* findOutputById(const QString& id);
	static RKOutputDirectory* findOutputByWorkPath(const QString& workpath);
	static RKOutputDirectory* findOutputByWindow(const RKMDIWindow* window);
/** Return a list of all current output directories that have been modified. Used for asking for save during shutdown. */
	static QList<RKOutputDirectory*> modifiedOutputDirectories();

/** Returns the active output (in case there is one).
 *  If no output is active, find and activate the next output without a save url.
 *  If that does not exist, activate and return the next existing output.
 *  If that does not exist, create a new output, activate and return it. */
	static RKOutputDirectoryCallResult getCurrentOutput(RCommandChain *chain=nullptr);
	static RKOutputDirectoryCallResult get(const QString &filename=QString(), bool create=false, RCommandChain *chain=nullptr);
	static QList<RKOutputDirectory*> allOutputs();
	static void purgeAllNoAsk();

	void setKnownModified(bool modified);
Q_SIGNALS:
	void stateChange(bool active, bool modified);
private:
	RKOutputDirectory();
	~RKOutputDirectory();
	void updateSavedHash();
	/** Currently active output. Could be nullptr in corner cases! */
	RKOutputDirectory* activeOutput();
	/** Note dest must be normalized. Hiding as private to avoid mis-use. */
	static RKOutputDirectory* findOutputBySaveUrl(const QString& dest);

	QString saved_hash;
	QDateTime save_timestamp;
	QString work_dir;
	QString save_filename;
	QString id;
	bool initialized;
	bool known_modified;  // TODO: needed?

	/** map of outputs. */
	static QMap<QString, RKOutputDirectory*> outputs;

	GenericRRequestResult import(const QString& from);
	static RKOutputDirectory* createOutputDirectoryInternal();
	GenericRRequestResult importZipInternal(const QString &from);
	GenericRRequestResult exportZipInternal(const QString &dest);
};

#endif

