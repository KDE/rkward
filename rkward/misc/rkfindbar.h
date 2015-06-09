/***************************************************************************
                          rkfindbar  -  description
                             -------------------
    begin                : Tue Feb 24 2015
    copyright            : (C) 2015 by Thomas Friedrichsmeier
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

#ifndef RKFINDBAR_H
#define RKFINDBAR_H

#include <QWidget>
#include <QMap>

class QCheckBox;
class KHistoryComboBox;
class QHBoxLayout;

class RKFindBar : public QWidget {
	Q_OBJECT
public:
	explicit RKFindBar (QWidget *parent, bool custom=false);
	~RKFindBar ();

	enum FindOptions {
		HighlightAll,
		FindAsYouType,
		MatchCase
	};

/** Insert the given option-widgets into the search bar. Widgets must be owned by the find bar. 

I'd love to be able to implement options as QWidgetAction, instead of QWidgets.
However, these can't be inserted into anything other than QToolBar or QMenu... */
	void setPrimaryOptions (const QList<QWidget*>& options);

	QCheckBox* getOption (const FindOptions option);
	bool isOptionSet (const FindOptions option) const;
public slots:
	void activate ();
	void activateWithFindAsYouType ();
	void forward ();
	void backward ();
signals:
	void findRequest (const QString& text, bool backwards, const RKFindBar *findbar, bool *result);
private slots:
/** search term _or_ search options changed. Triggers a forward search, if FindAsYouType is active */
	void searchChanged ();
private:
	QMap<FindOptions, QCheckBox*> default_actions;
	KHistoryComboBox* term_edit;
	QHBoxLayout *mlayout;
	void doSearch (bool backward);
	QPalette regular_palette;
	QPalette nomatch_palette;
};

#endif
