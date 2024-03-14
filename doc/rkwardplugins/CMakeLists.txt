# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
# NOTE: kdoctools_create_handbook does not support multiple docbooks inside one directory. That's why this is separate.
# KF5 TODO: Is the above still true?
kdoctools_create_handbook (index.docbook INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en SUBDIR rkwardplugins)

ADD_CUSTOM_TARGET (webdocs
	COMMAND meinproc5 --stylesheet customization/kde-web.xsl ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
