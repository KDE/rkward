\name{rk.JS.vars}
\alias{rk.JS.vars}
\title{Define variables in JavaScript code}
\usage{
  rk.JS.vars(variables, var.prefix = NULL, properties =
  NULL, default = FALSE, level = 2, indent.by = "\t")
}
\arguments{
  \item{variables}{A list with either character strings
  (the names of the variables to define), of objects of
  class \code{XiMpLe.node} with plugin XML nodes (whose ID
  will be extracted and used).}

  \item{var.prefix}{A character string. If
  \code{def.vars=TRUE}, this string will be used as a
  prefix for the JS variable names.}

  \item{properties}{A character vector with properties
  you'd like to get of the XML node.}

  \item{default}{Logical, if \code{TRUE} the default value
  (no special property) of the node will also be defined.
  Does nothing if \code{properties=NULL}.}

  \item{level}{Integer, which indentation level to use,
  minimum is 1.}

  \item{indent.by}{A character string defining how
  indentation should be done.}
}
\value{
  A character string.
}
\description{
  Define variables in JavaScript code
}
\examples{
# create three checkboxes
checkA <- rk.XML.cbox(label="Run Test A", value="A")
checkB <- rk.XML.cbox(label="Run Test B", value="B")
checkC <- rk.XML.cbox(label="Run Test C", value="C")
# define them by their ID in JavaScript
cat(rk.JS.vars(list(checkA, checkB, checkC)))
}
\seealso{
  \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
  \code{\link[rkwarddev:echo]{echo}},
  \code{\link[rkwarddev:id]{id}}
}

