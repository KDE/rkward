/*
rkvariable - This file is part of RKWard (https://rkward.kde.org). Created: Thu Aug 12 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkvariable.h"

#include "float.h"
#include <cmath>
#include <qstringlist.h>

#include "../rbackend/rkrinterface.h"
#include "rcontainerobject.h"
#include "robjectlist.h"

#include "rkmodificationtracker.h"

#define MAX_PRECISION DBL_DIG

#include "../debug.h"

RKVariable::RKVariable(RContainerObject *parent, const QString &name) : RObject(parent, name) {
	RK_TRACE(OBJECTS);
	type = Variable;
	data = nullptr;
	setDataType(RObject::DataNumeric);
}

RKVariable::~RKVariable() {
	RK_TRACE(OBJECTS);

	RK_ASSERT(!data); // endEdit() should have been called
}

void RKVariable::setVarType(RObject::RDataType new_type, bool sync) {
	RK_TRACE(OBJECTS);

	if (getDataType() == new_type) return;
	if ((new_type < RObject::MinKnownDataType) || (new_type > RObject::MaxKnownDataType)) {
		new_type = RObject::DataCharacter;
	}

	// if the variable is currently opened for editing, all values need to be rechecked / resynced
	if (data) {
		// quick and dirty approach! TODO: make more efficient
		QStringList list;
		list.reserve(getLength());
		bool labelled = (new_type == DataCharacter);
		for (int i = 0; i < getLength(); ++i)
			list.append(getText(i, labelled));

		// all pending changes are moot
		discardUnsyncedChanges();

		// store what we want to keep of the edit data
		int num_listeners = data->num_listeners;
		ValueLabels *value_labels = data->value_labels;
		data->value_labels = nullptr; // prevent destruction
		FormattingOptions formatting_options = data->formatting_options;

		// destroy and re-allocate edit data
		data->num_listeners = 0; // to avoid the otherwise useful assert in discardEditData
		bool pending = isPending();
		discardEditData();
		if (pending) type |= Pending; // flag is cleared in discardEditData()
		setDataType(new_type);
		allocateEditData();

		// re-set persistent aspects of the edit data
		data->value_labels = value_labels;
		data->formatting_options = formatting_options;
		data->num_listeners = num_listeners;

		// re-set all data
		lockSyncing(true);
		for (int i = list.size() - 1; i >= 0; --i)
			setText(i, list[i]);

		if (sync) {
			QString command = u".rk.set.vector.mode("_s + getFullName() + u", "_s;
			if (new_type == RObject::DataCharacter) command += u"as.character"_s;
			else if (new_type == RObject::DataNumeric) command += u"as.numeric"_s;
			else if (new_type == RObject::DataLogical) command += u"as.logical"_s;
			else if (new_type == RObject::DataFactor) command += u"as.factor"_s;
			command += u')';
			RCommand *rcommand = new RCommand(command, RCommand::App | RCommand::Sync);
			rcommand->setUpdatesObject(this);
			RInterface::issueCommand(rcommand);
			if (new_type == RObject::DataFactor) updateValueLabels(); // as.factor resets the "levels"-attribute!

			syncDataToR();
		} else discardUnsyncedChanges();
		lockSyncing(false);
	} else {
		setDataType(new_type);
	}
}

void RKVariable::writeMetaData(RCommandChain *chain) {
	RK_TRACE(OBJECTS);

	writeValueLabels(chain);
	RObject::writeMetaData(chain);
}

////////////////////// BEGIN: data-handling //////////////////////////////
#define ALLOC_STEP 2
#define INITIAL_ALLOC 100

void RKVariable::setLength(int len) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(!getLength()); // should only be called once
	RK_ASSERT(!dimensions.isEmpty());

	dimensions[0] = len;
}

bool RKVariable::updateType(RData *new_data) {
	RK_TRACE(OBJECTS);

	if (data) {
		int old_type = type;
		bool ret = RObject::updateType(new_data);
		if (old_type == type) return ret;

		// Convert old values to the new data type.
		// TODO: This is quite inefficient, as we will update the data from R in a second, anyway.
		// Still it is a quick, dirty, and safe way to keep the data representation in a suitable format
		int new_type = type;
		type = old_type; // needed to read out the old data
		setVarType(typeToDataType(new_type), false);
		auto datatype = getDataType(); // HACK: setVarType might have changed the type. We still want to apply any other flags from R
		type = new_type;
		setDataType(datatype);
		return ret;
	}
	return RObject::updateType(new_data);
}

// virtual
void RKVariable::beginEdit() {
	RK_TRACE(OBJECTS);

	if (!data) {
		allocateEditData();
		if (!(isPending() || (parentObject() && parentObject()->isPending()))) updateDataFromR(nullptr);
	}
	++(data->num_listeners);
}

// virtual
void RKVariable::endEdit() {
	RK_TRACE(OBJECTS);

	RK_ASSERT(data);
	RK_ASSERT(data->num_listeners > 0);
	--(data->num_listeners);
	if (!data->num_listeners) discardEditData();
}

void RKVariable::allocateEditData() {
	RK_TRACE(OBJECTS);

	// edit data should only be allocated once, even if there are multiple editors
	RK_ASSERT(!data);

	data = new RKVarEditData;
	data->sync_locks = 0;
	data->value_labels = nullptr;
	data->formatting_options.alignment = FormattingOptions::AlignDefault;
	data->formatting_options.precision_mode = FormattingOptions::PrecisionDefault;
	data->formatting_options.precision = 0;
	data->previously_valid = true;
	data->num_listeners = 0;
	discardUnsyncedChanges(); // initialize

	// initialization hack
	int length = getLength();
	dimensions[0] = -1;
	extendToLength(length);
	RK_ASSERT(data->cell_states.size() >= getLength());

	for (int i = 0; i < getLength(); ++i) {
		data->cell_states[i] = RKVarEditData::NA;
	}
}

void RKVariable::discardEditData() {
	RK_TRACE(OBJECTS);

	RK_ASSERT(data);
	RK_ASSERT(!(data->num_listeners));
	RK_ASSERT(data->changes.from_index == -1);

	delete data->value_labels;
	delete data;
	data = nullptr;

	if (isPending()) (type -= Pending);
}

void RKVariable::updateDataFromR(RCommandChain *chain) {
	RK_TRACE(OBJECTS);
	if (!data) return;

	RCommand *c = new RCommand(u".rk.get.vector.data ("_s + getFullName() + u')', RCommand::App | RCommand::Sync | RCommand::GetStructuredData);
	whenCommandFinished(c, [this](RCommand *command) {
		if (!data) return; // this can happen, if the editor is closed while a data update is still queued.

		// prevent resyncing of data
		lockSyncing(true);

		RK_ASSERT(command->getDataType() == RData::StructureVector);
		RK_ASSERT(command->getDataLength() == 3);

		RData::RDataStorage top = command->structureVector();
		RData *cdata = top.at(0);
		RData *levels = top.at(1);
		RData *invalids = top.at(2);

		// set factor levels first
		RK_ASSERT(levels->getDataType() == RData::StringVector);
		QStringList new_levels = levels->stringVector();
		int levels_len = new_levels.size();
		RK_ASSERT(levels_len >= 1);
		delete data->value_labels;
		data->value_labels = new RObject::ValueLabels;
		if ((levels_len == 1) && new_levels.at(0).isEmpty()) {
			// no levels
		} else {
			for (int i = 0; i < levels_len; ++i) {
				data->value_labels->insert(QString::number(i + 1), new_levels.at(i));
			}
		}

		// now set the data
		RK_ASSERT(cdata->getDataLength() == (unsigned int)getLength()); // not really a problem due to the line below, I'd still like to know if / when this happens.
		extendToLength(cdata->getDataLength());
		if (cdata->getDataType() == RData::StringVector) {
			setCharacterFromR(0, getLength() - 1, cdata->stringVector());
		} else if (cdata->getDataType() == RData::RealVector) {
			setNumericFromR(0, getLength() - 1, cdata->realVector());
		} else if (cdata->getDataType() == RData::IntVector) {
			RData::IntStorage int_data = cdata->intVector();
			unsigned int len = getLength();
			QVector<double> dd;
			dd.reserve(len);
			for (unsigned int i = 0; i < len; ++i) {
				if (RInterface::isNaInt(int_data.at(i))) dd.append(NAN);
				else dd.append((double)int_data.at(i));
			}
			setNumericFromR(0, getLength() - 1, dd);
		}

		// now set the invalid fields (only if they are still NAs in the R data)
		data->invalid_fields.clear();
		if (invalids->getDataLength() <= 1) {
			// no invalids
		} else {
			RK_ASSERT(invalids->getDataType() == RData::StringVector);
			QStringList invalids_list = invalids->stringVector();
			int invalids_length = invalids_list.size();
			RK_ASSERT((invalids_length % 2) == 0);
			int invalids_count = invalids_length / 2;
			for (int i = 0; i < invalids_count; ++i) {
				int row = invalids_list.at(i).toInt() - 1;
				if (data->cell_states[row] & RKVarEditData::NA) { // NOTE: Do *not* use setText(), here. It tries too hard to set a valid value.
					data->invalid_fields.insert(row, invalids_list.at(invalids_count + i));
					data->cell_states[row] = RKVarEditData::Invalid;
				}
			}
		}
		data->previously_valid = data->invalid_fields.isEmpty();
		data->formatting_options = parseFormattingOptionsString(getMetaProperty(QStringLiteral("format")));

		ChangeSet *set = new ChangeSet(0, getLength(), true);
		RKModificationTracker::instance()->objectDataChanged(this, set);
		RKModificationTracker::instance()->objectMetaChanged(this);
		type -= (type & NeedDataUpdate);
		discardUnsyncedChanges();
		lockSyncing(false);
	});
	RInterface::issueCommand(c, chain);
}

void RKVariable::lockSyncing(bool lock) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	if (lock) data->sync_locks++;
	else data->sync_locks--;
	RK_ASSERT(data->sync_locks >= 0);

	if (!(data->sync_locks)) {
		syncDataToR();
		discardUnsyncedChanges();
	}
}

void RKVariable::discardUnsyncedChanges() {
	RK_TRACE(OBJECTS);

	RK_ASSERT(data);
	data->changes.from_index = data->changes.to_index = -1;
}

void RKVariable::syncDataToR() {
	RK_TRACE(OBJECTS);
	if (data->changes.from_index == -1) return;

	// TODO
	writeData(data->changes.from_index, data->changes.to_index);
	discardUnsyncedChanges();
}

void RKVariable::restore(RCommandChain *chain) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	writeData(0, getLength() - 1, chain);
	discardUnsyncedChanges();
	writeMetaData(chain);
}

void RKVariable::writeInvalidFields(QList<int> rows, RCommandChain *chain) {
	RK_TRACE(OBJECTS);

	if (rows.isEmpty()) return;

	QString set;
	QString values;
	QString clear;

	for (int i = 0; i < rows.size(); ++i) {
		int row = rows[i];

		if (data->invalid_fields.contains(row)) {
			if (!set.isEmpty()) {
				set.append(u", "_s);
				values.append(u", "_s);
			}
			set.append(QString::number(row + 1));
			values.append(rQuote(data->invalid_fields[row]));
		} else {
			if (!clear.isEmpty()) clear.append(u", "_s);
			clear.append(QString::number(row + 1));
		}

		data->cell_states[row] -= (data->cell_states[row] & RKVarEditData::UnsyncedInvalidState);
	}

	if (!set.isEmpty()) {
		set = u"set=c("_s + set + u"), "_s;
		values = u"values=c("_s + values + u')';
	}
	if (!clear.isEmpty()) {
		clear = u"clear=c("_s + clear + u')';
		if (!values.isEmpty()) values.append(u',');
	}

	RCommand *command = new RCommand(u".rk.set.invalid.fields ("_s + getFullName() + u", "_s + set + values + clear + u')', RCommand::App | RCommand::Sync);
	command->setUpdatesObject(this);
	RInterface::issueCommand(command, chain);

	if (data->previously_valid != data->invalid_fields.isEmpty()) {
		data->previously_valid = data->invalid_fields.isEmpty();
		RKModificationTracker::instance()->objectMetaChanged(this);
	}
}

void RKVariable::writeData(int from_row, int to_row, RCommandChain *chain) {
	RK_TRACE(OBJECTS);
	if (from_row == -1) return;

	QList<int> changed_invalids;

	// TODO: try to sync in correct storage mode
	if (from_row == to_row) {
		RCommand *command = new RCommand(getFullName() + u'[' + QString::number(from_row + 1) + u"] <- "_s + getRText(from_row), RCommand::App | RCommand::Sync);
		command->setUpdatesObject(this);
		RInterface::issueCommand(command, chain);
		if (data->cell_states[from_row] & RKVarEditData::UnsyncedInvalidState) changed_invalids.append(from_row);
	} else {
		QString data_string = u"c ("_s;
		for (int row = from_row; row <= to_row; ++row) {
			// TODO: use getCharacter and direct setting of vectors.
			data_string.append(getRText(row));
			if (row != to_row) {
				data_string.append(u", "_s);
			}
			if (data->cell_states[row] & RKVarEditData::UnsyncedInvalidState) changed_invalids.append(row);
		}
		data_string.append(u')');
		RCommand *command = new RCommand(getFullName() + u'[' + QString::number(from_row + 1) + u':' + QString::number(to_row + 1) + u"] <- "_s + data_string, RCommand::App | RCommand::Sync);
		command->setUpdatesObject(this);
		RInterface::issueCommand(command, chain);
	}

	if (!changed_invalids.isEmpty()) writeInvalidFields(changed_invalids, chain);

	ChangeSet *set = new ChangeSet(from_row, to_row);
	RKModificationTracker::instance()->objectDataChanged(this, set);
}

void RKVariable::cellsChanged(int from_row, int to_row) {
	RK_TRACE(OBJECTS);
	if (!data->sync_locks) {
		writeData(from_row, to_row);
	} else {
		if ((data->changes.from_index > from_row) || (data->changes.from_index == -1)) data->changes.from_index = from_row;
		if (data->changes.to_index < to_row) data->changes.to_index = to_row;
	}
}

void RKVariable::extendToLength(int length) {
	if (!data) return;
	RK_TRACE(OBJECTS);

	if (length <= 0) length = 0;
	int old_length = getLength();
	if (length <= old_length) return;

	// pad storage to required list with "unknown" data
	for (int i = old_length; i < length; ++i) {
		if (getDataType() == DataCharacter) data->cell_strings.append(QString());
		else data->cell_doubles.append(0.0);
		data->cell_states.append(RKVarEditData::Unknown);
	}

	dimensions[0] = length;
}

bool RKVariable::hasInvalidFields() const {
	if (!data) return false; // should not ever happen, though
	return (!data->invalid_fields.isEmpty());
}

QString RKVariable::getText(int row, bool pretty) const {
	if (row >= getLength()) {
		RK_ASSERT(false);
		return (QString());
	}

	if (data->cell_states[row] & RKVarEditData::Invalid) {
		RK_ASSERT(data->invalid_fields.contains(row));
		return (data->invalid_fields.value(row));
	}

	if (data->cell_states[row] & RKVarEditData::NA) return (QString());

	QString ret;
	if (getDataType() == DataCharacter) {
		RK_ASSERT(!data->cell_strings.isEmpty());
		ret = data->cell_strings[row];
	} else {
		RK_ASSERT(!data->cell_doubles.isEmpty());
		if (pretty && (data->formatting_options.precision_mode != FormattingOptions::PrecisionDefault) && (getDataType() != DataLogical)) {
			if (data->formatting_options.precision_mode == FormattingOptions::PrecisionRequired) {
				ret = QString::number(data->cell_doubles[row], 'g', MAX_PRECISION);
			} else {
				ret = QString::number(data->cell_doubles[row], 'f', data->formatting_options.precision);
			}
		} else {
			ret = QString::number(data->cell_doubles[row], 'g', MAX_PRECISION);
		}
	}

	if (pretty) {
		if (getDataType() == DataLogical) {
			if (ret == QLatin1String("0")) return QStringLiteral("FALSE");
			else if (ret == QLatin1String("1")) return QStringLiteral("TRUE");
		} else if (data->value_labels) {
			if (data->value_labels->contains(ret)) {
				return (*(data->value_labels))[ret];
			}
		}
	}
	return ret;
}

QString RKVariable::getRText(int row) const {
	RK_TRACE(OBJECTS);

	Status cell_state = cellStatus(row);

	if ((cell_state == ValueUnused) || (cell_state == ValueInvalid)) {
		return u"NA"_s;
	} else if (getDataType() == DataFactor) {
		return (rQuote(getText(row, true)));
	} else if (getDataType() == DataCharacter) {
		return (rQuote(getText(row)));
	} else if (getDataType() == DataLogical) {
		RK_ASSERT(!data->cell_doubles.isEmpty());
		if (data->cell_doubles[row] == 0) return u"FALSE"_s;
		else return u"TRUE"_s;
	} else {
		RK_ASSERT(!data->cell_doubles.isEmpty());
		return (QString::number(data->cell_doubles[row], 'g', MAX_PRECISION));
	}
}

void RKVariable::setText(int row, const QString &text) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(row < getLength());

	// clear previous state
	if (data->cell_states[row] & RKVarEditData::Invalid) {
		data->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
		data->invalid_fields.remove(row);
	} else {
		data->cell_states[row] = 0;
	}

	bool valid = true;
	if (text.isNull()) {
		data->cell_states[row] |= RKVarEditData::NA;
	} else if (text.isEmpty() && getDataType() != DataCharacter) {
		data->cell_states[row] |= RKVarEditData::NA;
	} else {
		if (getDataType() == DataCharacter) {
			data->cell_strings[row] = text;
		} else if (getDataType() == DataFactor) {
			if (data->value_labels) {
				QString realtext = data->value_labels->key(text); // first, attempt to set by level
				if (!realtext.isEmpty()) data->cell_doubles[row] = realtext.toInt();
				else { // if this failed, try to set by index, instead.
					if (data->value_labels->contains(text)) data->cell_doubles[row] = text.toInt();
					else valid = false;
				}
			} else valid = false;
		} else if (getDataType() == DataLogical) {
			if (text == QLatin1String("0") || text == QLatin1String("F") || text == QLatin1String("FALSE")) data->cell_doubles[row] = 0;
			else if (text == QLatin1String("1") || text == QLatin1String("T") || text == QLatin1String("TRUE")) data->cell_doubles[row] = 1;
			else valid = false;
		} else {
			data->cell_doubles[row] = text.toDouble(&valid);
		}
	}

	if (valid) {
		if (!(data->cell_states[row] & RKVarEditData::NA)) data->cell_states[row] |= RKVarEditData::Valid;
	} else {
		data->invalid_fields.insert(row, text);
		data->cell_states[row] |= RKVarEditData::Invalid | RKVarEditData::UnsyncedInvalidState;
	}

	cellsChanged(row, row);
}

void RKVariable::setNumericFromR(int from_row, int to_row, const QVector<double> &numdata) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(to_row < getLength());
	RK_ASSERT((to_row - from_row) < numdata.size());

	if (getDataType() == DataCharacter) {
		// NOTE: This code path is hit, for data this somehow numeric in R, but "unknown" in the frontend (fallback in setVarType())
		int i = 0;
		for (int row = from_row; row <= to_row; ++row) {
			setText(row, QString::number(numdata[i++], 'g', MAX_PRECISION));
		}
	} else if (getDataType() == DataFactor) {
		int i = 0;
		for (int row = from_row; row <= to_row; ++row) {
			if (data->cell_states[row] & RKVarEditData::Invalid) data->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
			else data->cell_states[row] = 0;

			if (std::isnan(numdata[i]) || (!data->value_labels) || (!data->value_labels->contains(QString::number(numdata[i])))) {
				data->cell_states[row] |= RKVarEditData::NA;
			} else {
				data->cell_states[row] |= RKVarEditData::Valid;
				data->cell_doubles[row] = numdata[i];
			}
			++i;
		}
	} else {
		int i = 0;
		for (int row = from_row; row <= to_row; ++row) {
			if (data->cell_states[row] & RKVarEditData::Invalid) data->cell_states[row] = RKVarEditData::UnsyncedInvalidState;
			else data->cell_states[row] = 0;

			if (std::isnan(numdata[i])) {
				data->cell_states[row] |= RKVarEditData::NA;
			} else {
				data->cell_states[row] |= RKVarEditData::Valid;
				data->cell_doubles[row] = numdata[i];
			}
			++i;
		}
	}
	cellsChanged(from_row, to_row);
}

QString *RKVariable::getCharacter(int from_row, int to_row) const {
	RK_TRACE(OBJECTS);
	if (to_row >= getLength()) {
		RK_ASSERT(false);
		return nullptr;
	}
	RK_ASSERT(from_row <= to_row);

	QString *ret = new QString[(to_row - from_row) + 1];

	int i = 0;
	for (int row = from_row; row <= to_row; ++row) {
		ret[i] = getText(row);
		i++;
	}

	return ret;
}

void RKVariable::setCharacterFromR(int from_row, int to_row, const QStringList &txtdata) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(to_row < getLength());
	RK_ASSERT((to_row - from_row) < txtdata.size());

	lockSyncing(true);
	int i = 0;
	for (int row = from_row; row <= to_row; ++row) {
		setText(row, txtdata[i++]);
	}
	lockSyncing(false);
}

RKVariable::Status RKVariable::cellStatus(int row) const {
	if (row >= getLength()) return ValueUnknown;
	if (data->cell_states[row] == RKVarEditData::Unknown) return ValueUnknown;
	if (data->cell_states[row] & RKVarEditData::NA) return ValueUnused;
	if (data->cell_states[row] & RKVarEditData::Invalid) return ValueInvalid;
	return ValueValid;
}

void RKVariable::removeRows(int from_row, int to_row) {
	RK_TRACE(OBJECTS);

	QList<int> changed_invalids;
	int offset = (to_row - from_row) + 1;

	for (int row = from_row; row < getLength(); ++row) {
		if (data->invalid_fields.contains(row)) {
			QString inv = data->invalid_fields.take(row);
			changed_invalids.append(row);
			if (row > to_row) {
				changed_invalids.append(row - offset);
				data->invalid_fields.insert(row - offset, inv);
			}
		}
	}

	for (int row = to_row; row >= from_row; --row) {
		data->cell_states.removeAt(row);
		if (getDataType() == DataCharacter) data->cell_strings.removeAt(row);
		else data->cell_doubles.removeAt(row);
	}

	if (!changed_invalids.isEmpty()) writeInvalidFields(changed_invalids);

	dimensions[0] -= offset;
}

void RKVariable::insertRows(int row, int count) {
	RK_TRACE(OBJECTS);

	for (int i = row; i < row + count; ++i) {
		data->cell_states.insert(i, RKVarEditData::NA);
		if (getDataType() == DataCharacter) data->cell_strings.insert(i, QString());
		else data->cell_doubles.insert(i, 0.0);
	}

	QList<int> changed_invalids;
	for (int i = getLength() - 1; i >= row; --i) {
		if (data->invalid_fields.contains(i)) {
			QString dummy = data->invalid_fields.take(i);
			changed_invalids.append(i);
			changed_invalids.append(i + count);
			data->invalid_fields.insert(i + count, dummy);
		}
	}

	if (!changed_invalids.isEmpty()) writeInvalidFields(changed_invalids);

	dimensions[0] += count;
}

RObject::ValueLabels RKVariable::getValueLabels() const {
	RK_ASSERT(data);

	if (!data->value_labels) return RObject::ValueLabels();
	return (*(data->value_labels));
}

void RKVariable::setValueLabels(const ValueLabels &labels) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	if (labels.isEmpty()) {
		if (!data->value_labels) return; // no change: was empty, is empty

		delete data->value_labels;
		data->value_labels = nullptr;
	} else {
		if (!(data->value_labels)) data->value_labels = new RObject::ValueLabels;
		else {
			if (*(data->value_labels) == labels) return; // old and new lists are the same
		}
		*(data->value_labels) = labels;
	}

	updateValueLabels();
}

void RKVariable::updateValueLabels() {
	RK_TRACE(OBJECTS);

	writeValueLabels(nullptr);
	RKModificationTracker::instance()->objectMetaChanged(this);

	ValueLabels *labels = data->value_labels;

	lockSyncing(true);
	// find out which values got valid / invalid and change those
	for (int i = 0; i < getLength(); ++i) {
		if (cellStatus(i) == ValueInvalid) {
			if (labels && labels->contains(getText(i))) {
				setText(i, getText(i));
			}
		} else {
			if (!(labels && labels->contains(getText(i)))) {
				setText(i, getText(i));
			}
		}
	}
	lockSyncing(false);

	// also update display of all values:
	ChangeSet *set = new ChangeSet(0, getLength() - 1);
	RKModificationTracker::instance()->objectDataChanged(this, set);

	// TODO: find out whether the object is valid after the operation and update accordingly!
}

void RKVariable::writeValueLabels(RCommandChain *chain) const {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	QString level_string;
	if (data->value_labels && (!data->value_labels->isEmpty())) {
		int i = 1;
		level_string = u"c ("_s;
		while (data->value_labels->contains(QString::number(i))) {
			level_string.append(rQuote((*(data->value_labels))[QString::number(i)]));
			if (data->value_labels->contains(QString::number(++i))) {
				level_string.append(u", "_s);
			}
		}
		level_string.append(u')');
	} else {
		level_string = u"NULL"_s;
	}

	RCommand *command = new RCommand(u".rk.set.levels("_s + getFullName() + u", "_s + level_string + u')', RCommand::App | RCommand::Sync);
	command->setUpdatesObject(this);
	RInterface::issueCommand(command, chain);
}

QString RKVariable::getValueLabelString() const {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	if (data->value_labels) {
		int i = 1;
		QString level_string;
		while (data->value_labels->contains(QString::number(i))) {
			level_string.append((*(data->value_labels))[QString::number(i)]);
			if (data->value_labels->contains(QString::number(++i))) {
				level_string.append(u"#,#"_s);
			}
		}

		return level_string;
	} else {
		return QString();
	}
}

void RKVariable::setValueLabelString(const QString &string) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	ValueLabels new_labels;
	QStringList list = string.split(u"#,#"_s);

	int i = 1;
	for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it) {
		new_labels.insert(QString::number(i), *it);
		++i;
	}
	setValueLabels(new_labels);
}

RKVariable::FormattingOptions RKVariable::getFormattingOptions() const {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	return data->formatting_options;
}

void RKVariable::setFormattingOptions(const FormattingOptions new_options) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	if ((new_options.alignment == data->formatting_options.alignment) && (new_options.precision_mode == data->formatting_options.precision_mode) && (new_options.precision == data->formatting_options.precision)) return;

	data->formatting_options = new_options;
	setMetaProperty(QStringLiteral("format"), formattingOptionsToString(new_options));

	// also update display of all values:
	ChangeSet *set = new ChangeSet(0, getLength() - 1);
	RKModificationTracker::instance()->objectDataChanged(this, set);
}

QString RKVariable::getFormattingOptionsString() const {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	return getMetaProperty(QStringLiteral("format"));
}

void RKVariable::setFormattingOptionsString(const QString &string) {
	RK_TRACE(OBJECTS);
	RK_ASSERT(data);

	setFormattingOptions(parseFormattingOptionsString(string));
}

// static
QString RKVariable::formattingOptionsToString(const FormattingOptions &options) {
	RK_TRACE(OBJECTS);

	QString format_string;
	if (options.alignment != (int)FormattingOptions::AlignDefault) {
		format_string.append(u"align:"_s + QString::number(options.alignment));
	}

	if (options.precision_mode != (int)FormattingOptions::PrecisionDefault) {
		if (!format_string.isEmpty()) format_string.append(u'#');
		format_string.append(u"prec:"_s);
		if (options.precision_mode == (int)FormattingOptions::PrecisionRequired) {
			format_string.append(u'v');
		} else {
			format_string.append(QString::number(options.precision));
		}
	}

	return format_string;
}

// static
RKVariable::FormattingOptions RKVariable::parseFormattingOptionsString(const QString &string) {
	RK_TRACE(OBJECTS);

	FormattingOptions formatting_options;
	formatting_options.alignment = FormattingOptions::AlignDefault;
	formatting_options.precision_mode = FormattingOptions::PrecisionDefault;
	formatting_options.precision = 0;

	QStringList list = string.split(u'#', Qt::SkipEmptyParts);
	QString option, parameter;
	for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it) {
		option = (*it).section(u':', 0, 0);
		parameter = (*it).section(u':', 1, 1);

		if (parameter.isEmpty()) continue;

		if (option == "align"_L1) {
			int al = parameter.toInt();
			if ((al >= (int)FormattingOptions::AlignDefault) && (al <= (int)FormattingOptions::AlignRight)) {
				formatting_options.alignment = (FormattingOptions::Alignment)al;
			}
		} else if (option == "prec"_L1) {
			if (parameter == "d"_L1) {
				formatting_options.precision_mode = FormattingOptions::PrecisionDefault;
			} else if (parameter == "v"_L1) {
				formatting_options.precision_mode = FormattingOptions::PrecisionRequired;
			} else {
				int digits = parameter.toInt();
				if ((digits >= 0) && (digits <= 15)) {
					formatting_options.precision_mode = FormattingOptions::PrecisionFixed;
					formatting_options.precision = digits;
				}
			}
		} else {
			RK_ASSERT(false);
		}
	}

	return formatting_options;
}

RKVariable::CellAlign RKVariable::getAlignment() const {
	RK_ASSERT(data);

	if (data->formatting_options.alignment != FormattingOptions::AlignDefault) {
		if (data->formatting_options.alignment == FormattingOptions::AlignLeft) return AlignCellLeft;
		return AlignCellRight;
	} else {
		// TODO: use global (configurable) defaults, if not specified
		if ((getDataType() == DataCharacter) || (getDataType() == DataFactor)) {
			return AlignCellLeft;
		} else {
			return AlignCellRight;
		}
	}
}

/////////////////// END: data-handling ///////////////////////////
