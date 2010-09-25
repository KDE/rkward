\name{rk.call.plugin}
\alias{rk.call.plugin}
\alias{rk.list.plugins}

\title{Call or list built-in RKWard plugin(s)}

\description{
}

\usage{
rk.call.plugin(plugin, ..., submit.mode = c("manual", "auto", "submit"))
rk.list.plugins(path, type = "all")
}

\arguments{
\item{plugin}{character string, giving the name of the plugin to call. This is of the form "\code{rkward::}\emph{plugin_name}". See Details.}
\item{\dots}{arguments passed to the \code{plugin}}
\item{submit.mode}{character string, specifying the submission mode: \code{"manual"} will open the plugin GUI and wait for the user to submit it manually, \code{"submit"} will submit the plugin to the output, or \bold{TODO: _check me_} \code{"auto"} will use the built-in default of the plugin (which in most cases will be same as \code{"submit"}).}
\item{path}{character string, giving the directory to search for various pluginmap files. This is the directory where the KDE application, "RKWard," is installed (and not the R package, "rkward"). For example, on most Debian based Linux systems this is generally \code{/usr/share/kde4/apps/rkward} or \code{~/.kde/share/apps/rkward}.}
\item{type}{character string, giving the specific pluginmap file to scan. Currently suported strings are \code{"all"}, \code{"analysis"}, \code{"data"}, \code{"distributions"}, \code{"import_export"}, \code{"irt"}, \code{"plots"}, and \code{"x11device"}. The default value of \code{"all"} scans all pluginmap files.}
}

\details{
\code{rk.call.plugin} provides a high level wrapper to call any plugin available in RKWard. The exact string to be used as \code{plugin} is generally not transparent to the user. Use \code{rk.list.plugins} to list these. A particular character string (\emph{plugin_name}) thus obtained needs to be prefixed by \code{"rkward::"} for a valid \code{plugin} argument. See example.

\code{rk.list.plugins} scans the given pluginmap file and lists all the available plugins under it. 
}

\value{
\code{rk.call.plugin} returns \code{TRUE} invisibly.

\code{rk.list.plugins} returns a character vector of plugin names. If none found, \code{NULL} is returned. 
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\seealso{
\code{\link{rk.results}}, \url{rkward://page/rkward_output} 
}

\examples{
## list all analysis related plugins
rk.list.plugins ("~/.kde/share/apps/rkward", "analysis")

## "t_test_two_vars" plugin:
## see the output: Windows->Show Output
local({
x1 <- rnorm (100)
x2 <- rnorm (100, 2)

rk.call.plugin ("rkward::t_test_two_vars", 
  confint.state="1", conflevel.real="0.95", hypothesis.string="greater", paired.state="0", varequal.state="0", 
  x.available="x1", y.available="x2", 
  submit.mode="submit")
})
}

\keyword{utilities}
