\name{rktest.makeplugintests}
\alias{rktest.makeplugintests}
\title{Run RKWard plugin test suite}
\usage{rktest.makeplugintests(testsuites="testsuite.R", testroot=getwd(),
outfile="make_plugintests.txt", append=FALSE, test.id=NULL)}
\description{Run a whole RKWard plugin test suite}
\details{The function \code{rktest.makeplugintests} will run a whole test suite that was prepared to check one or several RKWard plugins.}
\alias{rktest.makeplugintests}
\value{Results are printed to stdout and saved to the defined output file.}
\author{Thomas Friedrichsmeier \email{thomas.friedrichsmeier@ruhr-uni-bochum.de}, Meik Michalke \email{meik.michalke@uni-duesseldorf.de}}
\keyword{utilities}
\seealso{\code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:RKTestResult]{RKTestResult-class}}}
\arguments{\item{testsuites}{A character string or vector naming the test suites to be run.}
\item{testroot}{A character string pointing to the root directory where the test suite resides (including its folder with test standards).}
\item{outfile}{A character string giving a file name for the result log.}
\item{append}{If TRUE, append output to an existing file.}
\item{test.id}{Optional character string or vector naming one or more tests of a suite to be run (if NULL, all tests are run).}
}
\examples{\dontrun{
rktest.makeplugintests(testsuites=c("rkward_application_tests.R",
"import_export_plugins.R"), testroot=getwd())
rktest.makeplugintests(testsuites="distribution.R",
testroot=getwd(), test.id=c("poisson_quantiles", "geom_quantiles"))
}}

