\name{rktest.setSuiteStandards}
\alias{rktest.setSuiteStandards}
\title{Set RKWard suite standards}
\usage{rktest.setSuiteStandards(suite, testroot=getwd(), file=TRUE)}
\description{Set RKWard plugin test suite standards}
\details{Use this function after you plugin passed all tests to set the resulting code,
output and R messages as the standard that will be compared to during following tests.}
\alias{rktest.setSuiteStandards}
\value{The function simply copies the previously created files from the temporary directory to
the directory with the test standards.}
\author{Thomas Friedrichsmeier \email{thomas.friedrichsmeier@ruhr-uni-bochum.de}}
\keyword{utilities}
\seealso{\code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}}
\arguments{\item{suite}{Character string naming the test suite to set standards for.}
\item{testroot}{Path to the test root directory, defaults to the working directory.}
\item{file}{Logical: If \code{suite} is already a present R object, set this to FALSE.
Otherwise it is assumed to be a file and fed to \code{source}.}
}
\examples{\dontrun{
rktest.setSuiteStandards("rkward_application_tests.R")
}}

