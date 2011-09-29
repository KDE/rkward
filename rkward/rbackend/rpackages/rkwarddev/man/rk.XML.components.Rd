\name{rk.XML.components}
\alias{rk.XML.components}
\title{Create XML "components" node for RKWard plugins}
\usage{
  rk.XML.components(nodes = NULL)
}
\arguments{
  \item{nodes}{A (list of) objects of class
  \code{XiMpLe.node}. They must all have the name
  "component".}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a components node for a
  .pluginmap file, with optional child nodes "component".
}
\note{
  NOT WORKING YET
}
\seealso{
  \code{\link[rkwarddev:rk.XML.pluginmap]{rk.XML.pluginmap}}
  \code{\link[rkwarddev:rk.XML.component]{rk.XML.component}}
}

