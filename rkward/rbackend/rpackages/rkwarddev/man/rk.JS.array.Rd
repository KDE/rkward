\name{rk.JS.array}
\alias{rk.JS.array}
\title{Create a simple JavaScript array}
\usage{
  rk.JS.array(option, variables = NULL, list = FALSE,
  def.vars = TRUE, var.prefix = "chc", indent.by = "\t")
}
\arguments{
  \item{option}{A character string, naming the option of,
  e.g., an R function which should be constructed from
  several variables.}

  \item{variables}{A character vector, the names of the
  variables to combine to a vector or list.}

  \item{list}{Logical, if \code{TRUE} the option will be
  constructed by \code{list()}, otherwise by \code{c()}.}

  \item{def.vars}{Logical, if \code{TRUE} the provided
  variables will also be defined.}

  \item{var.prefix}{A character string. If
  \code{def.vars=TRUE}, this string will be used as a
  prefix for the JS variable names.}

  \item{indent.by}{A character string defining how
  indentation should be done.}
}
\value{
  A character string.
}
\description{
  If you need to combine multiple options (like values of
  several checkboxes) into one vector or list, this
  function can help with that task. All relevant variables
  will become part of an array and then joined into the
  desired argument type.
}
\examples{
cat(rk.JS.array("my.option", variables=c("frst.var", "scnd.var")))
}

