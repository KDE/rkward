\name{rk.XML.attribute}
\alias{rk.XML.attribute}
\title{Create XML "attribute" node for RKWard plugins}
\usage{
  rk.XML.attribute(label, value, id.name = "auto")
}
\arguments{
  \item{label}{A label.}

  \item{value}{A value.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"}, an ID will be generated
  automatically from the tag names and IDs of the given
  nodes.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a attribute node for component
  sections in .pluginmap files.
}
\note{
  NOT WORKING YET
}
\seealso{
  \code{\link[rkwarddev:rk.XML.attributes]{rk.XML.attributes}}
}

