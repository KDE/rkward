\name{rk.rkh.related}
\alias{rk.rkh.related}
\title{Create XML "related" node for RKWard help pages}
\usage{
  rk.rkh.related(links = NULL, text = NULL)
}
\arguments{
  \item{links}{A (list of) objects of class
  \code{XiMpLe.node}. They must all have the name "link".}

  \item{text}{Character string, the text to be displayed.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML "related" node for RKWard help pages
}
\seealso{
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

