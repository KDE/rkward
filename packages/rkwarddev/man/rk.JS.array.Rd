\name{rk.JS.array}
\alias{rk.JS.array}
\title{Create a simple JavaScript array}
\usage{
  rk.JS.array(option, variables = list(), funct = "c",
  var.prefix = NULL)
}
\arguments{
  \item{option}{A character string, naming, e.g., an option
  of an R function which should be constructed from several
  variables.}

  \item{variables}{A list with either character strings
  (the names of the variables to combine to a vector or
  list), of objects of class \code{XiMpLe.node} with plugin
  XML nodes (whose ID will be extracted and used).}

  \item{funct}{Character string, name of the R function to
  be called to combine the options, e.g. "list" for
  \code{list()}, or "c" for \code{c()}.}

  \item{var.prefix}{A character string. sets a global
  string to be used as a prefix for the JS variable names.}
}
\value{
  An object of class \code{rk.JS.arr}.
}
\description{
  If you need to combine multiple options (like values of
  several checkboxes) into one vector or list, this
  function can help with that task. All relevant variables
  will become part of an array and then joined into the
  desired argument type.
}
\examples{
# create three checkboxes for independent options
checkA <- rk.XML.cbox(label="Run Test A", value="A")
checkB <- rk.XML.cbox(label="Run Test B", value="B")
checkC <- rk.XML.cbox(label="Run Test C", value="C")
# combine them into one list of options via JavaScript
rk.JS.array("run.tests", variables=list(checkA, checkB, checkC), funct="list")
}
\seealso{
  \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
  \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
  \code{\link[rkwarddev:echo]{echo}},
  \code{\link[rkwarddev:id]{id}}
}

