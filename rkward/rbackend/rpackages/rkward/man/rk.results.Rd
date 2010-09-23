\name{rk.results}
\alias{rk.print}
\alias{rk.print.literal}
\alias{rk.header}
\alias{rk.results}
\alias{rk.describe.alternative}

\title{Print objects and results to output}

\description{
  Various utilty functions which can be used to print or export R objects to the (html) output file. The output file can be accessed from Windows -> Show Output. Basically, these functions along with the ones described in \code{\link{rk.get.label}}, \code{\link{rk.get.tempfile.name}}, and \code{\link{rk.graph.on}} can be used to create a HTML report.
}

\usage{
rk.print(x, ...)
rk.print.literal(x)
rk.header(title, parameters = list(), level = 1)
rk.results(x, titles = NULL)
rk.describe.alternative(x)
}

\arguments{
\item{x}{any R object to be printed/exported. A suitable list in case of \code{rk.describe.alternative}.}
\item{title}{a string, used as a header for the html output}
\item{level}{an integer, header level. For example, \code{level=2} creates the header with \code{<h2></h>} tag.}
\item{parameters}{a list, preferably named, giving a list of "parameters" to be printed to the output}
\item{titles}{a character vector, giving the column headers for a html table.}
}

\details{
\code{rk.print} prints/exports the given object to the output (html) file using the \code{\link{HTML}} function. This requires the \code{R2HTML} package. Additional arguments in \code{...} are passed on to \code{\link{HTML}}.

\code{rk.print.literal} prints/exports the given object using a \code{paste(x, collapse="\n")} construct to the output (html) file.

\code{rk.header} prints a html header, possibly with parameters, to the output file. See example.

\code{rk.results} is similar to \code{rk.print} but prints in a more tabulated fashion. This has been implemented only for certain types of \code{x}: tables, lists (or data.frames), and vectors. See example.

\code{rk.describe.alternatives} describes the alternative (H1) hypothesis of a \code{htest}. This is similar to \code{stats:::print.htext} and makes sense only when \code{x$alternatives} exists.
}

\value{
\code{rk.describe.alternatives} returns a string while all other functions return \code{NULL}, invisibly.
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\seealso{
\code{\link{HTML}}, \code{\link{rk.get.output.html.file}}, \code{\link{rk.get.description}}, \url{rkward://page/rkward_output}
}

\examples{
require (rkward)
require (R2HTML)

## see the output: Windows->Show Output
## stolen from the two-sample t-test plugin ;)
local({
x1 <- rnorm (100)
x2 <- rnorm (100, 2)
nm <- rk.get.description (x1,x2)

result <- t.test (x1, x2, alternative="less")

rk.header (result$method,
  parameters=list ("Comparing", paste (nm[1], "against", nm[2]),
  "H1", rk.describe.alternative (result),
  "Equal variances", "not assumed"))

rk.print.literal ("Raw data (first few rows):")
rk.print (head (cbind (x1,x2)), align = "left")

rk.print.literal ("Test results:")
rk.results (list (
  'Variable Name'=nm,
  'estimated mean'=result$estimate,
  'degrees of freedom'=result$parameter,
  t=result$statistic,
  p=result$p.value,
  'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
  'confidence interval of difference'=result$conf.int ))
})
}

\keyword{utilities}
