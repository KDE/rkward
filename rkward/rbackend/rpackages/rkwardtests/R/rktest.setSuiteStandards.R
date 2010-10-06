#' Set RKWard plugin test suite standards
#'
#' Use this function after you plugin passed all tests to set the resulting code,
#' output and R messages as the standard that will be compared to during following tests.
#' 
#' @title Set RKWard suite standards
#' @usage rktest.setSuiteStandards(suite, basedir=getwd())
#' @aliases rktest.setSuiteStandards
#' @param suite Character string naming the test suite to set standards for.
#' @param basedir Defaults to the working directory.
#' @return The function simply changes the names of the previously created files,
#' specifically adding the prefix "RKTestStandard.".
#' @docType function
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}
#' @export
#' @examples
#' \dontrun{
#' rktest.setSuiteStandards()
#' }

rktest.setSuiteStandards <- function (suite, basedir=getwd ()) {
	if (!inherits (suite, "RKTestSuite")) return (result)
	if (!validObject (suite)) return (result)

	ok <- readline ("You are about to set new standards for this suite. This means you are certain that ALL tests in this suite have produced the expected/correct result on the last run. If you are absolutely sure, enter \"I am sure\" to proceed.");
	if (ok != "I am sure") stop ("Aborted")

	oldwd = getwd ()
	on.exit (setwd (oldwd))
	setwd (paste (basedir, suite@id, sep="/"))

	files <- list.files ()
	files <- grep ("\\.(messages.txt|rkcommands.R|rkout)$", files, value=TRUE)
	files <- grep ("^RKTestStandard", files, value=TRUE, invert=TRUE)
	file.copy (files, paste ("RKTestStandard.", files, sep=""), overwrite=TRUE)

	# clean anything that is *not* a standard file
	rktest.cleanRKTestSuite (suite, basedir)
}
