\name{rktest.setSuiteStandards}
\alias{rktest.setSuiteStandards}
\title{Set RKWard suite standards}
\usage{rktest.setSuiteStandards(suite, basedir=getwd())}
\description{Set RKWard plugin test suite standards}
\details{Use this function after you plugin passed all tests to set the resulting code,
output and R messages as the standard that will be compared to during following tests.}
\alias{rktest.setSuiteStandards}
\value{The function simply changes the names of the previously created files,
specifically adding the prefix "RKTestStandard.".}
\author{Thomas Friedrichsmeier \email{thomas.friedrichsmeier@ruhr-uni-bochum.de}}
\keyword{utilities}
\seealso{\code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}}
\arguments{\item{suite}{Character string naming the test suite to set standards for.}
\item{basedir}{Should point to the testroot, defaults to the working directory.}
}
\examples{\dontrun{
rktest.setSuiteStandards("rkward_application_tests.R")
}}

