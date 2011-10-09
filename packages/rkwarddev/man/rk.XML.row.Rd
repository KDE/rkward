\name{rk.XML.row}
\alias{rk.XML.row}
\title{Create XML node "row" for RKWard plugins}
\usage{
  rk.XML.row(..., id.name = "auto")
}
\arguments{
  \item{...}{Objects of class \code{XiMpLe.node}.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"}, an ID will be generated
  automatically from the objects in \code{...}. If
  \code{NULL}, no ID will be given.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "row" for RKWard plugins
}
\examples{
test.checkboxes <- rk.XML.row(rk.XML.col(
rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
rk.XML.cbox(label="bar", val="bar2")))
cat(pasteXMLNode(test.checkboxes))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

