\name{rk.paste.JS}
\alias{rk.paste.JS}
\title{Paste objects of class rk.JS.ite}
\usage{
  rk.paste.JS(object, level = 1, indent.by = "\t", funct =
  NULL)
}
\arguments{
  \item{object}{An object of class \code{rk.JS.ite} or
  \code{rk.JS.arr}}

  \item{level}{Integer, which indentation level to use,
  minimum is 1.}

  \item{indent.by}{A character string defining the
  indentation string to use.}

  \item{funct}{For \code{rk.JS.arr} objects only: Character
  string, name of the R function to be called to combine
  the options, e.g. "list" for \code{list()}, or "c" for
  \code{c()}.}
}
\value{
  A character string.
}
\description{
  Paste objects of class rk.JS.ite
}
\seealso{
  \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
  \code{\link[rkwarddev:ite]{ite}}
}

