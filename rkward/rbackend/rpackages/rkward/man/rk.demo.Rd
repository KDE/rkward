\name{rk.demo}
\alias{rk.demo}
\title{Opens an R demo script for editing}
\usage{
  rk.demo(topic, package = NULL, lib.loc = NULL)
}
\arguments{
  \item{topic}{topic of the example}

  \item{package}{package(s) to search for the demo. If NULL
  (the default), all currently loaded packages are
  searched.}

  \item{lib.loc}{Library locations.}
}
\value{
  Return \code{NULL}, unconditionally.
}
\description{
  \code{rk.demo} behaves similar to \code{\link{demo}}, but
  opens the demo script for editing, instead of sourcing
  it. Contrary to \code{\link{demo}}, the specification of
  a topic is mandatory.
}
\examples{
## Not run
rk.demo("graphics")
}
\author{
  Thomas Friedrichsmeier
  \email{rkward-devel@lists.sourceforge.net}
}
\seealso{
  \code{\link{rk.edit.files}}, \code{\link{rk.show.files}},
  \code{\link{demo}}
}
\keyword{IO}
\keyword{utilities}

