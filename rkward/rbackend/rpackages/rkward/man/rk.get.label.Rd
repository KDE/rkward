\name{rk.get.label}

\alias{rk.get.label}
\alias{rk.set.label}
\alias{rk.get.short.name}
\alias{rk.get.description}
\alias{rk.list.names}

\title{Various label related utility functions}

\usage{
rk.get.label(x)
rk.set.label(x, label, envir = parent.frame())
rk.get.short.name(x)
rk.get.description(..., paste.sep = NULL, is.substitute = FALSE)
rk.list.names(..., deparse.level = 2)
}

\arguments{
  \item{x}{any R object}
  \item{label}{a string, to set the label attribute of an object}
  \item{envir}{an environment, where the attribute is evaluated}
  \item{paste.sep}{a string, used as the \code{collapse} argument for paste}
  \item{is.substitute}{a logical (not NA). See Details.}
}

\details{
 \code{rk.get.label} retrieves the rkward label (if any) of the given object.

 \code{rk.set.label} sets the rkward label for the given object.

 \code{rk.get.short.name} creates a short name for the given object.

 \code{rk.get.description} creates descriptive string(s) for each of the arguments in "\code{\dots}"; collapsing into a single string using \code{paste.sep} (if not NULL). If \code{is.substitute=TRUE}, the arguments will be deparsed, first, which can be useful when using \code{rk.get.description} inside a function.

 \code{rk.list.names} returns the names of the arguments passed as \code{...}; when using \code{rk.list.names} inside a function, it may be necessary to increase the \code{deparse.level} level.
}

\value{
  \code{rk.set.label} returns the result of the evaluation of "setting the label" while the others return a character vector.
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\examples{
x <- data.frame(a=c(1:3), b=c(2:4))
rk.set.label(x[["a"]], "First column")
rk.get.short.name (x$a)                   # "x$a"
rk.get.label (x$a)                        # "First column"
rk.get.description (x$a)                  # "x$a (First column)"
rk.list.names (x, x$a, x$b)               # "x" "x$a" "x$b"
}

\keyword{utilities}
