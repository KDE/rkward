#' show method for S4 objects of class RKTestResult
#'
#' Prints a summary of plugin test results.
#'
#' @title show method for objects of class RKTestResult
#' @method show RKTestResult
#' @param object An object of class RKTestResult
#' @aliases show,RKTestResult-method
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}
#' @keywords methods
#' @examples
#' \dontrun{
#' rktest.makeplugintests("rkward_application_tests.R")
#' }
#' @exportMethod show
#' @rdname show

setMethod ("show", "RKTestResult", function (object) {
	stopifnot (inherits (object, "RKTestResult"))

	cat (format ("ID", width=30))
	cat (format ("code match", width=15))
	cat (format ("output match", width=15))
	cat (format ("message match", width=15))
	cat (format ("error", width=15))
	cat (format ("result", width=15))
	cat ("\n", rep ("-", 96), "\n", sep="")

	for (i in 1:length (object@id)) {
		cat (format (object@id[i], width=30))
		cat (format (object@code_match[i], width=15))
		cat (format (object@output_match[i], width=15))
		cat (format (object@message_match[i], width=15))
		cat (format (object@error[i], width=15))
		cat (format (if (is.na (object@passed[i])) "--skipped--" else if (object@passed[i]) "pass" else "FAIL", width=15))
		cat ("\n")
	}

	cat (rep ("-", 96), "\n", sep="")
	cat (as.character (sum (object@passed, na.rm=TRUE)), " / ", as.character (sum (!is.na (object@passed))), " tests passed\n")
	if (sum (is.na (object@passed)) > 0) cat ("(", as.character (sum (is.na (object@passed))), " / ", as.character (length (object@passed)), " tests skipped due to missing libraries)", sep="");
})
