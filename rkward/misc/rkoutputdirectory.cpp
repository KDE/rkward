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

#include "rkoutputdirectory.h"

#include <QDir>
#include <QFileInfo>
#include <QFileDialog>

#include <KLocalizedString>
#include <KMessageBox>

#include "../settings/rksettingsmodulegeneral.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rkrinterface.h"
#include "../windows/rkmdiwindow.h"
#include "../windows/rkhtmlwindow.h"
#include "../misc/rkcommonfunctions.h"
#include "../agents/rkquitagent.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

/** much like `ls -Rl`. List directory contents including timestamps and sizes, recursively. Used to detect whether an output directory has any changes. */
void listDirectoryState(const QString& _dir, QString *list, const QString prefix) {
	RK_TRACE(APP);

	QDir dir(_dir);
	QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot, QDir::Name | QDir::DirsLast);
	for (int i = 0; i < entries.size(); ++i) {
		const QFileInfo fi = entries[i];
		if (fi.isDir()) {
			listDirectoryState(fi.absolutePath (), list, prefix + '/' + fi.fileName());
		} else {
			list->append(fi.fileName () + '\t');
			list->append(fi.lastModified ().toString ("dd.hh.mm.ss.zzz") + '\t');
			list->append(QString::number (fi.size()) + '\n');
		}
	}
}

QString hashDirectoryState(const QString& dir) {
	RK_TRACE(APP);
	QString list;
	listDirectoryState(dir, &list, QString());
	return list;
//	return QCryptographicHash::hash (list.toUtf8 (), QCryptographicHash::Md5);
}

bool copyDirRecursively(const QString& _source_dir, const QString& _dest_dir) {
	RK_TRACE(APP);

	QDir dest_dir(_dest_dir);
	QDir source_dir(_source_dir);
	if (!QDir().mkpath(_dest_dir)) return false;
	if (!source_dir.exists()) return false;

	bool ok = true;
	QFileInfoList entries = source_dir.entryInfoList(QDir::NoDotAndDotDot);
	for (int i = 0; i < entries.size (); ++i) {
		const QFileInfo fi = entries[i];
		if (fi.isDir()) {
			ok = ok && copyDirRecursively(fi.absoluteFilePath(), dest_dir.absoluteFilePath(fi.fileName ()));
		} else {
			// NOTE: this does not overwrite existing target files, but in our use case, we're always writing into empty targets
			ok = ok && QFile::copy(fi.absoluteFilePath(), dest_dir.absoluteFilePath(fi.fileName ()));
		}
	}

	return ok;
}

RKOutputDirectory::RKOutputDirectory() : initialized(false), window(nullptr) {
	RK_TRACE(APP);
}

RKOutputDirectory::~RKOutputDirectory() {
	RK_TRACE(APP);
}

RKOutputDirectory* RKOutputDirectory::getOutputById(const QString& id, GenericRCallResult* result) {
	RK_TRACE (APP);

	if (!outputs.contains(id)) {
		if (result) result->error = i18n("The output identified by '%1' is not loaded in this session.", id);
		return 0;
	}
	return outputs[id];
}

RKOutputDirectory* RKOutputDirectory::getOutputBySaveUrl(const QString& _dest, bool create) {
	RK_TRACE (APP);

	if (_dest.isEmpty()) {
		if (!create) return RKOutputWindowManager::self()->getCurrentOutput();
		return createOutputDirectoryInternal();
	}

	QString dest = QFileInfo(_dest).canonicalFilePath();

	auto it = outputs.constBegin();
	while (it != outputs.constEnd()) {
		if (it.value()->save_dir == dest) {
			return(it.value());
		}
	}

# error move to handleRCall
	// not returned, yet? We need to create and import an output
	auto ret = createOutputDirectoryInternal();
	ret->import(_dest);
	return ret;
}

RKOutputDirectory::GenericRCallResult RKOutputDirectory::save(const QString& dest, RKOutputDirectory::OverwriteBehavior overwrite) {
	RK_TRACE (APP);
	GenericRCallResult res = exportAs(dest, overwrite);
	if (!res.failed()) {
		updateSavedHash();
		save_dir = res.ret.toString(); // might by different from dest, notably, if dest was empty
	}
	return res;
}

RKOutputDirectory::GenericRCallResult RKOutputDirectory::exportAs (const QString& _dest, RKOutputDirectory::OverwriteBehavior overwrite) {
	RK_TRACE (APP);

	QString dest = _dest;
	if (dest.isEmpty()) {
		QFileDialog dialog(RKWardMainWindow::getMain(), i18n("Specify directory where to export output"), save_dir);
		dialog.setFileMode(QFileDialog::Directory);
		dialog.setOption(QFileDialog::ShowDirsOnly, true);
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.setOption(QFileDialog::DontConfirmOverwrite, true);  // custom handling below

		if (dialog.exec () != QDialog::Accepted) {
			return GenericRCallResult::makeError(i18n("File selection cancelled"));
		}

		dest = QDir::cleanPath(dialog.selectedFiles().value(0));
	}

	// If destination already exists, rename it before copying, so we can restore the save in case of an error
	QString tempname;

	dest = QFileInfo(dest).canonicalFilePath();
	bool exists = QFileInfo(dest).exists();
	if (exists) {
#warning TODO Check terminology ("output directory") once finalized
		if (!isRKWwardOutputDirectory(dest)) {
			return GenericRCallResult::makeError(i18n("The directory %1 exists, but does not appear to be an RKWard output directory. Refusing to overwrite it.", dest));
		}
/*		if (isOutputDirectoryModified(dest)) {
			return GenericRCallResult::makeError(i18n("The output directory %1 has been modified by an external process. Refusing to overwrite it.", dest));
		} */
		if (overwrite == Ask) {
			const QString warning = i18n("Are you sure you want to overwrite the existing directory '%1'? All current contents, <b>including subdirectories</b> will be lost.", dest);
			KMessageBox::ButtonCode res = KMessageBox::warningContinueCancel(RKWardMainWindow::getMain(), warning, i18n("Overwrite Directory?"), KStandardGuiItem::overwrite(),
												KStandardGuiItem::cancel(), QString(), KMessageBox::Options(KMessageBox::Notify | KMessageBox::Dangerous));
			if (KMessageBox::Continue != res) return GenericRCallResult::makeError(i18n("User cancelled"));
		} else if (overwrite != Force) {
			return GenericRCallResult::makeError(i18n("Not overwriting existing output"));
		}

		tempname = dest + '~';
		while (QFileInfo(tempname).exists()) {
			tempname.append('~');
		}
		if (!QDir().rename(dest, tempname)) {
			return GenericRCallResult::makeError(i18n("Failed to create temporary backup file %1.", tempname));
		}
	}

	bool error = copyDirRecursively(work_dir, dest);
	if (error) {
		if (!tempname.isEmpty()) {
			QDir().rename(tempname, dest);
		}
		return GenericRCallResult::makeError(i18n("Error while copying %1 to %2", work_dir, dest));
	}
	if (!tempname.isEmpty()) {
		QDir(tempname).removeRecursively();
	}

	return GenericRCallResult(QVariant(dest));  // return effective destination path. Needed by save()
}

RKOutputDirectory::GenericRCallResult RKOutputDirectory::import(const QString& _dir) {
	RK_TRACE (APP);

	if (initialized) {
		return GenericRCallResult::makeError(i18n("Output directory %1 is already in use.", work_dir));
	}
	QFileInfo fi(_dir);
	if (!fi.isDir()) {
		return GenericRCallResult::makeError(i18n("The path %1 does not exist or is not a directory.", _dir));
	}
	QString dir = fi.canonicalFilePath ();

	if (!copyDirRecursively(dir, work_dir)) {
		QDir(work_dir).removeRecursively();
		return GenericRCallResult::makeError(i18n("The path %1 could not be imported (copy failure).", _dir));
	}

	save_dir = dir;
	updateSavedHash();
	initialized = true;

	return GenericRCallResult(QVariant(id));
}

RKOutputDirectory* RKOutputDirectory::createOutputDirectoryInternal() {
	RK_TRACE (APP);

	QString prefix = QStringLiteral("unsaved_output");
	QString destname = prefix;
	QDir ddir(RKSettingsModuleGeneral::filesPath());
	int x = 0;
	while (ddir.exists(destname)) {
		destname = prefix + QString::number(x++);
	}
	ddir.mkpath(destname);

	QFile marker(destname + QStringLiteral("/rkward_output_marker.txt"));
	marker.open(QIODevice::WriteOnly);
	marker.write(i18n("This file is used to indicate that this directory is an ouptut directory created by RKWard (http://rkward.kde.org). Do not place any other contents in this directory, as the entire directory will be purged if/when overwriting the output.\nRKWard will ask you before purging this directory (unless explicitly told otherwise), but if you remove this file, RKWard will not purge this directory.\n").toLocal8Bit());
	marker.close();

	auto d = new RKOutputDirectory();
	d->work_dir = QFileInfo(ddir.absoluteFilePath(destname)).canonicalFilePath();
	d->id = d->work_dir;
	d->initialized = false;
	outputs.insert(d->id, d);
	return d;
}

RKOutputDirectory::GenericRCallResult RKOutputDirectory::activate(RCommandChain* chain) {
	RK_TRACE (APP);

	QString index_file = work_dir + "/index.html";
	RKGlobals::rInterface()->issueCommand(QStringLiteral("rk.set.output.html.file(\"") + RKCommonFunctions::escape(index_file) + QStringLiteral("\")\n"), RCommand::App, QString(), 0, 0, chain);
	if(!initialized) {
		// when an output directory is first initialized, we don't want that to count as a "modification". Therefore, update the "saved hash" _after_ initialization
		RCommand *command = new RCommand(QString(), RCommand::App | RCommand::Sync | RCommand::EmptyCommand);
		connect(command->notifier(), &RCommandNotifier::commandFinished, this, &RKOutputDirectory::updateSavedHash);
		RKGlobals::rInterface()->issueCommand(command, chain);
		initialized = true;
	}

	return GenericRCallResult(QVariant(index_file));
}

RKOutputDirectory::GenericRCallResult RKOutputDirectory::clear(OverwriteBehavior discard) {
	RK_TRACE(APP);

	if (!isEmpty()) {
		if (discard == Ask) {
			if (KMessageBox::warningContinueCancel(RKWardMainWindow::getMain(), i18n("Clearing will destroy any unsaved changes, and - upon saving - also saved changes. Are you sure you want to proceed?")) == KMessageBox::Continue) {
				discard = Force;
			}
			if (discard != Force) {
				return GenericRCallResult::makeError(i18n("Output is not empty. Not clearing it."));
			}
		}
	}

	QDir dir(work_dir);
	dir.removeRecursively();
	dir.mkpath(".");
	initialized = false;
	if (isActive()) activate();
}

bool RKOutputDirectory::isEmpty() const {
	RK_TRACE(APP);

	if (!save_dir.isEmpty()) return false;  // we _could_ have saved an empty output, of course, but no worries about corner cases. In any doubt we return false.

	if (!initialized) return true;
	if (!isModified()) return true;   // because we have not saved/loaded this file, before, see above
	return false;
}

RKOutputDirectory::GenericRCallResult RKOutputDirectory::purge(RKOutputDirectory::OverwriteBehavior discard) {
	RK_TRACE(APP);

	if (isModified()) {
		if (discard == Fail) {
			return GenericRCallResult::nakeError(i18n("Output has been modified. Not closing it."));
		}
		if (discard == Ask) {
			auto res = KMessageBox::questionYesNoCancel(RKWardMainWindow::getMain(), i18n("The output has been modified, and closing it will discard all changes. What do you want to do?"), i18n("Discard unsaved changes?"), KStandardGuiItem::discard(), KStandardGuiItem::save(), KStandardGuiItem::cancel());
			if (res == KMessageBox::Cancel) {
				return GenericRCallResult::nakeError(i18n("User cancelled"));
			}
			if (res == KMessageBox::No) {
				auto ret = save();
				if (ret.failed()) return ret;
			}
		}
	}

	QDir dir(work_dir);
	dir.removeRecursively();
	if (isActive()) {
		if (RKQuitAgent::quittingInProgress()) {
			RK_DEBUG(APP, DEBUG, "Skipping activation of new output file: quitting in progress");
		} else {
			activateDefault();
		}
	}
	outputs.remove(id);
	deleteLater();
	return GenericRCallResult();
}

bool RKOutputDirectory::isActive() const {
	RK_TRACE(APP);

	return RKOutputWindowManager::self()->currentDefaultPath().startsWith(work_dir);
}

QStringList RKOutputDirectory::modifiedOutputDirectories() const {
	RK_TRACE (APP);

	QStringList ret;
	for (auto it = outputs.constBegin (); it != outputs.constEnd (); ++it) {
		if (it.value()->isModified()) ret.append(it.key());
	}
	return ret;
}

bool RKOutputDirectory::isRKWwardOutputDirectory(const QString& dir) {
	RK_TRACE (APP);

	return (QDir(dir).exists(QStringLiteral("rkward_output_marker.txt")));
}

void RKOutputDirectory::updateSavedHash() {
	RK_TRACE (APP);

	saved_hash = hashDirectoryState(work_dir);
	save_timestamp = QDateTime::currentDateTime();
}

#include "rkoutputdirectory.moc"
