#' Run a whole RKWard plugin test suite
#'
#' The function \code{rktest.makeplugintests} will run a whole test suite that was prepared to check one or several RKWard plugins.
#' 
#' @title Run RKWard plugin test suite
#' @usage rktest.makeplugintests(testsuites, outfile="make_plugintests.txt")
#' @aliases rktest.makeplugintests
#' @param testsuites A character vector naming the test suites to be run.
#' @param outfile A character string giving a file name for the result log.
#' @return Results are printed to stdout and saved to the defined output file.
#' @docType function
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:RKTestResult]{RKTestResult-class}}
#' @export
#' @examples
#' \dontrun{
#' rktest.makeplugintests(testsuites=c("rkward_application_tests.R", "import_export_plugins.R"))
#' }

rktest.makeplugintests <- function(testsuites, outfile="make_plugintests.txt"){
  ## initialize
  rktest.initializeEnvironment()

  ## add your test suite files, to this vector:
  #testsuites <- c ("rkward_application_tests.R", "import_export_plugins.R", "item_response_theory.R", "analysis_plugins.R", "distributions.R", "plots.R")

  #plugintest.outfile <- 'make_plugintests.txt'
  sink (file = outfile, append=FALSE, type="output", split=TRUE)
  cat ("RKWard Version:\n")
  print (.rk.app.version)
  cat ("\n\nR-Version:\n")
  print (R.version)
  cat ("\n\nInstalled packages:\n")
  print (subset(installed.packages(),select=c(LibPath,Version)))

  allresults <- new ("RKTestResult")
  for (testsuite in testsuites) {
	  source (testsuite)
	  allresults <- rktest.appendTestResults (allresults, results)
	  rm ("results")
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