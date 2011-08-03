\name{rk.replace.function}
\alias{rk.replace.function}

\title{Replace a function inside its package environment / namespace}

\usage{
rk.replace.function(functionname, environment, replacement, copy.formals=TRUE)
}

\arguments{
  \item{functionname}{name of the function to be replaced (character).}
  \item{environment}{package environment or namespace, where replacment should be done.}
  \item{replacement}{the replacement. This should be a function.}
  \item{copy.formals}{logical; whether to copy the \code{\link{formals}} from the original function.}
}

\details{
  \code{rk.replace.function} can be used to replace a function inside a different package / namespace. It is mainly intended for internal usage inside rkward, e.g. to replace \code{menu} and \code{select.list} with appropriate GUI implementations.

  The original function is assigned to the environment \code{rkward::.rk.backups} with the same name as the original, and can be referred to from the replacement. WARNING: This mechansim does not support several subsequent replacments of the same function.

  WARNING: This function can be used to alter - and disrupt - internal functions in arbitrary ways. You better know what you are doing.

  WARNING: Does not work well on generics!
}

\value{
  Returns \code{NULL}, invisibly, unconditionally.
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\seealso{
  \code{\link{assignInNamespace}}, \code{\link{debug}}
}

\examples{
## Not run
rk.replace.function ("history", as.environment ("package:utils"),
  function () {
    cat ("This is what you typed:\n")
    eval (body (.rk.backups$history))
  })
## End not run
}

\keyword{utilities}
\keyword{IO}
