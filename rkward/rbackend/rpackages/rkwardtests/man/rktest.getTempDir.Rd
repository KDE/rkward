\name{rktest.getTempDir}
\alias{rktest.getTempDir}
\title{Get path to the temporary directory}
\usage{rktest.getTempDir()}
\description{Get the path to the recent temporary directory, if one exists.}
\details{This function will return either the local path to the temporary directory where
all test results have been saved to, or FALSE if none exitsts.}
\alias{rktest.getTempDir}
\value{Either a character string, or FALSE.}
\author{Meik Michalke \email{meik.michalke@uni-duesseldorf.de}}
\keyword{utilities}
\seealso{\code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}}
\examples{rktest.getTempDir()}

