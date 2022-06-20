/*
rktexthints - This file is part of the RKWard project. Created: Sat Jun 17 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKTEXTHINTS_H
#define RKTEXTHINTS_H

#include <ktexteditor/view.h>
#include <ktexteditor/texthintinterface.h>

class RKCodeCompletionSettings;

/** Provides text hints for a KTextEditor::View */
class RKTextHints : public QObject, public KTextEditor::TextHintProvider {
	Q_OBJECT
public:
	RKTextHints(KTextEditor::View *view, const RKCodeCompletionSettings *settings);
	QString textHint(KTextEditor::View *view, const KTextEditor::Cursor &position) override;
private:
	~RKTextHints();
	const RKCodeCompletionSettings *settings;
};

#endif
