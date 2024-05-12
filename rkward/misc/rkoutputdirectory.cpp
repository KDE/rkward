/*
rkoutputdirectory - This file is part of RKWard (https://rkward.kde.org). Created: Mon Oct 12 2020
SPDX-FileCopyrightText: 2020-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkoutputdirectory.h"

#include <QDir>
#include <QFileInfo>
#include <QFileDialog>

#include <KLocalizedString>
#include <KMessageBox>

#include "../settings/rksettingsmodulegeneral.h"
#include "../settings/rksettingsmoduleoutput.h"
#include "../settings/rkrecenturls.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rkrinterface.h"
#include "../windows/rkmdiwindow.h"
#include "../windows/rkhtmlwindow.h"
#include "../windows/rkworkplace.h"
#include "../misc/rkcommonfunctions.h"
#include "../agents/rkquitagent.h"
#include "../rkward.h"

#include "../debug.h"

/** much like `ls -Rl`. List directory contents including timestamps and sizes, recursively. Used to detect whether an output directory has any changes. */
void listDirectoryState(const QString& _dir, QString *list, const QString &prefix) {
	RK_TRACE(APP);

	QDir dir(_dir);
	QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name | QDir::DirsLast);
	for (int i = 0; i < entries.size(); ++i) {
		const QFileInfo fi = entries[i];
		if (fi.isDir()) {
			listDirectoryState(fi.absolutePath(), list, prefix + '/' + fi.fileName());
		} else {
			list->append(fi.fileName() + '\t');
			list->append(fi.lastModified().toString("dd.hh.mm.ss.zzz") + '\t');
			list->append(QString::number(fi.size()) + '\n');
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

void RKOutputDirectoryCallResult::setDir(RKOutputDirectory *d) {
	_dir = d;
	if (d) ret = d->getId();
}

QMap<QString, RKOutputDirectory*> RKOutputDirectory::outputs;

RKOutputDirectory::RKOutputDirectory() : initialized(false), known_modified(false) {
	RK_TRACE(APP);
}

RKOutputDirectory::~RKOutputDirectory() {
	RK_TRACE(APP);
}

RKOutputDirectory* RKOutputDirectory::findOutputById(const QString& id) {
	RK_TRACE (APP);

	return outputs.value(id);
}

RKOutputDirectory* RKOutputDirectory::findOutputByWorkPath(const QString& workpath) {
	RK_TRACE (APP);

	if (workpath.endsWith("index.html")) {
		QString wp = workpath;
		return(outputs.value(wp.chopped(11)));  // index.html, including pathsep
	}
	return nullptr;
}

RKOutputDirectory* RKOutputDirectory::findOutputBySaveUrl(const QString& dest) {
	RK_TRACE (APP);

	for (auto it = outputs.constBegin(); it != outputs.constEnd(); ++it) {
		if (it.value()->save_filename == dest) {
			return(it.value());
		}
	}
	return nullptr;
}

RKOutputDirectory* RKOutputDirectory::findOutputByWindow(const RKMDIWindow *window) {
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

GenericRRequestResult RKOutputDirectory::save(const QString& dest, RKOutputDirectory::OverwriteBehavior overwrite) {
	RK_TRACE (APP);

	GenericRRequestResult res = exportAs(dest, overwrite);
	if (!res.failed()) {
		save_filename = res.ret.toString();  // might by different from dest, notably, if dest was empty or not yet normalized
		known_modified = true;  // dirty trick to ensure that updateSavedHash() will trigger a stateChange()->update caption in views, even if using SaveAs on an unmodified directory
		updateSavedHash();
		RKRecentUrls::addRecentUrl(RKRecentUrls::outputId(), QUrl::fromLocalFile(save_filename));
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
			return GenericRRequestResult::makeError(i18n("File selection canceled"));
		}

		dest = QDir::cleanPath(dialog.selectedFiles().value(0));
	}

	bool exists = QFileInfo::exists(dest);
	if (exists && (dest != save_filename)) {
		if (overwrite == Ask) {
			const QString warning = i18n("Are you sure you want to overwrite the existing file '%1'?", dest);
			KMessageBox::ButtonCode res = KMessageBox::warningContinueCancel(RKWardMainWindow::getMain(), warning, i18n("Overwrite?"), KStandardGuiItem::overwrite(),
												KStandardGuiItem::cancel(), QString(), KMessageBox::Options(KMessageBox::Notify | KMessageBox::Dangerous));
			if (KMessageBox::Continue != res) return GenericRRequestResult::makeError(i18n("User canceled"));
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
	while (QFileInfo::exists(tempname)) {
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
		if (!QFile(dest).remove()) return GenericRRequestResult::makeError(i18n("Failure to remove existing file %1. Missing permissions?", dest));
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

	RKRecentUrls::addRecentUrl(RKRecentUrls::outputId(), QUrl::fromLocalFile(_dir));
	return importZipInternal(_dir);
}

GenericRRequestResult RKOutputDirectory::revert(OverwriteBehavior discard) {
	RK_TRACE (APP);

	if (!isModifiedAccurate()) return GenericRRequestResult(id, i18n("Output had no modifications. Nothing reverted."));
	if (discard == Ask) {
		if (save_filename.isEmpty()) {
			if (KMessageBox::warningContinueCancel(RKWardMainWindow::getMain(), i18n("This output has not been saved before. Reverting will clear it, entirely. Are you sure you want to proceed?"), QString(), KStandardGuiItem::clear()) == KMessageBox::Continue) {
				return clear(Force);
			}
		} else if (KMessageBox::warningContinueCancel(RKWardMainWindow::getMain(), i18n("Reverting will destroy any changes, since the last time you saved (%1). Are you sure you want to proceed?", save_timestamp.toString())) == KMessageBox::Continue) {
			discard = Force;
		}
	}
	if (discard != Force) {
		return GenericRRequestResult::makeError(i18n("User canceled."));
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
	outputs.insert(d->id, d);
	return d;
}

GenericRRequestResult RKOutputDirectory::activate(RCommandChain* chain) {
	RK_TRACE (APP);

	QString index_file = work_dir + "/index.html";
	RInterface::issueCommand(new RCommand(QStringLiteral("rk.set.output.html.file(\"") + RKCommonFunctions::escape(index_file) + QStringLiteral("\")\n"), RCommand::App), chain);
	if (!initialized) {
		// when an output directory is first initialized, we don't want that to count as a "modification". Therefore, update the "saved hash" _after_ initialization
		RInterface::whenAllFinished(this, [this]() { updateSavedHash(); }, chain);
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
		known_modified = true;
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
	if (!isModifiedAccurate()) return true;   // because we have not saved/loaded this file, before, see above
	return false;
}

bool RKOutputDirectory::isModifiedAccurate() const {
	RK_TRACE(APP);

	return saved_hash != hashDirectoryState(work_dir);
}

QString RKOutputDirectory::caption() const {
	RK_TRACE(APP);
	if (!save_filename.isEmpty()) return QFileInfo(save_filename).fileName();
	return i18n("[Unnamed]");
}

GenericRRequestResult RKOutputDirectory::purge(RKOutputDirectory::OverwriteBehavior discard, RCommandChain* chain, bool activate_other) {
	RK_TRACE(APP);

	if ((discard != Force) && isModifiedAccurate()) {
		if (discard == Fail) {
			return GenericRRequestResult::makeError(i18n("Output has been modified. Not closing it."));
		}
		if (discard == Ask) {
			auto res = KMessageBox::questionTwoActionsCancel(RKWardMainWindow::getMain(), i18n("The output has been modified, and closing it will discard all changes. What do you want to do?"), i18n("Discard unsaved changes?"), KStandardGuiItem::discard(), KStandardGuiItem::save(), KStandardGuiItem::cancel());
			if (res == KMessageBox::Cancel) {
				return GenericRRequestResult::makeError(i18n("User canceled"));
			}
			if (res == KMessageBox::SecondaryAction) {
				auto ret = save(save_filename);
				if (ret.failed()) return ret;
			}
		}
	}
	auto list = RKOutputWindowManager::self()->existingOutputWindows(workPath());
	for (int i = 0; i < list.size(); ++i) {
		list[i]->close(RKMDIWindow::NoAskSaveModified);
	}

	QDir dir(work_dir);
	dir.removeRecursively();
	outputs.remove(id);
	deleteLater();
	if (activate_other) return getCurrentOutput(chain);
	return GenericRRequestResult();
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
		if (it.value()->isModifiedAccurate()) ret.append(it.value());
	}
	return ret;
}

void RKOutputDirectory::updateSavedHash() {
	RK_TRACE (APP);
	saved_hash = hashDirectoryState(work_dir);
	save_timestamp = QDateTime::currentDateTime();
	setKnownModified(false);
}

QList<RKOutputDirectory *> RKOutputDirectory::allOutputs() {
	RK_TRACE(APP);

	QList<RKOutputDirectory*> ret;
	for (auto it = outputs.constBegin (); it != outputs.constEnd (); ++it) {
		ret.append(it.value());
	}
	return ret;
}

RKOutputDirectoryCallResult RKOutputDirectory::getCurrentOutput(RCommandChain* chain) {
	RK_TRACE(APP);

	RKOutputDirectoryCallResult ret;
	if (outputs.isEmpty()) {
		if (RKSettingsModuleOutput::sharedDefaultOutput()) {
			QString filename = RKSettingsModuleGeneral::filesPath() + "default.rko";
			ret = get(filename, !QFileInfo::exists(filename), chain);
			if (ret.dir()) {
				ret.dir()->activate(chain);
				return ret;
			}
			// otherwise: fallthrough to below, just in case
		}

		auto n = createOutputDirectoryInternal();
		n->activate(chain);
		ret.addMessages(GenericRRequestResult(QVariant(), i18n("New empty output directory has been created, automatically")));
		ret.setDir(n);
		return ret;
	}

	RKOutputDirectory* candidate = nullptr;
	for (auto it = outputs.constBegin(); it != outputs.constEnd(); ++it) {
		if (it.value()->isActive()) {
			ret.setDir(it.value());
			return ret;
		}
		if (it.value()->filename().isEmpty()) candidate = it.value();
	}

	if (!candidate) candidate = outputs.constBegin().value();
	RK_ASSERT(candidate);
	candidate->activate(chain);
	ret.addMessages(GenericRRequestResult(QVariant(), i18n("Output has been activated, automatically")));
	ret.setDir(candidate);
	return ret;
}

RKOutputDirectory::OverwriteBehavior parseOverwrite(const QString &param) {
	if (param == QStringLiteral("ask")) return RKOutputDirectory::Ask;
	if (param == QStringLiteral("force")) return RKOutputDirectory::Force;
	return RKOutputDirectory::Fail;
}

RKOutputDirectory* RKOutputDirectory::activeOutput() {
	for (auto it = outputs.constBegin(); it != outputs.constEnd(); ++it) {
		if (it.value()->isActive()) {
			return it.value();
		}
	}
	return nullptr;
}

RKMDIWindow * RKOutputDirectory::getOrCreateView(bool raise, RCommandChain* chain) {
	RK_TRACE(APP);

	if (!initialized) {
		// currently, activating is the only way to init. That's a bit lame, but then, viewing an uninitialzed output is rather a corner case, anyway.
		RKOutputDirectory *active = activeOutput();
		activate(chain);
		if (active) active->activate();
	}
	auto list = RKOutputWindowManager::self()->existingOutputWindows(workPath());
	if (!list.isEmpty()) {
		auto w = list[0];
		if (!w->isActiveWindow() || raise) {
			list[0]->activate();
		}

		return w;
	}

	if (!known_modified) known_modified = isModifiedAccurate();
	return RKWorkplace::mainWorkplace()->openNewOutputWindow(this);
}

GenericRRequestResult RKOutputDirectory::view(bool raise, RCommandChain* chain) {
	RK_TRACE(APP);

	getOrCreateView(raise, chain);
	return GenericRRequestResult(id);
}

RKOutputDirectoryCallResult RKOutputDirectory::get(const QString &_filename, bool create, RCommandChain *chain) {
	RK_TRACE(APP);

	RKOutputDirectoryCallResult ret;
	if (_filename.isEmpty()) {
		if (create) {
			ret.setDir(createOutputDirectoryInternal());
		} else {
			return (getCurrentOutput(chain));
		}
	} else {  // filename not empty
		QFileInfo fi(_filename);
		bool file_exists = fi.exists();
		QString filename = file_exists ? fi.canonicalFilePath() : _filename;
		RKOutputDirectory *dir = findOutputBySaveUrl(filename);
		// NOTE: annoyingly QFileInfo::canonicalFilePath() returns an empty string, if the file does not exist
		if (create) {
			if (dir) return GenericRRequestResult::makeError(i18n("Output '%1' is already loaded in this session. Cannot create it.", filename));
			if (file_exists) return GenericRRequestResult::makeError(i18n("A file named '%1' already exists. Cannot create it.", filename));

			// NOTE: This may seem a bit of an unusual case: Creating a fresh output with save file name already specified.
			// However, this actually happens, when using a shared default output file.
			// To avoid issues (esp. around canonicalFilePath()), we make sure to initialize and save the new output, right away (much like "touch"), instead of only setting the save file name for later usage
			ret.setDir(dir = createOutputDirectoryInternal());
			dir->save_filename = filename;
			// this is a bit cumbersome. TODO: create a dedicated function to init an output.
			RCommand *command = new RCommand(QStringLiteral("local({o <- rk.output(); n <- rk.output(\"") + RKCommonFunctions::escape(filename) + QStringLiteral("\"); n$activate(); n$save(); try(o$activate(), silent=TRUE); o$save() })\n"), RCommand::App);
			RInterface::issueCommand(command, chain);
		} else {
			if (!(file_exists || dir)) return GenericRRequestResult::makeError(i18n("File '%1' does not exist.", filename));

			if (dir) {
				ret.setDir(dir);
			} else {
				dir = createOutputDirectoryInternal();
				ret.addMessages(dir->import(filename));
				if (ret.failed()) {
					dir->purge(Force, chain, false);
				} else {
					ret.setDir(dir);
				}
			}
		}
	}
	return ret;
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
		return get(filename, create);
	} else {
		// all other commands pass the output id as second parameter. Look that up, first
		QString id = params.value(1);
		auto out = findOutputById(id);
		if (!out) {
			return GenericRRequestResult::makeError(i18n("The output identified by '%1' is not loaded in this session.", id));
		}

		if (command == QStringLiteral("activate")) {
			return out->activate(chain);
		} else if (command == QStringLiteral("isEmpty")) {
			return GenericRRequestResult(out->isEmpty());
		} else if (command == QStringLiteral("isModified")) {
			return GenericRRequestResult(out->isModifiedAccurate());
		} else if (command == QStringLiteral("revert")) {
			return out->revert(parseOverwrite(params.value(2)));
		} else if (command == QStringLiteral("save")) {
			QString filename = params.value(3);
			if (filename.isEmpty()) filename = out->filename();
			return out->save(filename, parseOverwrite(params.value(2)));
		} else if (command == QStringLiteral("export")) {
			return out->exportAs(params.value(3), parseOverwrite(params.value(2)));
		} else if (command == QStringLiteral("clear")) {
			return out->clear(parseOverwrite(params.value(2)));
		} else if (command == QStringLiteral("close")) {
			return out->purge(parseOverwrite(params.value(2)), chain);
		} else if (command == QStringLiteral("view")) {
			return out->view(params.value(2) == QStringLiteral("raise"), chain);
		} else if (command == QStringLiteral("workingDir")) {
			return GenericRRequestResult(out->workDir());
		} else if (command == QStringLiteral("filename")) {
			return GenericRRequestResult(out->filename());
		}
	}
	return GenericRRequestResult::makeError(i18n("Unhandled output command '%1'", command));
}

void RKOutputDirectory::setKnownModified(bool modified) {
	RK_TRACE(APP);
	if (known_modified != modified) {
		known_modified = modified;
		Q_EMIT stateChange(isActive(), modified);
	}
}

bool RKOutputDirectory::isModifiedFast() const {
	return known_modified;
}

