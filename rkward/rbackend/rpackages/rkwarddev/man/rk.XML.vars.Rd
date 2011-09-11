\name{rk.XML.vars}
\alias{rk.XML.vars}
\title{Create a variable selector for RKWard plugins}
\usage{
  rk.XML.vars(label, slot.text = NULL, required = FALSE,
  multi = FALSE, min = 1, any = 1, max = 0, dim = 0,
  min.len = 0, max.len = NULL, classes = NULL, types =
  NULL, horiz = TRUE, id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for the whole
  frame.}

  \item{slot.text}{Character string, a text label for the
  variable selection slot.}

  \item{required}{Logical, whether the selection of
  variables is mandatory or not.}

  \item{multi}{Logical, whether the varslot holds only one
  or several objects.}

  \item{min}{If \code{multi=TRUE} defines how many objects
  must be selected.}

  \item{any}{If \code{multi=TRUE} defines how many objects
  must be selected at least if any are selected at all.}

  \item{max}{If \code{multi=TRUE} defines how many objects
  can be selected in total (0 means any number).}

  \item{dim}{The number of dimensions, an object needs to
  have. If \code{dim=0} any number of dimensions is
  acceptable.}

  \item{min.len}{The minimum length, an object needs to
  have.}

  \item{max.len}{The maximum length, an object needs to
  have. If \code{NULL}, defaults to the largest integer
  number representable on the system.}

  \item{classes}{An optional character vector, defining
  class names to which the selection must be limited.}

  \item{types}{If you specify one or more variables types
  here, the varslot will only accept objects of those
  types. Valid types are "unknown", "number", "string",
  "factor", "invalid". Optional, use with great care, the
  user should not be prevented from making valid choices,
  and rkward does not always know the type of a variable!}

  \item{horiz}{Logical. If \code{TRUE}, the varslot will be
  placed next to the selector, if \code{FALSE} below it.}

  \item{id.name}{Character vector, unique IDs for the frame
  (first entry), the varselector (second entry) and varslot
  (third entry). If \code{"auto"}, IDs will be generated
  automatically from \code{label} and \code{slot.text}.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  This function will create a <frame> node including a
  <varselector> and a <varslot> node.
}
\examples{
test.vars <- rk.XML.vars("Select some vars", "Vars go here")
cat(pasteXMLNode(test.vars, shine=1))
}

