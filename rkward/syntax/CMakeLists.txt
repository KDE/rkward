# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
# rkward.xml is also installed as a qresource via the parent CMakeLists.txt, but will not be found, there, before KF5 5.86.0
IF(${KF5_VERSION} VERSION_LESS "5.86.0")
    INSTALL(FILES rkward.xml DESTINATION ${DATA_INSTALL_DIR}/org.kde.syntax-highlighting/syntax/)
ENDIF()

