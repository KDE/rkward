# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
kdoctools_create_handbook (index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR rkward)
kdoctools_create_manpage (man-rkward.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})
