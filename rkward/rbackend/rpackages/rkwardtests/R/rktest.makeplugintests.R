#' Run a whole RKWard plugin test suite
#'
#' The function \code{rktest.makeplugintests} will run a whole test suite that was prepared to check one or several RKWard plugins.
#' 
#' @title Run RKWard plugin test suite
#' @usage rktest.makeplugintests(testsuites="testsuite.R", testroot=getwd(),
#' outfile="make_plugintests.txt", append=FALSE, test.id=NULL)
#' @aliases rktest.makeplugintests
#' @param testsuites A character string or vector naming the test suites to be run.
#' @param testroot A character string pointing to the root directory where the test suite resides (including its folder with test standards).
#' @param outfile A character string giving a file name for the result log.
#' @param append If TRUE, append output to an existing file.
#' @param test.id Optional character string or vector naming one or more tests of a suite to be run (if NULL, all tests are run).
#' @return Results are printed to stdout and saved to the defined output file.
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

rktest.makeplugintests <- function(testsuites="testsuite.R", testroot=getwd(), outfile="make_plugintests.txt", append=FALSE, test.id=NULL){
  ## change to test root directory
  oldwd <- getwd()
  on.exit(setwd(oldwd))
  setwd(testroot)

  ## initialize
  rktest.initializeEnvironment()
  on.exit(rktest.resetEnvironment(), add=TRUE)

  sink (file = outfile, append=append, type="output", split=TRUE)
  cat ("RKWard Version:\n")
  print (.rk.app.version)
  cat ("\n\nR-Version:\n")
  print (R.version)
  cat ("\n\nInstalled packages:\n")
  print (subset(installed.packages(),select=c(LibPath,Version)))

  allresults <- new ("RKTestResult")
  for (testsuite in testsuites) {
	  tryCatch(source(testsuite, local=TRUE), error=function(e) e)
	  results <- rktest.runRKTestSuite (suite=suite, testroot=testroot, test.id=test.id)
	  allresults <- rktest.appendTestResults (allresults, results)
  }

  cat ("\n\nOverall results:\n")
  print (allresults)

  if (any (is.na (allresults@passed))) {
	  missing.libs <- unique(allresults@missing_libs[!is.na(allresults@missing_libs)])
	  cat ("\nNOTE: Skipped tests due to missing libaries are not an indication of problems.")
	  cat ("\nThe following missing R packages are needed in order to run all tests:\n")
	  print(missing.libs)
  }

  sink()

  cat (paste ("\n\nThese output are saved in: ", paste (getwd(), outfile, sep=.Platform$file.sep), ".\nIf needed, send them to rkward-devel@lists.sourceforge.net\n", sep=""))
  cat (paste("\nThe full test results have been saved to this temporary directory:\n", rktest.getTempDir(),"\n"))

}