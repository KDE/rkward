\name{rktest.getTempDir}
\alias{rktest.getTempDir}
\title{Get path to the temporary directory}
\usage{
  rktest.getTempDir()
}
\value{
  Either a character string, or FALSE.
}
\description{
  Get the path to the recent temporary directory, if one
  exists.
}
\details{
  This function will return either the local path to the
  temporary directory where all test results have been
  saved to, or FALSE if none exitsts.
}
\examples{
rktest.getTempDir()
}
\author{
  Meik Michalke \email{meik.michalke@uni-duesseldorf.de}
}
\seealso{
  \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}
}
\keyword{utilities}

