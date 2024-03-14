# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
ADD_SUBDIRECTORY(app-icon)

INSTALL(FILES
	rkward_logo.png
	function.png
	list.png
	matrix.png
	menu.svg
	data-numeric.png
	data-factor.png
	data-logical.png
	window_back.png
	window_forward.png
	run_line.png
	run_selection.png
	run_all.png
	s4_slots.png
	paste_inside_selection.png
	paste_inside_table.png
    DESTINATION ${KDE_INSTALL_DATADIR}/rkward/icons)
