\name{rk.XML.cbox}
\alias{rk.XML.cbox}
\title{Create XML node "checkbox" for RKWard plugins}
\usage{
  rk.XML.cbox(label, val, chk = FALSE, id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for this
  plugin element.}

  \item{val}{Character string, the value to submit if the
  element is checked.}

  \item{chk}{Logical, whether this element should be
  checked by default.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"}, an ID will be generated
  automatically from the label.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "checkbox" for RKWard plugins
}
\examples{
test.checkboxes <- rk.XML.row(rk.XML.col(
list(
rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
rk.XML.cbox(label="bar", val="bar2"))))
cat(pasteXMLNode(test.checkboxes, shine=1))
}

