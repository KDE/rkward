\name{rktest.makeplugintests}
\alias{rktest.makeplugintests}
\title{Run RKWard plugin test suite}
\usage{rktest.makeplugintests(testsuites, outfile="make_plugintests.txt")}
\description{Run a whole RKWard plugin test suite}
\details{The function \code{rktest.makeplugintests} will run a whole test suite that was prepared to check one or several RKWard plugins.}
\alias{rktest.makeplugintests}
\value{Results are printed to stdout and saved to the defined output file.}
\docType{function}
\author{Thomas Friedrichsmeier \email{thomas.friedrichsmeier@ruhr-uni-bochum.de}}
\keyword{utilities}
\seealso{\code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:RKTestResult]{RKTestResult-class}}}
\arguments{\item{testsuites}{A character vector naming the test suites to be run.}
\item{outfile}{A character string giving a file name for the result log.}
}
\examples{\dontrun{
rktest.makeplugintests(testsuites=c("rkward_application_tests.R", "import_export_plugins.R"))
}}

