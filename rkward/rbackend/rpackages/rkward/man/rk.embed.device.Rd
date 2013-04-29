\name{rk.embed.device}
\alias{rk.embed.device}
\title{Embed non-RKWard device windows}
\usage{
  rk.embed.device(expr)
}
\arguments{
  \item{expr}{Expression to evaluate.}
}
\description{
  \code{rk.embed.device} evaluates the given expression,
  and if this has created a window on the screen, tries to
  embed it as an RKWard window.
}
\details{
  Theoretically, \code{expr} can be any valid R expression.
  However typically this should be calls to X11(),
  Windows(), or, perhaps dev.copy().  Importantly, the
  expression should create exactly one new window for
  \code{rk.embed.device()} to work. Keep in mind, that this
  is not always the case for \code{plot(...)} and similar
  commands, which will re-use an existing plot window, if
  available.
}
\note{
  \code{rk.embed.device()} will not work on all platforms
  (most importantly, not in most MacOSX binaries). Further,
  note that a captured \code{X11()} or \code{Windows}
  device may look similar to an \code{RK()} device, but is
  actually a very different thing.
}
\seealso{
  \link{RK}
}

