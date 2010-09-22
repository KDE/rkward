\name{rk.get.tempfile.name}

\alias{rk.get.tempfile.name}
\alias{rk.get.output.html.file}
\alias{rk.set.output.html.file}

\title{Output file names}

\usage{
rk.get.tempfile.name(prefix="image", extension=".jpg")
rk.get.output.html.file()
rk.set.output.html.file(x)
}

\arguments{
  \item{prefix}{a string, used as a filename prefix when saving images to the output file}
  \item{extension}{a string, used as a filename extension when saving images to the output file}
  \item{x}{a string, giving the filename of the of the output file}
}

\details{
  In RKWard the output is saved as a html file which is located at "~/.rkward" (\bold{TODO}: make this platform free). The name of this html file can be retrieved and set using \code{rk.get.output.html.file} and \code{rk.set.output.html.file}. Images that are copied to output are also saved here. The filenames of these images are of the form "\code{prefix}%03d.\code{extension}"; which can be retrieved using \code{rk.get.tempfile.name}.
}

\value{
  \code{rk.get.tempfile.name} and \code{rk.get.output.html.file} returns a string while \code{rk.set.output.html.file} returns \bold{TODO} NULL?
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\examples{
\bold{TODO}
}

\keyword{utilities}
\keyword{IO}
