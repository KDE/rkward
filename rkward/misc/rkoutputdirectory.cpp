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
#include "../windows/rkworkplace.h"
#include "../misc/rkcommonfunctions.h"
#include "../agents/rkquitagent.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

/** much like `ls -Rl`. List directory contents including timestamps and sizes, recursively. Used to detect whether an output directory has any changes. */
void listDirectoryState(const QString& _dir, QString *list, const QString prefix) {
	RK_TRACE(APP);

	QDir dir(_dir);
	QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name | QDir::DirsLast);
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

QMap<QString, RKOutputDirectory*> RKOutputDirectory::outputs;

RKOutputDirectory::RKOutputDirectory() : initialized(false) {
	RK_TRACE(APP);
}

RKOutputDirectory::~RKOutputDirectory() {
	RK_TRACE(APP);
}

RKOutputDirectory* RKOutputDirectory::getOutputById(const QString& id) {
	RK_TRACE (APP);

	return outputs.value(id);
}

RKOutputDirectory* RKOutputDirectory::getOutputBySaveUrl(const QString& _dest) {
	RK_TRACE (APP);

	QString dest = QFileInfo(_dest).canonicalFilePath();
	for (auto it = outputs.constBegin(); it != outputs.constEnd(); ++it) {
		if (it.value()->save_filename == dest) {
			return(it.value());
		}
	}
	return nullptr;
}

RKOutputDirectory* RKOutputDirectory::getOutputByWindow(const RKMDIWindow *window) {
	RK_TRACE (APP);

	if (!window) return nullptr;
	if (!window->isType(RKMDIWindow::OutputWindow)) return nullptr;
	for (auto it = outputs.constBegin(); it != outputs.constEnd(); ++it) {
		if (it.value()->workPath() == static_cast<const RKHTMLWindow*>(window)->url().toLocalFile()) {
			return(it.value());
		}
	}
	return nullptr;
}

GenericRRequestResult RKOutputDirectory::save(const QString& _dest, RKOutputDirectory::OverwriteBehavior overwrite) {
	RK_TRACE (APP);

	QString dest = _dest;
	if (dest.isEmpty()) {
		dest = filename();  // might still be empty, in which case exportAs will ask for destination
		if (!dest.isEmpty()) overwrite = Force;  // don't prompt when writing to same file
	}
	GenericRRequestResult res = exportAs(dest, overwrite);
	if (!res.failed()) {
		updateSavedHash();
		save_filename = res.ret.toString();  // might by different from dest, notably, if dest was empty
	}
	return res;
}

GenericRRequestResult RKOutputDirectory::exportAs (const QString& _dest, RKOutputDirectory::OverwriteBehavior overwrite) {
	RK_TRACE (APP);

	QString dest = _dest;
	if (dest.isEmpty()) {
		QFileDialog dialog(RKWardMainWindow::getMain(), i18n("Select destination file name"), QFileInfo(save_filename).absolutePath());
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setNameFilters(QStringList() << i18n("RKWard Output Files [*.rko](*.rko)") << i18n("All Files [*](*)"));
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.setOption(QFileDialog::DontConfirmOverwrite, true);  // custom handling below

		if (dialog.exec () != QDialog::Accepted) {
			return GenericRRequestResult::makeError(i18n("File selection cancelled"));
		}

		dest = QDir::cleanPath(dialog.selectedFiles().value(0));
	}

	bool exists = QFileInfo(dest).exists();
	if (exists) {
		if (overwrite == Ask) {
			const QString warning = i18n("Are you sure you want to overwrite the existing file '%1'?", dest);
			KMessageBox::ButtonCode res = KMessageBox::warningContinueCancel(RKWardMainWindow::getMain(), warning, i18n("Overwrite?"), KStandardGuiItem::overwrite(),
												KStandardGuiItem::cancel(), QString(), KMessageBox::Options(KMessageBox::Notify | KMessageBox::Dangerous));
			if (KMessageBox::Continue != res) return GenericRRequestResult::makeError(i18n("User cancelled"));
		} else if (overwrite != Force) {
			return GenericRRequestResult::makeError(i18n("Not overwriting existing file"));
		}
	}

	return exportZipInternal(dest);
}

#include <KZip>
GenericRRequestResult RKOutputDirectory::exportZipInternal(const QString &dest) {
	RK_TRACE (APP);

	// write to a temporary location, first, then - if successful - copy to final destination
	QString tempname = dest + '~';
	while (QFileInfo(tempname).exists()) {
		tempname.append('~');
	}

	KZip zip(tempname);
	bool ok = zip.open(QIODevice::WriteOnly);
	zip.addLocalDirectory(work_dir, "rkward_output");
	ok = ok && zip.close();
	if (!ok) {
		QFile(tempname).remove();
		return GenericRRequestResult::makeError(i18n("Error while writing temporary output archive %1", tempname));
	}

	if (QFile(dest).exists()) {
		if (!QFile(dest).remove()) return GenericRRequestResult::makeError(i18n("Failure to remove existing file %1. Missing permissions?s", dest));
	}
	ok = QFile(tempname).copy(dest);
	QFile(tempname).remove();
	if (!ok) {
		return GenericRRequestResult::makeError(i18n("Failure while copying output archive to %1", dest));
	}

	return GenericRRequestResult(QFileInfo(dest).canonicalFilePath()); // return effective destination path. Needed by save()
}

#include <KArchiveDirectory>
GenericRRequestResult RKOutputDirectory::importZipInternal(const QString &_from) {
	RK_TRACE (APP);

	QFileInfo fi(_from);
	if (!fi.isFile()) {
		return GenericRRequestResult::makeError(i18n("The path %1 does not exist or is not a file.", _from));
	}
	QString from = fi.canonicalFilePath();

	KZip zip(from);
	bool ok = zip.open(QIODevice::ReadOnly);
	if (ok) {
		auto dir = zip.directory()->entry("rkward_output");
		if (!(dir && dir->isDirectory())) ok = false;
		if (ok && !static_cast<const KArchiveDirectory*>(dir)->copyTo(work_dir, true)) ok = false;
	}
	if (!ok) return GenericRRequestResult::makeError(i18n("Failure to open %1. Not an rkward output file?", from));

	save_filename = from;
	updateSavedHash();
	initialized = true;

	return GenericRRequestResult(QVariant(id));
}

GenericRRequestResult RKOutputDirectory::import(const QString& _dir) {
	RK_TRACE (APP);

	if (initialized) {
		return GenericRRequestResult::makeError(i18n("Output directory %1 is already in use.", id));
	}

	return importZipInternal(_dir);
}

GenericRRequestResult RKOutputDirectory::revert(OverwriteBehavior discard) {
	RK_TRACE (APP);

	if (save_filename.isEmpty()) {
		return GenericRRequestResult::makeError(i18n("Output has not previously been saved. Cannot revert."));
	}
	if (!isModified()) return GenericRRequestResult(id, i18n("Output had no modifications. Nothing reverted."));
	if (discard == Ask) {
		if (KMessageBox::warningContinueCancel(RKWardMainWindow::getMain(), i18n("Reverting will destroy any changes, since the last time you saved (%1). Are you sure you want to proceed?", save_timestamp.toString())) == KMessageBox::Continue) {
			discard = Force;
		}
	}
	if (discard != Force) {
		return GenericRRequestResult::makeError(i18n("User cancelled."));
	}

	return importZipInternal(save_filename);
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

	auto d = new RKOutputDirectory();
	d->work_dir = QFileInfo(ddir.absoluteFilePath(destname)).canonicalFilePath();
	d->id = d->work_dir;
	d->initialized = false;
	outputs.insert(d->id, d);
	return d;
}

GenericRRequestResult RKOutputDirectory::activate(RCommandChain* chain) {
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

	return GenericRRequestResult(QVariant(index_file));
}

GenericRRequestResult RKOutputDirectory::clear(OverwriteBehavior discard) {
	RK_TRACE(APP);

	if (!isEmpty()) {
		if (discard == Ask) {
			if (KMessageBox::warningContinueCancel(RKWardMainWindow::getMain(), i18n("Clearing will destroy any unsaved changes, and - upon saving - also saved changes. Are you sure you want to proceed?")) == KMessageBox::Continue) {
				discard = Force;
			}
		}
		if (discard != Force) {
			return GenericRRequestResult::makeError(i18n("Output is not empty. Not clearing it."));
		}
	}

	QDir dir(work_dir);
	dir.removeRecursively();
	dir.mkpath(".");
	initialized = false;
	if (isActive()) activate();

	return GenericRRequestResult();
}

bool RKOutputDirectory::isEmpty() const {
	RK_TRACE(APP);

	if (!save_filename.isEmpty()) return false;  // we _could_ have saved an empty output, of course, but no worries about corner cases. In any doubt we return false.

	if (!initialized) return true;
	if (!isModified()) return true;   // because we have not saved/loaded this file, before, see above
	return false;
}

bool RKOutputDirectory::isModified() const {
	RK_TRACE(APP);

	return saved_hash != hashDirectoryState(work_dir);
}

QString RKOutputDirectory::caption() const {
	RK_TRACE(APP);
	if (!save_filename.isEmpty()) return QFileInfo(save_filename).fileName();
	return i18n("Unsaved output");
}

GenericRRequestResult RKOutputDirectory::purge(RKOutputDirectory::OverwriteBehavior discard, RCommandChain* chain, bool activate_other) {
	RK_TRACE(APP);

	if (isModified()) {
		if (discard == Fail) {
			return GenericRRequestResult::makeError(i18n("Output has been modified. Not closing it."));
		}
		if (discard == Ask) {
			auto res = KMessageBox::questionYesNoCancel(RKWardMainWindow::getMain(), i18n("The output has been modified, and closing it will discard all changes. What do you want to do?"), i18n("Discard unsaved changes?"), KStandardGuiItem::discard(), KStandardGuiItem::save(), KStandardGuiItem::cancel());
			if (res == KMessageBox::Cancel) {
				return GenericRRequestResult::makeError(i18n("User cancelled"));
			}
			if (res == KMessageBox::No) {
				auto ret = save();
				if (ret.failed()) return ret;
			}
		}
	}

	bool activate = activate_other && isActive();
	QDir dir(work_dir);
	dir.removeRecursively();
	outputs.remove(id);
	deleteLater();
	GenericRRequestResult messages;
	if (activate) getCurrentOutput(chain, &messages);
	return messages;
}

void RKOutputDirectory::purgeAllNoAsk() {
	RK_TRACE(APP);

	auto outputs_copy = outputs;
	for (auto it = outputs_copy.constBegin(); it != outputs_copy.constEnd(); ++it) {
		auto res = outputs.constBegin().value()->purge(Force, nullptr, false);
		RK_ASSERT(!res.failed());
	}
}

QString RKOutputDirectory::workPath() const {
	RK_TRACE(APP);

	// hardcoded for now, might be made to support several files in the future
	return (QDir(work_dir).absoluteFilePath("index.html"));
}

bool RKOutputDirectory::isActive() const {
	RK_TRACE(APP);

	return RKOutputWindowManager::self()->currentOutputPath() == workPath();
}

QList<RKOutputDirectory*> RKOutputDirectory::modifiedOutputDirectories() {
	RK_TRACE (APP);

	QList<RKOutputDirectory*> ret;
	for (auto it = outputs.constBegin(); it != outputs.constEnd(); ++it) {
		if (it.value()->isModified()) ret.append(it.value());
	}
	return ret;
}

void RKOutputDirectory::updateSavedHash() {
	RK_TRACE (APP);
	saved_hash = hashDirectoryState(work_dir);
	save_timestamp = QDateTime::currentDateTime();
}

QList<RKOutputDirectory *> RKOutputDirectory::allOutputs() {
	RK_TRACE(APP);

	QList<RKOutputDirectory*> ret;
	for (auto it = outputs.constBegin (); it != outputs.constEnd (); ++it) {
		ret.append(it.value());
	}
	return ret;
}

RKOutputDirectory* RKOutputDirectory::getCurrentOutput(RCommandChain* chain, GenericRRequestResult* message_res) {
	RK_TRACE(APP);

	if (outputs.isEmpty()) {
		auto n = createOutputDirectoryInternal();
		n->activate(chain);
		if (message_res) message_res->addMessages(GenericRRequestResult(QVariant(), i18n("New empty output directory has been created, automatically")));
		return n;
	}

	RKOutputDirectory* candidate = nullptr;
	for (auto it = outputs.constBegin(); it != outputs.constEnd(); ++it) {
		if (it.value()->isActive()) return it.value();
		if (it.value()->filename().isEmpty()) candidate = it.value();
	}

	if (!candidate) candidate = outputs[0];
	RK_ASSERT(candidate);
	candidate->activate(chain);
	if (message_res) message_res->addMessages(GenericRRequestResult(QVariant(), i18n("Output has been activated, automatically")));
	return candidate;
}

RKOutputDirectory::OverwriteBehavior parseOverwrite(const QString &param) {
	if (param == QStringLiteral("ask")) return RKOutputDirectory::Ask;
	if (param == QStringLiteral("force")) return RKOutputDirectory::Force;
	return RKOutputDirectory::Fail;
}

GenericRRequestResult RKOutputDirectory::view(bool raise) {
	RK_TRACE(APP);

	auto list = RKOutputWindowManager::self()->existingOutputWindows(workPath());
	if (!list.isEmpty()) {
		auto w = list[0];
		if (!w->isActiveWindow() || raise) {
			list[0]->activate();
		}
	} else {
		RKWorkplace::mainWorkplace()->openOutputWindow(QUrl::fromLocalFile(workPath()));
	}
	return GenericRRequestResult(id);
}

GenericRRequestResult RKOutputDirectory::handleRCall(const QStringList& params, RCommandChain *chain) {
	RK_TRACE(APP);

	QString command = params.value(0);
	if (command == QStringLiteral("get")) {
		if (params.value(1) == QStringLiteral("all")) {
			QStringList ret;
			auto all = allOutputs();
			for (int i=0; i < all.size(); ++i) {
				ret.append(all[i]->getId());
			}
			return GenericRRequestResult(ret);
		}

		QString filename = params.value(3);
		bool create = (params.value(2) == QStringLiteral("create"));
		RKOutputDirectory *out = nullptr;
		GenericRRequestResult messages;
		if (filename.isEmpty()) {
			if (create) {
				out = createOutputDirectoryInternal();
			} else {
				out = getCurrentOutput(chain, &messages);
			}
		} else {
			filename = QFileInfo(filename).canonicalFilePath();
			out = getOutputBySaveUrl(filename);
			if (create) {
				if (out) return GenericRRequestResult::makeError(i18n("Output '1%' is already loaded in this session. Cannot create it.", filename));
				if (QFileInfo(filename).exists()) return GenericRRequestResult::makeError(i18n("As file named '1%' already exists. Cannot create it.", filename));
				out = createOutputDirectoryInternal();
				return (out->import(filename));
			} else {
				if (!out) {
					out = createOutputDirectoryInternal();
					return (out->import(filename));
				}
			}
		}
		return GenericRRequestResult(out->getId()).addMessages(messages);
	} else {
		// all other commands pass the output id as second parameter. Look that up, first
		QString id = params.value(1);
		auto out = getOutputById(id);
		if (!out) {
			return GenericRRequestResult::makeError(i18n("The output identified by '%1' is not loaded in this session.", id));
		}

		if (command == QStringLiteral("activate")) {
			return out->activate(chain);
		} else if (command == QStringLiteral("isEmpty")) {
			return GenericRRequestResult(out->isEmpty());
		} else if (command == QStringLiteral("isModified")) {
			return GenericRRequestResult(out->isModified());
		} else if (command == QStringLiteral("revert")) {
			return out->revert(parseOverwrite(params.value(2)));
		} else if (command == QStringLiteral("save")) {
			return out->save(params.value(3), parseOverwrite(params.value(2)));
		} else if (command == QStringLiteral("export")) {
			return out->exportAs(params.value(3), parseOverwrite(params.value(2)));
		} else if (command == QStringLiteral("clear")) {
			return out->clear(parseOverwrite(params.value(2)));
		} else if (command == QStringLiteral("close")) {
			return out->purge(parseOverwrite(params.value(2)), chain);
		} else if (command == QStringLiteral("view")) {
			return out->view(params.value(2) == QStringLiteral("raise"));
		} else if (command == QStringLiteral("workingDir")) {
			return GenericRRequestResult(out->workDir());
		} else if (command == QStringLiteral("filename")) {
			return GenericRRequestResult(out->filename());
		}
	}
	return GenericRRequestResult::makeError(i18n("Unhandled output command '%1'", command));
}


#include "rkoutputdirectory.moc"
