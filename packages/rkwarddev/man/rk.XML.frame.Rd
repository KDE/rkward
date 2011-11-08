\name{rk.XML.frame}
\alias{rk.XML.frame}
\title{Create XML node "column" for RKWard plugins}
\usage{
  rk.XML.frame(..., label = NULL, checkable = FALSE, chk =
  TRUE, id.name = "auto")
}
\arguments{
  \item{...}{Objects of class \code{XiMpLe.node}.}

  \item{label}{Character string, a text label for this
  plugin element.}

  \item{checkable}{Logical, if \code{TRUE} the frame can be
  switched on and off.}

  \item{chk}{Logical, if \code{TRUE} and
  \code{checkable=TRUE} the frame is checkable and active
  by default.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"} and a label was
  provided, an ID will be generated automatically from the
  label if presen, otherwise from the objects in the frame.
  If \code{NULL}, no ID will be given.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "column" for RKWard plugins
}
\examples{
test.dropdown <- rk.XML.dropdown("mydrop",
options=list("First Option"=c(val="val1"),
"Second Option"=c(val="val2", chk=TRUE)))
cat(pasteXMLNode(rk.XML.frame(test.dropdown, label="Some options")))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

