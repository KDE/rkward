\name{rk.graph.on}
\alias{rk.graph.on}
\alias{rk.graph.off}

\title{Copy device to HTML output}

\description{
  Create or copy a device to a html page which is displayed as "output" in RKWard; accessible from Windows->Show Output. \code{rk.graph.off} closes the device that was opened by \code{rk.graph.on}. The default settings for \code{device.type}, \code{width}, \code{height}, and \code{quality} can be modified from Settings -> Configure RKWard -> Output.
}

\usage{
rk.graph.on(device.type = getOption("rk.graphics.type"), 
            width = getOption("rk.graphics.width"), height = getOption("rk.graphics.height"), 
            quality, ...)

rk.graph.off()
}
\arguments{
  \item{device.type}{a string, either \code{"PNG"} or \code{"JPG"} or \code{"SVG"} or \code{NULL}. In case of \code{NULL}, the default, \code{"PNG"}, is used.}
  \item{width,height}{in pixels, the width and height of the copied device; default is 480 for both.}
  \item{quality, \dots}{other arguments passed to the device function}
}

\value{
  \code{rk.graph.on} invisibly returns the value of the corresponding device call, which, generally, is nothing.
  
  \code{rk.graph.off} returns the number and name of the device that was active before \code{rk.graph.on} was called, whenever possbile. When not possible it returns the number and name of the next active device as done by \code{\link{dev.off}}.
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\section{Warning}{
  \itemize{
    \item It is advised to use \code{rk.graph.off} and \bold{not} \code{dev.off} to close the device opened by \code{rk.graph.on}.
  
    \item \code{dev.print(device = rk.graph.on)} is a \bold{wrong} usage for this "device," and will result in errors.
  }
}

\seealso{
  \code{\link{png}}, \code{\link{svg}}, \code{\link{dev.copy}}, \code{\link{dev.off}}, \url{rkward://page/rkward_output}
}

\examples{
require (rkward)

## Plot directly to the output (html) file, by-passing screen device:
rk.graph.on ("JPG", 480, 480, 75)
plot (rnorm (100))
rk.graph.off ()

## Copy the displayed plot to the output:
plot (rnorm (100))
dev.copy (device = rk.graph.on)
rk.graph.off ()

## WRONG USAGE: not run:
#plot (rnorm (100))
#dev.print (device = rk.graph.on)
}

\keyword{device}
