# This file is part of KDSingleApplication.
#
# SPDX-FileCopyrightText: 2020-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# NOTE: severly stripped down from upstream version
set(KDSINGLEAPPLICATION_SRCS kdsingleapplication.cpp kdsingleapplication_localsocket.cpp)

add_library(kdsingleapplication STATIC ${KDSINGLEAPPLICATION_INSTALLABLE_INCLUDES} ${KDSINGLEAPPLICATION_SRCS})
target_compile_definitions(kdsingleapplication PUBLIC KDSINGLEAPPLICATION_STATIC_BUILD)
add_library(
    KDAB::kdsingleapplication ALIAS kdsingleapplication
)

if(WIN32)
    target_link_libraries(kdsingleapplication PRIVATE kernel32)
endif()
target_link_libraries(
    kdsingleapplication
    PUBLIC Qt::Core
    PRIVATE Qt::Network
)
