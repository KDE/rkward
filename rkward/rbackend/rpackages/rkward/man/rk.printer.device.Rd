\name{rk.printer.device}
\alias{rk.printer.device}
\title{Device for printing using the KDE print dialog}
\usage{
  rk.printer.device(...)
}
\arguments{
  \item{...}{arguments are passed to
  \code{\link{postscript}}}
}
\value{
  Returns the name of the underlying temporary file,
  invisibly.
}
\description{
  Creates a device operating on a temporary file
  (internally a \code{\link{postscript}}() device). When
  the device is closed, it is printed, automatically, using
  the KDE print dialog (if installed).
}
\details{
  Typically this device is used with
  \code{\link{dev.print}}, as shown in the example, below.
}
\examples{
## Not run:
plot (rnorm (10))
dev.print (rk.printer.device)
}
\author{
  Thomas Friedrichsmeier
  \email{rkward-devel@lists.sourceforge.net}
}
\seealso{
  \code{\link{postscript}}, \code{\link{dev.print}},
  \code{\link{rk.graph.on}}
}
\keyword{device}
\keyword{utilities}

