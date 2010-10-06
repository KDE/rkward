\name{rktest.runRKTestSuite}
\alias{rktest.runRKTestSuite}
\title{Run RKWard plugin test suite}
\usage{rktest.runRKTestSuite(suite, basedir=getwd())}
\description{Run a single RKWard plugin test suite}
\details{This function can be called to run a single plugin test suite.}
\alias{rktest.runRKTestSuite}
\value{An object of class \code{\link[rkwardtests:RKTestResult]{RKTestResult-class}}.}
\docType{function}
\author{Thomas Friedrichsmeier \email{thomas.friedrichsmeier@ruhr-uni-bochum.de}}
\keyword{utilities}
\seealso{\code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}}
\arguments{\item{suite}{Character string naming the test suite to run.}
\item{basedir}{Defaults to the working directory.}
}
\examples{\dontrun{
result <- rktest.runRKTestSuite()
}}

