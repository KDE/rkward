# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' S4 class RKTest
#' 
#' @description
#' This class is used internally by \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}.
#'
#' @slot id A unique character string
#' @slot call A function to be called
#' @slot fuzzy_output Allow fuzzy results
#' @slot expect_error Expect errors
#' @slot libraries A charcter vector naming needed libraries
#' @slot files A character vector naming needed files, path relative to the test standards directory
#' @slot record.all.commands Should synchronization commands and commands to generate run-again-links be included in the command recording? Generally, this should be FALSE (the default).
#' @slot ignore May include one of more of "output", "messages", "commands", for skipping comparison of these against the standard, completely.
#' @name RKTest
#' @import methods
#' @keywords classes
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}
#' @exportClass RKTest
# @rdname RKTest-class

setClass ("RKTest",
		representation (id="character", call="function", fuzzy_output="logical", expect_error="logical", libraries="character", files="character", record.all.commands="logical", ignore="character"),
		prototype(character(0), id=NULL, call=function () { stop () }, fuzzy_output=FALSE, expect_error=FALSE, libraries=character(0), files=character(0), record.all.commands=FALSE, ignore=character(0)),
		validity=function (object) {
			if (is.null (object@id)) return (FALSE)
			return (TRUE)
		}
	)
