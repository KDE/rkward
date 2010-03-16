/***************************************************************************
                          rkspecialactions  -  description
                             -------------------
    begin                : Mon Mar 15 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RKSPECIALACTIONS_H
#define RKSPECIALACTIONS_H

#include <kpastetextaction.h>

/** This namespace provides functions to generate some standard actions, i.e. actions which are needed at more than one place.

@author Thomas Friedrichsmeier */
class RKPasteSpecialAction : public KPasteTextAction {
	Q_OBJECT
public:
	RKPasteSpecialAction (QObject* parent);
	~RKPasteSpecialAction ();
public slots:
/** The actual workhorse of the action. */
	void doSpecialPaste ();
signals:
/** Connect to this signal to receive the resulting text to be pasted */
	void pasteText (const QString&);
};


#include <kdialog.h>

class QButtonGroup;
class QLineEdit;
class QCheckBox;

/** Dialog used in RKPasteSpecialAction */
class RKPasteSpecialDialog : public KDialog {
	Q_OBJECT
public:
	RKPasteSpecialDialog (QWidget* parent);
	~RKPasteSpecialDialog ();

	enum Dimensionality {
		DimSingleString,
		DimVector,
		DimMatrix
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
public slots:
	void updateState ();
private:
	QString prepString (const QString& src) const;

	QButtonGroup* dimensionality_group;
	QButtonGroup* separator_group;
	QLineEdit* separator_freefield;
	QButtonGroup* quoting_group;
	QCheckBox* transpose_box;
	QCheckBox* reverse_h_box;
	QCheckBox* reverse_v_box;
	QCheckBox* insert_nas_box;
};

#endif
