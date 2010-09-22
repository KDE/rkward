\name{rk.edit}
\alias{rk.edit}
\alias{rk.edit.files}
\alias{rk.show.files}
\alias{rk.show.html}

\title{Edit / show an object / file}

\usage{
rk.edit(x)
rk.edit.files(file = file, title = file, name = NULL)
rk.show.files(file = file, title = file, wtitle = NULL, delete = FALSE)
rk.show.html(url)
}

\arguments{
  \item{x}{an object to edit.}
  \item{file}{character vector, filenames to show or edit.}
  \item{title}{character vector, of the same length as \code{file}; This can be used to give descriptive titles to each file, which will be displayed to the user.}
  \item{wtitle}{character vector, of length 1. This will be used as the window title.}
  \item{delete}{a logical (not NA), when \code{TRUE} the shown file(s) are deleted after closing.}
}

\details{
  \code{rk.edit} can be used to edit an object in the RKWard data editor. Currently only \link{data.frame}s are supported. This is similar to \link{edit.data.frame}, but the function returns immediately, and the object is edit asynchronously.

  \code{rk.edit.files}, \code{rk.show.files}, and \code{rk.show.html} are equivalent to \link{file.edit}, \link{file.show}, and \link{browseURL}, respectively, but use RKWard as text/html editor/viewer. Generally it is recommended to use \link{file.edit}, \link{file.show}, and \link{browseURL}, instead. These will call the respective RKWard functions by default, when run inside an RKWard session.
}

\value{
  All functions described on this page return \code{NULL}, unconditionally.
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\seealso{
  \code{\link{edit}}, \code{\link{file.edit}}, \code{\link{file.show}}, \code{\link{browseURL}}
}

\examples{
## Not run
x <- data.frame (a=c(1:3), b=c(2:4))
rk.edit(x)
}

\keyword{utilities}
\keyword{IO}
