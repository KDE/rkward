\name{RKWard_misc}
\alias{rk.misc}
\alias{rk.rename.in.container}
\alias{rk.make.repos.string}
\alias{rk.select.CRAN.mirror}

\title{Miscellaneous utility functions}

\description{
}

\usage{
rk.rename.in.container(x, old_name, new_name, envir = parent.frame())
rk.make.repos.string()
rk.select.CRAN.mirror()
}

\arguments{
\item{x}{a data.frame or list.}
\item{old_name}{a string, the name of the column or element to be renamed.}
\item{new_name}{a string, the new name.}
\item{envir}{an environment where \code{x} is available.}
}

\details{
\code{rk.rename.in.container} renames a named object (column/element) in a data.frame/list without changing its position.

\code{rk.make.repos.string} just creates a R statement for \code{repos}. A typical user should not need to use this function.

\code{rk.select.CRAN.mirror} is an in-house replacement for \code{\link{chooseCRANmirror}} without changing \code{options ("repos")}, permanently. It uses native KDE gui and provides more information on each mirror.
}

\value{
\code{rk.rename.in.container} returns \code{NULL} on successfule renaming, otherwise an error.

\code{rk.make.repos.string} returns a valid R expression as a character string which can then be parsed and evaluated.

\code{rk.select.CRAN.mirror} returns the URL of the chosen mirror, as a string.
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\examples{
## rk.rename.in.container
ir <- iris
str (ir)
rk.rename.in.container(ir, "Species", "Taxonomic.Group")
str (ir)
}

\keyword{attribute}
\keyword{misc}
\keyword{utilities}
