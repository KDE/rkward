# Copyright 2011-2014 Meik Michalke <meik.michalke@hhu.de>
#
# This file is part of the R package XiMpLe.
#
# XiMpLe is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# XiMpLe is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with XiMpLe.  If not, see <http://www.gnu.org/licenses/>.


# package description files
# this internal object can be used by the package roxyPackage to
# automatically create/update DESCRIPTION and CITATION files
pckg.dscrptn <- data.frame(
    Package="XiMpLe",
    Type="Package",
    Title="A simple XML tree parser and generator",
    Author="m.eik michalke",
    AuthorsR="c(person(given=\"Meik\", family=\"Michalke\", email=\"meik.michalke@hhu.de\",
      role=c(\"aut\", \"cre\")))",
    Maintainer="m.eik michalke <meik.michalke@hhu.de>",
    Depends="R (>= 2.9.0),methods",
    Suggests="testthat",
    Enhances="rkward",
    Description="This package provides a simple XML tree parser/generator. It includes functions to read XML files into R objects,
            get information out of and into nodes, and write R objects back to XML code.
            It's not as powerful as the XML package and doesn't aim to be, but for simple XML handling
            it could be useful. It was originally programmed for the R GUI and IDE RKWard, to make plugin development easier.
            You can install RKWard from http://rkward.kde.org.",
    License="GPL (>= 3)",
    Encoding="UTF-8",
    LazyLoad="yes",
    URL="http://reaktanz.de/?c=hacking&s=XiMpLe",
    stringsAsFactors=FALSE)
