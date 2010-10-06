\name{rk.rerun.plugin.link.replacement}
\alias{.rk.rerun.plugin.link.replacement}
\title{Replace "Run again" link in RKWard}
\usage{.rk.rerun.plugin.link <- .rk.rerun.plugin.link.replacement}
\description{Replace "Run again" link in RKWard with code}
\details{You can use this to temporarily replace .rk.rerun.plugin.link (see example below).
This way, after running a plugin, you are shown the call needed to run this
plugin with those settings, instead of the link.

This code can be used in a plugin test suite.}
\alias{.rk.rerun.plugin.link.replacement}
\value{Replaces the "Run again" link in RKWard with the code that would have been called.}
\docType{function}
\author{Thomas Friedrichsmeier \email{thomas.friedrichsmeier@ruhr-uni-bochum.de}}
\keyword{utilities}
\seealso{\code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}}
\arguments{\item{plugin}{(used internally)}
\item{settings}{(used internally)}
\item{label}{(used internally)}
}
\examples{\dontrun{
# NOTE: Do NOT end the function with brackets, as its code has to be
# written into .rk.rerun.plugin.link
.rk.rerun.plugin.link <- .rk.rerun.plugin.link.replacement
}}

