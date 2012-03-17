\name{rk.XML.varselector}
\alias{rk.XML.varselector}
\title{Create node "varselector" for RKWard plugins}
\usage{
  rk.XML.varselector(label = NULL, id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for the
  variable selection slot.  Must be set if
  \code{id.name="auto"}.}

  \item{id.name}{Character vector, unique ID for this
  element.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create node "varselector" for RKWard plugins
}
\examples{
test.varselector <- rk.XML.varselector("Select some vars")
cat(pasteXML(test.varselector))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.varslot]{rk.XML.varslot}},
  \code{\link[rkwarddev:rk.XML.vars]{rk.XML.vars}}, and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

