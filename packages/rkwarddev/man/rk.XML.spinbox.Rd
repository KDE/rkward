\name{rk.XML.spinbox}
\alias{rk.XML.spinbox}
\title{Create XML node "spinbox" for RKWard plugins}
\usage{
  rk.XML.spinbox(label, min = NULL, max = NULL,
    initial = 0, real = TRUE, precision = 2,
    max.precision = 8, id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for this
  plugin element.}

  \item{min}{Numeric, the lowest value allowed. Defaults to
  the lowest value technically representable in the
  spinbox.}

  \item{max}{Numeric, the largest value allowed. Defaults
  to the highest value technically representable in the
  spinbox.}

  \item{initial}{Numeric, will be used as the initial
  value.}

  \item{real}{Logical, whether values should be real or
  integer numbers.}

  \item{precision}{Numeric, if \code{real=TRUE} defines the
  default number of decimal places shown in the spinbox.}

  \item{max.precision}{Numeric, maximum number of digits
  that can be meaningfully represented.}

  \item{id.name}{Character string, a unique ID for this
  plugin element.  If \code{"auto"} and a label was
  provided, an ID will be generated automatically from the
  label.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "spinbox" for RKWard plugins
}
\examples{
test.spinbox <- rk.XML.spinbox("Spin this:")
cat(pasteXMLNode(test.spinbox))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

