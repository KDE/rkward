/*
rkspecialactions - This file is part of the RKWard project. Created: Mon Mar 15 2010
SPDX-FileCopyrightText: 2010-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSPECIALACTIONS_H
#define RKSPECIALACTIONS_H

#include <QAction>

/** This namespace provides functions to generate some standard actions, i.e. actions which are needed at more than one place.

@author Thomas Friedrichsmeier */
class RKPasteSpecialAction : public QAction {
	Q_OBJECT
public:
	explicit RKPasteSpecialAction (QObject* parent);
	~RKPasteSpecialAction ();
public Q_SLOTS:
/** The actual workhorse of the action. */
	void doSpecialPaste ();
Q_SIGNALS:
/** Connect to this signal to receive the resulting text to be pasted */
	void pasteText (const QString&);
};


#include <QDialog>

class QButtonGroup;
class QLineEdit;
class QCheckBox;
class RKSaveObjectChooser;

/** Dialog used in RKPasteSpecialAction
    TODO: move to separate file, now that it can be used standalone */
class RKPasteSpecialDialog : public QDialog {
	Q_OBJECT
public:
	explicit RKPasteSpecialDialog(QWidget* parent, bool standalone=false);
	~RKPasteSpecialDialog ();

	enum Dimensionality {
		DimSingleString,
		DimVector,
		DimMatrix,
		DimDataFrame
	};
	enum Separator {
		SepTab,
		SepComma,
		SepSpace,
		SepWhitespace,
		SepCustom
	};
	enum Quoting {
		QuoteNone,
		QuoteAuto,
		QuoteAll
	};
	
	QString resultingText ();
	void accept() override;
public Q_SLOTS:
	void updateState ();
private:
	QString prepString (const QString& src, const Quoting quot) const;

	QButtonGroup* dimensionality_group;
	QButtonGroup* separator_group;
	QLineEdit* separator_freefield;
	QButtonGroup* quoting_group;
	QCheckBox* transpose_box;
	QCheckBox* reverse_h_box;
	QCheckBox* reverse_v_box;
	QCheckBox* insert_nas_box;
	QCheckBox* names_box;
	QCheckBox* rownames_box;
	RKSaveObjectChooser *objectname;
	QPushButton *ok_button;
};

#endif
