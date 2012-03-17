\name{rk.XML.radio}
\alias{rk.XML.radio}
\title{Create XML node "radio" for RKWard plugins}
\usage{
  rk.XML.radio(label,
    options = list(label = c(val = NULL, chk = FALSE)),
    id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for this
  plugin element.}

  \item{options}{A named list with options to choose from.
  The names of the list elements will become labels of the
  options, \code{val} defines the value to submit if the
  option is checked, and \code{chk=TRUE} should be set in
  the one option which is checked by default.}

  \item{id.name}{Character string, a unique ID for this
  plugin element.  If \code{"auto"} and a label was
  provided, an ID will be generated automatically from the
  label.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "radio" for RKWard plugins
}
\examples{
test.radio <- rk.XML.radio("Chose one",
  options=list("First Option"=c(val="val1"),
  "Second Option"=c(val="val2", chk=TRUE)))
cat(pasteXML(test.radio))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

