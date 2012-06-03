## temporarily turned off most of the roxygen comments
## class docs will remain static until roxygen2 supports "@slot"

#' S4 Class RKTestResult
#'
#' This class is used internally by \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}.
#'
#' @noRd
# @slot id A unique character string naming a test.
# @slot code_match A character string indicating whether the run code matched the standard.
# @slot output_match A character string indicating whether the resulting output matched the standard.
# @slot message_match A character string indicating whether the resulting R messages matched the standard.
# @slot error A character string indicating errors.
# @slot missing_libs A character string indicating missing libraries.
# @slot passed Logical: Did the test pass?
#' @name RKTestResult
#' @import methods
#' @keywords classes
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}
#' @exportClass RKTestResult
# @rdname RKTestResult-class

setClass ("RKTestResult",
		representation (id = "character", code_match = "character", output_match = "character", message_match = "character", error="character", missing_libs="character", passed="logical"),
		prototype(character(0), id = character (0), code_match = character (0), output_match = character (0), message_match = character (0), error = character (0), missing_libs = character (0), passed=FALSE),
		validity=function (object) {
			return (all.equal (length (object@id), length (object@code_match), length (object@output_match), length (object@message_match), length (object@error), length (object@missing_libs), length (object@passed)))
		}
	)
