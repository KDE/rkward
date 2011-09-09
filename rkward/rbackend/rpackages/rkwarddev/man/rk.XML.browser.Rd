\name{rk.XML.browser}
\alias{rk.XML.browser}
\title{Create XML node "browser" for RKWard plugins}
\usage{
  rk.XML.browser(label, dir = TRUE, id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for this
  plugin element.}

  \item{dir}{Logical, if \code{TRUE} type of object browser
  defaults to "dir", otherwise "file".}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"} and a label was
  provided, an ID will be generated automatically from the
  label.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "browser" for RKWard plugins
}
\examples{
test.browser <- rk.XML.browser("Browse here:")
cat(pasteXMLNode(test.browser, shine=1))
}

