#' Class RKTestSuite
#'
#' This class is used to create test suite objects that can be fed to \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}.
#'
#' @title S4 class RKTestSuite
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
#' @rdname RKTestSuite-class

setClass ("RKTestSuite",
		representation (id="character", libraries="character", initCalls="list", tests="list", postCalls="list"),
		prototype(character(0), id=NULL, libraries=character(0), initCalls=list(), tests=list(), postCalls=list ()),
		validity=function (object) {
			if (length (object@id) != 1) return (FALSE)
			if (length (object@tests) < 1) return (FALSE)
			return (TRUE)
		}
	)
