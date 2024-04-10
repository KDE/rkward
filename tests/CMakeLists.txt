# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
ADD_CUSTOM_TARGET (plugintests
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../bin/rkward --evaluate "source\\(\\\"all_tests.R\\\"\\)"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
