\name{rk.XML.menu}
\alias{rk.XML.menu}
\title{Create XML "menu" node for RKWard plugins}
\usage{
  rk.XML.menu(label, index = 1, id.name = "auto")
}
\arguments{
  \item{label}{A label.}

  \item{index}{An index.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"}, an ID will be generated
  automatically from the tag names and IDs of the given
  nodes.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a menu node for entry sections.
}
\note{
  NOT WORKING YET
}
\seealso{
  \code{\link[rkwarddev:rk.XML.menus]{rk.XML.menus}}
}

