\name{rk.paste.JS}
\alias{rk.paste.JS}
\title{Paste JavaScript objects and character strings}
\usage{
  rk.paste.JS(..., level = 1, indent.by = "\t", funct =
  NULL)
}
\arguments{
  \item{...}{Objects of class \code{rk.JS.ite},
  \code{rk.JS.arr}, \code{rk.JS.opt} or character.}

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
  Paste JavaScript objects and character strings
}
\seealso{
  \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
  \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
  \code{\link[rkwarddev:ite]{ite}}, and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

