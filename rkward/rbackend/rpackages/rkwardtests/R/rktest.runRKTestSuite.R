#' Run a single RKWard plugin test suite
#'
#' This function can be called to run a single plugin test suite.
#' 
#' @title Run RKWard plugin test suite
#' @usage rktest.runRKTestSuite(suite, testroot=getwd(), test.id=NULL)
#' @aliases rktest.runRKTestSuite
#' @param suite Character string naming the test suite to run.
#' @param testroot Defaults to the working directory.
#' @param test.id An optional character string or vector naming one or more tests of a suite to be run (if NULL, all tests are run).
#' @return An object of class \code{\link[rkwardtests:RKTestResult]{RKTestResult-class}}.
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}, Meik Michalke \email{meik.michalke@@uni-duesseldorf.de}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}
#' @export
#' @examples
#' \dontrun{
#' result <- rktest.runRKTestSuite()
#' }

rktest.runRKTestSuite <- function (suite, testroot=getwd (), test.id=NULL) {
	# check wheter test environment is already set,
	# otherwise initialize
	if(!exists("initialized", where=rkwardtests::.rktest.tmp.storage) || !get("initialized", pos=rkwardtests::.rktest.tmp.storage)){
	  rktest.initializeEnvironment()
	  on.exit(rktest.resetEnvironment())
	}

	result <- new ("RKTestResult")		# FALSE by default

	if (!inherits (suite, "RKTestSuite")) return (result)
	if (!validObject (suite)) return (result)

	# clean any old results
	rktest.cleanRKTestSuite (suite)

	oldwd = getwd ()
	on.exit (setwd (oldwd), add=TRUE)
#	setwd (file.path(testroot, suite@id))
	setwd (rktest.createTempSuiteDir(suite@id))

	if (length (suite@initCalls) > 0) {
		for (i in 1:length (suite@initCalls)) try (suite@initCalls[[i]]())
	}
	rk.sync.global ()	# objects might have been added/changed in the init calls

	# check if only a subset of tests is desired
	if(length(test.id) > 0)
	  suite@tests <- suite@tests[is.element(sapply(suite@tests, function(test){test@id}), test.id)]

	for (i in 1:length (suite@tests)) {
		suite@tests[[i]]@libraries <- c(suite@libraries, suite@tests[[i]]@libraries)
		try (res <- rktest.runRKTest(test=suite@tests[[i]], standard.path=file.path(testroot, suite@id), suite.id=suite@id))
		result <- rktest.appendTestResults (result, res)
	}

	if (length (suite@postCalls) > 0) {
		for (i in 1:length (suite@postCalls)) try (suite@postCalls[[i]]())
	}

	return(result)
}
