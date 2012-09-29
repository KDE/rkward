\name{rk.get.label}
\alias{rk.get.description}
\alias{rk.get.label}
\alias{rk.get.short.name}
\alias{rk.list}
\alias{rk.list.labels}
\alias{rk.list.names}
\alias{.rk.make.short.name}
\alias{rk.set.label}
\title{Various label related utility functions}
\usage{
  rk.get.label(x)

  rk.set.label(x, label, envir = parent.frame())

  rk.list.labels(x, fill = FALSE)

  rk.get.short.name(x)

  .rk.make.short.name(x)

  rk.get.description(..., paste.sep = NULL,
    is.substitute = FALSE)

  rk.list(...)

  rk.list.names(..., deparse.level = 2)
}
\arguments{
  \item{x}{any R object}

  \item{label}{a string, to set the label attribute of an
  object}

  \item{envir}{an environment, where the attribute is
  evaluated}

  \item{fill}{a logical or character. See Details.}

  \item{paste.sep}{a string, used as the \code{collapse}
  argument for paste}

  \item{is.substitute}{a logical (not NA). See Details.}
}
\value{
  \code{rk.set.label} returns the result of the evaluation
  of "setting the label" while the others return a
  character vector.
}
\description{
  \code{rk.get.label} retrieves the rkward label (if any)
  of the given object.
}
\details{
  \code{rk.set.label} sets the rkward label for the given
  object.

  \code{rk.list.labels} retrieves the rkward labels for a
  list of objects. Most importantly, this can be used for
  extracting all column labels in a \code{data.frame},
  conveniently. The parameter \code{fill} controls, what
  happens, when no rkward labels have been assigned. The
  default (\code{FALSE}) is to return empty strings for any
  missing labels. For \code{fill=TRUE}, missing labels will
  be filled with the short names of the object. You can
  also pass a character vector of default labels to use as
  the \code{fill} parameter.

  \code{rk.get.short.name} creates a short name for the
  given object.

  \code{rk.get.description} creates descriptive string(s)
  for each of the arguments in "\code{\dots{}}"; collapsing
  into a single string using \code{paste.sep} (if not
  NULL). If \code{is.substitute=TRUE}, the arguments will
  be deparsed, first, which can be useful when using
  \code{rk.get.description} inside a function.

  \code{rk.list.names} returns the names of the arguments
  passed as \code{...}; when using \code{rk.list.names}
  inside a function, it may be necessary to increase the
  \code{deparse.level} level.

  \code{rk.list} returns a list of its arguments, with
  \code{names} set as returned by
  \code{rk.get.description()}. This can be used as a
  drop-in replacement for \code{\link{list}}.
}
\examples{
x <- data.frame(a=c(1:3), b=c(2:4))
rk.set.label(x[["a"]], "First column")
rk.get.short.name (x$a)                   # "x$a"
rk.get.label (x$a)                        # "First column"
rk.get.description (x$a)                  # "x$a (First column)"
rk.list.labels (x)                        # "First column" ""
rk.list.labels (x, TRUE)                  # "First column" "b"
rk.list.names (x, x$a, x$b)               # "x" "x$a" "x$b"
names (rk.list (x$a, x$b))                # "x$a (First column)" "x$b"
}
\author{
  Thomas Friedrichsmeier
  \email{rkward-devel@lists.sourceforge.net}
}
\keyword{utilities}

