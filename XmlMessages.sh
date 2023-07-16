#!/usr/bin/python3
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: 2023 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

function get_files
{
    echo rkward/vnd.rkward.r.xml
}

function po_for_file
{
    case "$1" in
       rkward/vnd.rkward.r.xml)
           echo rkward_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       rkward/vnd.rkward.r.xml)
           echo comment
       ;;
    esac
}
