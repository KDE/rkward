\name{rk.load.pluginmaps}
\alias{rk.load.pluginmaps}
\title{(Re-)load the given pluginmap files into the RKWard GUI}
\arguments{
  \item{pluginmap.files}{a character vector of file names
  to add. This may be left empty, if the only desired
  effect is to reload all active pluginmaps.}

  \item{force.add}{logical. Whether the pluginmap files
  should also be added, if they had been previously
  de-selected in the settings menu. In scripted usage, this
  should generally be set to FALSE.}

  \item{force.reload}{logical. By default the active
  pluginmaps are reloaded, only if any new ones were added.
  If set to TRUE, pluginmaps are reloaded in any case. In
  scripted usage, this should generally be set to FALSE.
  NOTE: Since a reload always means reloading _all_ active
  pluginmaps, This may be slow, and should be used with
  care.

  \bold{Note}: It is not necessary to reload the pluginmap,
  in order to refresh an individual plugin (e.g. after
  modifying the dialog), as plugins are not kept in memory
  after closing. Any currently opened plugins are not
  affected by this function.}
}
\description{
  (Re-)load the given pluginmap files into the RKWard GUI
}
\examples{
## NOT RUN

## reload all active pluginmaps
rk.load.pluginmaps()

## END NOT RUN
}
\author{
  Thomas Friedrichsmeier
  \email{rkward-devel@lists.sourceforge.net}
}
\seealso{
  \code{\link{rk.call.plugin}},

  \code{\link{rkwarddev::rk.plugin.skeleton}}
}
\keyword{utilities}

