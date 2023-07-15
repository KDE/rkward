# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' S4 Class RKTestSuite
#'
#' @description
#' This class is used to create test suite objects that can be fed to \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}.
#'
#' @slot id A unique character string to identify a test suite
#' @slot libraries A charcter vector naming libraries that the test suite depends on.
#' @slot initCalls A list of functions to be run before any tests, e.g. to load libraries or data objects.
#' @slot tests A list of the actual plugin tests.
#' @slot postCalls  A list of functions to be run after all tests, e.g. to clean up.
#' @name RKTestSuite
#' @import methods
#' @keywords classes
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}
#' @exportClass RKTestSuite
# @rdname RKTestSuite-class

setClass ("RKTestSuite",
		representation (id="character", libraries="character", initCalls="list", tests="list", postCalls="list"),
		prototype(character(0), id=NULL, libraries=character(0), initCalls=list(), tests=list(), postCalls=list ()),
		validity=function (object) {
			if (length (object@id) != 1) return (FALSE)
			if (length (object@tests) < 1) return (FALSE)
			return (TRUE)
		}
	)
