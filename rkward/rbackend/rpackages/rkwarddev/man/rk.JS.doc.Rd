\name{rk.JS.doc}
\alias{rk.JS.doc}
\title{Create JavaScript outline from RKWard plugin XML}
\usage{
  rk.JS.doc(require = c(), variables = NULL, results.header
  = NULL, indent.by = "\t")
}
\arguments{
  \item{require}{A character vector with names of R
  packages that the dialog depends on.}

  \item{variables}{A character string to be included to
  read in all needed variables from the dialog. Refer to
  \code{\link{rk.JS.scan}} for a function to create this
  from an existing plugin XML file.}

  \item{results.header}{A character string to headline the
  printed results.}

  \item{indent.by}{A character string defining how
  indentation should be done.}
}
\value{
  A character string.
}
\description{
  Create JavaScript outline from RKWard plugin XML
}

