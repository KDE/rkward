\name{rk.capture.device}
\alias{rk.capture.device}
\title{Capture / embed non-RKWard device windows}
\usage{
  rk.capture.device(expr)
}
\arguments{
  \item{expr}{Expression to evaluate.}
}
\description{
  \code{rk.capture.device} evaluates the given expression,
  and if this has created a window on the screen, tries to
  embed it as an RKWard window.
}
\details{
  Theoretically, \code{expr} can be any valid R expression.
  However typically this should be calls to X11(),
  Windows(), or, perhaps dev.copy().  Importantly, the
  expression should create exactly one new window for
  \code{rk.capture.device()} to work. Keep in mind, that
  this is not always the case for \code{plot(...)} and
  similar commands, which will re-use an existing plot
  window, if available.
}
\note{
  \code{rk.capture.device()} will not work on all platforms
  (most importantly, not in most MacOSX binaries). Further,
  note that a captured \code{X11()} or \code{Windows}
  device may look similar to an \code{RK()} device, but is
  actually a very different thing.
}
\seealso{
  \link{RK}
}

