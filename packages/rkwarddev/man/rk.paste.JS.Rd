\name{rk.paste.JS}
\alias{rk.paste.JS}
\title{Paste JavaScript objects and character strings}
\usage{
  rk.paste.JS(..., level = 2, indent.by = "\t", funct =
  NULL, array = NULL, var.prefix = NULL, modifiers = NULL,
  default = NULL, join = NULL)
}
\arguments{
  \item{...}{Objects of class \code{rk.JS.ite},
  \code{rk.JS.arr}, \code{rk.JS.opt} or character. Another
  special case is XiMpLe nodes created by
  \code{rk.comment()}, which will be turned into JavaScript
  comments (i.e., lines starting with "//").}

  \item{level}{Integer, which indentation level to use,
  minimum is 1.}

  \item{indent.by}{A character string defining the
  indentation string to use.}

  \item{funct}{For \code{rk.JS.arr} and \code{rk.JS.opt}
  objects only: Character string, name of the R function to
  be called to combine the options, e.g. "list" for
  \code{list()}, or "c" for \code{c()}.}

  \item{array}{For \code{rk.JS.opt} objects only: Logical,
  whether the options should be collected in an array or a
  concatenated character string.}

  \item{var.prefix}{For \code{rk.JS.var} objects only: A
  character string. will be used as a prefix for the JS
  variable names.}

  \item{modifiers}{For \code{rk.JS.var} objects only: A
  character vector with modifiers you'd like to apply the
  XML node's property.}

  \item{default}{For \code{rk.JS.var} objects only:
  Logical, if \code{TRUE} the default value (no special
  modifier) of the node will also be defined. Does nothing
  if \code{modifiers=NULL}.}

  \item{join}{For \code{rk.JS.var} objects only: A
  character string, useful for GUI elements which accept
  multiple objects (i.e., multi-varslots). If \code{join}
  is something other than \code{""}, these objects will be
  collapsed into one string when pasted, joined by this
  string.}
}
\value{
  A character string.
}
\description{
  Paste JavaScript objects and character strings
}
\note{
  To get a list of the implemented modifiers in this
  package, call \code{rkwarddev:::all.valid.modifiers}.
}
\seealso{
  \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
  \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
  \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
  \code{\link[rkwarddev:ite]{ite}}, and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

