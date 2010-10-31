\name{rktest.replaceRunAgainLink}
\alias{rktest.replaceRunAgainLink}
\title{Replace "Run again" link in RKWard}
\usage{rktest.replaceRunAgainLink(restore=FALSE)}
\description{Replace "Run again" link in RKWard with code}
\details{You can use this to temporarily replace .rk.rerun.plugin.link (see example below).
This way, after running a plugin, you are shown the call needed to run this
plugin with those settings, instead of the link.

This code can be used in a plugin test suite.}
\alias{.rk.rerun.plugin.link.replacement}
\value{Replaces the "Run again" link in RKWard with the code that would have been called, or vice versa.}
\author{Thomas Friedrichsmeier \email{thomas.friedrichsmeier@ruhr-uni-bochum.de}, Meik Michalke \email{meik.michalke@uni-duesseldorf.de}}
\keyword{utilities}
\seealso{\code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}}
\arguments{\item{restore}{Logical: If TRUE, restore the original behaviour.}
}
\examples{rktest.replaceRunAgainLink()}

