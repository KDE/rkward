\name{rk.XML.varslot}
\alias{rk.XML.varslot}
\title{Create a XML node "varslot" for RKWard plugins}
\usage{
  rk.XML.varslot(label, source, required = FALSE, multi =
  FALSE, min = 1, any = 1, max = 0, dim = 0, min.len = 0,
  max.len = NULL, classes = NULL, types = NULL, id.name =
  "auto")
}
\arguments{
  \item{label}{Character string, a text label for the
  varslot.}

  \item{source}{Either a character string (the \code{id}
  name of the \code{varselector} to select variables from),
  or an object of class \code{XiMpLe.node} (whose \code{id}
  will be extracted and used, must be a
  \code{<varselector>} node).}

  \item{required}{Logical, whether the selection of
  variables is mandatory or not.}

  \item{multi}{Logical, whether the varslot holds only one
  or several objects.}

  \item{min}{If \code{multi=TRUE} defines how many objects
  must be selected. Sets \code{multi=TRUE}.}

  \item{any}{If \code{multi=TRUE} defines how many objects
  must be selected at least if any are selected at all.
  Sets \code{multi=TRUE}.}

  \item{max}{If \code{multi=TRUE} defines how many objects
  can be selected in total (0 means any number). Sets
  \code{multi=TRUE}.}

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

  \item{id.name}{Character vector, unique ID for the
  varslot. If \code{"auto"}, the ID will be generated
  automatically from \code{label}.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create a XML node "varslot" for RKWard plugins
}
\examples{
\dontrun{
test.varselector <- rk.XML.varselector("Select some vars")
test.varslot <- rk.XML.varslot("Vars go here", source=test.varselector)
cat(pasteXMLNode(test.varslot))
}
}
\seealso{
  \code{\link[rkwarddev:rk.XML.vars]{rk.XML.vars}},
  \code{\link[rkwarddev:rk.XML.varselector]{rk.XML.varselector}},
  and the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

