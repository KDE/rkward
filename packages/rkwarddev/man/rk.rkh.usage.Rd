\name{rk.rkh.usage}
\alias{rk.rkh.usage}
\title{Create XML "usage" node for RKWard help pages}
\usage{
  rk.rkh.usage(text = NULL)
}
\arguments{
  \item{text}{Character string, the text to be displayed.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML "usage" node for RKWard help pages
}
\examples{
plugin.usage <- rk.rkh.usage("First do this, then do that ...")
}
\seealso{
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

