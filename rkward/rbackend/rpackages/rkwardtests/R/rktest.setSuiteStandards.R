#' Set RKWard plugin test suite standards
#'
#' Use this function after you plugin passed all tests to set the resulting code,
#' output and R messages as the standard that will be compared to during following tests.
#' 
#' @title Set RKWard suite standards
#' @usage rktest.setSuiteStandards(suite, basedir=getwd())
#' @aliases rktest.setSuiteStandards
#' @param suite Character string naming the test suite to set standards for.
#' @param basedir Should point to the testroot, defaults to the working directory.
#' @return The function simply changes the names of the previously created files,
#' specifically adding the prefix "RKTestStandard.".
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}
#' @export
#' @examples
#' \dontrun{
#' rktest.setSuiteStandards("rkward_application_tests.R")
#' }

rktest.setSuiteStandards <- function (suite, basedir=getwd ()) {
	if (!inherits (suite, "RKTestSuite")) return (result)
	if (!validObject (suite)) return (result)

	ok <- readline ("You are about to set new standards for this suite. This means you are certain that ALL tests in this suite have produced the expected/correct result on the last run. If you are absolutely sure, enter \"I am sure\" to proceed.");
	if (ok != "I am sure") stop ("Aborted")

	oldwd = getwd ()
	on.exit (setwd (oldwd))
	setwd (file.path(basedir, suite@id))

	temp.suite.dir <- rktest.createTempSuiteDir(suite@id)
	files <- list.files (temp.suite.dir)
	files <- grep ("\\.(messages.txt|rkcommands.R|rkout)$", files, value=TRUE)
	file.copy (file.path(temp.suite.dir, files), paste ("RKTestStandard.", files, sep=""), overwrite=TRUE)

	# clean anything that is *not* a standard file
	rktest.cleanRKTestSuite (suite, basedir)
}
