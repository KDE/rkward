#' Run a whole RKWard plugin test suite
#'
#' The function \code{rktest.makeplugintests} will run a whole test suite that was prepared to check one or several RKWard plugins.
#' 
#' @title Run RKWard plugin test suite
#' @usage rktest.makeplugintests(testsuites, testroot,
#' outfile="make_plugintests.txt", append=FALSE, test.id=NULL)
#' @aliases rktest.makeplugintests
#' @param testsuites A character string or vector naming the test suites to be run.
#' @param testroot A character string pointing to the root directory where the test suite resides (including its folder with test standards).
#' @param outfile A character string giving a file name for the result log.
#' @param append If TRUE, append output to an existing file.
#' @param test.id An optional character string or vector naming one or more tests of a suite to be run (if NULL, all tests are run).
#' @return Results are printed to stdout and saved to the defined output file.
#' @docType function
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}, Meik Michalke \email{meik.michalke@@uni-duesseldorf.de}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:RKTestResult]{RKTestResult-class}}
#' @export
#' @examples
#' \dontrun{
#' rktest.makeplugintests(testsuites=c("rkward_application_tests.R",
#'    "import_export_plugins.R"), testroot=getwd())
#' rktest.makeplugintests(testsuites="distribution.R",
#'   testroot=getwd(), test.id=c("poisson_quantiles", "geom_quantiles"))
#' }

rktest.makeplugintests <- function(testsuites, testroot, outfile="make_plugintests.txt", append=FALSE, test.id=NULL){
  ## change to test root directory
  oldwd <- getwd()
  on.exit(setwd(oldwd))
  setwd(testroot)

  ## initialize
  rktest.initializeEnvironment()

  sink (file = outfile, append=append, type="output", split=TRUE)
  cat ("RKWard Version:\n")
  print (.rk.app.version)
  cat ("\n\nR-Version:\n")
  print (R.version)
  cat ("\n\nInstalled packages:\n")
  print (subset(installed.packages(),select=c(LibPath,Version)))

  allresults <- new ("RKTestResult")
  for (testsuite in testsuites) {
	  source (testsuite)
	  results <- rktest.runRKTestSuite (suite=suite, basedir=getwd(), test.id=test.id)
	  allresults <- rktest.appendTestResults (allresults, results)
  }

  cat ("\n\nOverall results:\n")
  print (allresults)

  if (any (is.na (allresults@passed))) {
	  cat ("\nNOTE: Skipped tests due to missing libaries are not an indication of problems.")
	  cat ("\nCurrently, the following R packages are needed in order to run all available tests:")
	  # TODO: Make this list dynamic and / or print only the missing libs
	  cat ('\n"R2HTML", "tseries", "nortest", "outliers", "exactRankTests", "moments", "car", "hdrcde", "qcc", "xtable", "eRm", "ltm"')
  }

  sink()

  cat (paste ("\n\nThese output are saved in: ", paste (getwd(), outfile, sep=.Platform$file.sep), ".\nIf needed, send them to rkward-devel@lists.sourceforge.net\n", sep=""))

}