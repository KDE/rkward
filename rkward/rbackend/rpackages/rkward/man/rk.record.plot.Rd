\name{rk.record.plot}
\alias{rk.record.plot}
\alias{rk.toggle.plot.history}
\alias{rk.first.plot}
\alias{rk.previous.plot}
\alias{rk.next.plot}
\alias{rk.last.plot}
\alias{rk.goto.plot}
\alias{rk.force.append.plot}
\alias{rk.removethis.plot}
\alias{rk.clear.plot.history}
\alias{rk.show.plot.info}
\alias{rk.verify.plot.hist.limits}
\alias{rk.plot.history.summary}
\alias{rk.duplicate.device}

\title{Record screen device history and duplicate a screen device}
\description{
  TODO TODO
}

\usage{
rk.toggle.plot.history(x = TRUE)

rk.first.plot(devId = dev.cur())
rk.previous.plot(devId = dev.cur())
rk.next.plot(devId = dev.cur())
rk.last.plot(devId = dev.cur())
rk.goto.plot(devId = dev.cur(), index=1)

rk.force.append.plot(devId = dev.cur())
rk.removethis.plot(devId = dev.cur())

rk.clear.plot.history()

rk.show.plot.info(devId = dev.cur())
rk.plot.history.summary(which = NULL, type = c("devices", "history"))

rk.duplicate.device(devId = dev.cur())
}

\arguments{
  \item{devId, which}{ an integer, the screen device on which an action is to be performed.}
  \item{x}{ a logical (not NA), whether to enable the screen device history.}
  \item{index}{ an integer, which plot to jump to?}
  \item{type}{ a string, either \code{"devices"} or \code{"history"}, the type of summary to be printed.}
}

\details{
  TODO TODO
  
  \code{rk.record.plot} is an environment to store the screen device history. You should not use the functions / variables in this environment directly, instead use the many wrapper functions as described below.
  
  \code{rk.toggle.plot.history} enables or disables the screen device history. You should not use this function. Instead use the checkbox in Settings->Configure RKWard->Onscreen Graphics->Screen device history. After the needed initialization / clean up,  it sets the option variable \code{"rk.enable.graphics.history"}  to \code{x}.
  
  \code{rk.first.plot}, \code{rk.previous.plot}, \code{rk.next.plot}, \code{rk.last.plot} these functions provide browing actions to respective plots saved in the history on the specified device (\code{devId}).
  
  \code{rk.goto.plot} this function provides a one step jump action to the plot specified by \code{index} on the specified device (\code{devId}).
  
  \code{rk.force.append.plot} forcefully append the currently displayed plot to the history. This function ignores the type of plot (graphics / lattice) and by-passes the general recording mechanism, as a result the plot call can not be identified.
  
  \code{rk.removethis.plot} removes the plot displayed on the specified device from history. If there are more than one device showing the same plot then removing from one device does not wipe it from the other devices. They can be re-added to the history from the other devices.
  
  \code{rk.clear.plot.history} clears the screen device history.
  
  \code{rk.show.plot.info} when available, shows some extra information regarding the displayed plot on the specified device.
  
  \code{rk.plot.history.summary} provides some summaries of the screen device history. \code{type = "devices"} provides summary of all or one device(s) depending on whether \code{which = NULL} or \code{which} is a single number identifying the device. \code{type = "history"} provides summary of the entire stored history. 
  
  \code{rk.duplicate.device} duplicates the specified screen device. The plot on the new device behaves independently of the one it was duplicated from.
}

\value{
  Except those mentioned below, the rest return \code{NULL} invisibly.
  
  \code{rk.plot.history.summary} returns a data.frame with some messages.
  
  \code{rk.duplicate.device} returns the value of a \code{dev.copy} call.
}
\author{Prasenjit Kapat \email{rkward-devel@lists.sourceforge.net}}

\section{Warning}{
  TODO TODO
}

\seealso{
  \code{\link{dev.cur}}, \code{\link{dev.copy}}, \code{\link{setHook}},
  
  \code{\link{graphics}}, \code{\link{recordPlot}}, \code{\link{replayPlot}}, \code{\link{persp}},
  
  \code{\link{Lattice}}, \code{\link{print.trellis}}, \code{\link{trellis.last.object}}
}

\examples{
## Example for this?
require (rkward)

.L. <- getOption ("rk.graphics.hist.max.length")

local ({
 options ("rk.graphics.hist.max.length" = 150)
 x <- seq(-2*pi,2*pi,length=400)
 xlim <- range(x); ylim <- c(-1,1)
 n <- 100;
 for (i in seq_len (n)) {
   plot(x, sin(x-(i-1)*4*pi/n), type='l', xlim=xlim, ylim=ylim, 
        bty='n', xlab='', ylab='', xaxt='n', yaxt='n')
 }
})

rk.first.plot ()
for (i in 1:(rk.record.plot$sP.length-1)) rk.next.plot ()

## Not run
rk.clear.plot.history ()
options ("rk.graphics.hist.max.length" = .L.)

}
\keyword{device}
\keyword{dynamic}
\keyword{environment}
