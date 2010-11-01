#' This class is used internally by \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}.
#'
#' @title S4 class RKTest
#' @slot id A unique character string
#' @slot call A function to be called
#' @slot fuzzy_output Allow fuzzy results
#' @slot expect_error Expect errors
#' @slot libraries A charcter vector naming needed libraries
#' @slot files A character vector naming needed files, path relative to the test standards directory
#' @name RKTest
#' @import methods
#' @keywords classes
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}
#' @exportClass RKTest
#' @rdname RKTest-class

setClass ("RKTest",
		representation (id="character", call="function", fuzzy_output="logical", expect_error="logical", libraries="character", files="character"),
		prototype(character(0), id=NULL, call=function () { stop () }, fuzzy_output=FALSE, expect_error=FALSE, libraries=character(0), files=character(0)),
		validity=function (object) {
			if (is.null (object@id)) return (FALSE)
			return (TRUE)
		}
	)
