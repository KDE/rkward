/*
rkfindbar - This file is part of the RKWard project. Created: Tue Feb 24 2015
SPDX-FileCopyrightText: 2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	void indicateSearchFail ();
public Q_SLOTS:
	void activate ();
	void activateWithFindAsYouType ();
	void forward ();
	void backward ();
Q_SIGNALS:
	void findRequest (const QString& text, bool backwards, const RKFindBar *findbar, bool *result);
private Q_SLOTS:
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
