\name{rk.XML.external}
\alias{rk.XML.external}
\title{Create XML node "external" for RKWard plugins}
\usage{
  rk.XML.external(id, default = NULL)
}
\arguments{
  \item{id}{Character string, the ID of the new property.}

  \item{default}{Character string, initial value of the
  property if not connected.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "external" for RKWard plugins
}
\examples{
test.external <- rk.XML.external(id="ext_property", default="none")
cat(pasteXMLNode(test.external, shine=1))
}

