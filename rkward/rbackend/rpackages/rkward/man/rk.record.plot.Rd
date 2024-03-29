% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/public_graphics.R
\docType{data}
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
\alias{rk.without.plot.history}
\title{Record screen device history and duplicate a screen device}
\format{
An object of class \code{environment} of length 57.
}
\usage{
rk.record.plot

rk.toggle.plot.history(x = TRUE)

rk.first.plot(devId = dev.cur())

rk.previous.plot(devId = dev.cur())

rk.next.plot(devId = dev.cur())

rk.last.plot(devId = dev.cur())

rk.goto.plot(devId = dev.cur(), index = 1)

rk.force.append.plot(devId = dev.cur())

rk.removethis.plot(devId = dev.cur())

rk.clear.plot.history()

rk.show.plot.info(devId = dev.cur())

rk.verify.plot.hist.limits(lmax)

rk.plot.history.summary(which = NULL, type = c("devices", "history"))

rk.duplicate.device(devId = dev.cur())

rk.without.plot.history(expr)
}
\arguments{
\item{x}{a logical (not \code{NA}), whether to enable the screen device history.}

\item{devId}{integer, the screen device on which an action is to be performed.}

\item{index}{integer, which plot to jump to.}

\item{lmax}{integer, the desired history limit.}

\item{which}{integer identifying the device. If \code{NULL} defaults to all devices.}

\item{type}{one of \code{"devices"} or \code{"history"}, the type of summary to be
printed. \code{type = "devices"} provides summary of all or one device(s),
depending on the value of \code{which}.
\code{type = "history"} provides summary of the entire stored history.}

\item{expr}{the plot call to evaluate.}
}
\value{
\code{rk.plot.history.summary} returns a data.frame with messages.
  \code{rk.duplicate.device} returns the value of a \code{\link[grDevices:dev.copy]{dev.copy}} call.
  All other functions return \code{NULL} invisibly.
}
\description{
\code{rk.record.plot} is an environment to store the screen device history.
You should not use the functions/variables in this environment directly, instead use
the many wrapper functions as described below.
}
\details{
\code{rk.toggle.plot.history} enables or disables the screen device history.
You should \emph{not} use this function directly. Instead, use the checkbox in
Settings->Configure RKWard->Onscreen Graphics->Screen device history.
After the needed initialization / clean up, it sets the option variable
\code{"rk.enable.graphics.history"} to \code{x}.

The functions \code{rk.first.plot}, \code{rk.previous.plot}, \code{rk.next.plot}, and
\code{rk.last.plot} provide browing actions to respective plots saved in the history
on the specified device (\code{devId}).

\code{rk.goto.plot} provides a one step jump action to the plot specified
by \code{index} on the specified device (\code{devId}).

\code{rk.force.append.plot} forcefully append the currently displayed plot to the history.
This function ignores the type of plot (graphics/lattice) and by-passes the general
recording mechanism, as a result the plot call can not be identified.

\code{rk.removethis.plot} removes the plot displayed on the specified device from history.
If there are more than one device showing the same plot then removing from one device does
not wipe it from the other devices. They can be re-added to the history from the other
devices.

\code{rk.clear.plot.history} clears the screen device history.

\code{rk.show.plot.info} shows some extra information regarding the
displayed plot on the specified device, when available.

\code{rk.plot.history.summary} provides some summaries of the screen device history.

\code{rk.duplicate.device} duplicates the specified screen device.
The plot on the new device behaves independently of the one it was duplicated from.

\code{rk.without.plot.history} runs a (plotting) action, without recording anything in the
plot history. Internally, the plot history option is turned off for the duration of the
action.
}
\examples{
\dontrun{
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

rk.clear.plot.history ()
options ("rk.graphics.hist.max.length" = .L.)
}
}
\seealso{
\code{\link[grDevices:recordPlot]{recordPlot}},
  \code{\link[grDevices:replayPlot]{replayPlot}},
  \code{\link[lattice:print.trellis]{print.trellis}},
  \code{\link[lattice:trellis.last.object]{trellis.last.object}},
  \url{rkward://page/rkward_plot_history}
}
\author{
Prasenjit Kapat \email{rkward-devel@kde.org}
}
\keyword{device}
\keyword{dynamic}
\keyword{environment}
