# Copyright 2010-2014 Meik Michalke <meik.michalke@hhu.de>
#
# This file is part of the R package rkwarddev.
#
# rkwarddev is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rkwarddev is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rkwarddev.  If not, see <http://www.gnu.org/licenses/>.


# package description files
# this internal object can be used by the package roxyPackage to
# automatically create/update DESCRIPTION and CITATION files
pckg.dscrptn <- data.frame(
    Package="rkwarddev",
    Type="Package",
    Title="A collection of tools for RKWard plugin development",
    Author="m.eik michalke <meik.michalke@hhu.de>",
    AuthorsR="c(person(given=\"Meik\", family=\"Michalke\", email=\"meik.michalke@hhu.de\",
      role=c(\"aut\", \"cre\")))",
    Maintainer="m.eik michalke <meik.michalke@hhu.de>",
    Depends="R (>= 2.9.0),methods,XiMpLe (>= 0.03-21),rkward (>= 0.5.7)",
    Enhances="rkward",
    Description="Provides functions to create plugin skeletons and XML structures for RKWard.",
    License="GPL (>= 3)",
    Encoding="UTF-8",
    LazyLoad="yes",
    URL="http://rkward.sourceforge.net",
    stringsAsFactors=FALSE)
