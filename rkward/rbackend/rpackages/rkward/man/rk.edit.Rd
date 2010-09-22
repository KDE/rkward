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
  \item{x}{an object}
  \item{file}{character vector, filenames to edit. \bold{TODO} _check me_}
  \item{title}{character vector, of the same length as \code{file}; used as ??? \bold{TODO} _complete me_}
  \item{wtitle}{character vector, of the same length as \code{file}; used as the title strings of the displayed windows \bold{TODO} _check me_}
  \item{delete}{a logical (not NA), when \code{TRUE} the shown file(s) are deleted after closing.}
}

\details{
}

\value{
  All the functions return their corresponding internal call values. Except \code{rk.edit}, all other functions return invisibly. \bold{TODO} __fix me__
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\seealso{
  \code{\link{edit}}, \code{\link{file}}
}

\examples{
\bold{TODO}
}

\keyword{utilities}
\keyword{IO}
