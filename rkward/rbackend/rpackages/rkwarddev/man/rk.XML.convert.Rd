\name{rk.XML.convert}
\alias{rk.XML.convert}
\title{Create XML node convert for RKWard plugins}
\usage{
  rk.XML.convert(sources, mode = c(), required = FALSE,
  id.name = "auto")
}
\arguments{
  \item{sources}{A character vector with at least one
  value, resembling the \code{id} of an existing element to
  be queried. If you want to examine the state or string
  value specificly, just name the value accoringly, e.g.,
  \code{sources=c("vars0", string="input1",
  state="chkbx2")}.}

  \item{mode}{A named vector with either exactly one of the
  following elements: \itemize{ \item{\code{equals}}{True
  if \code{sources} equals this value.}
  \item{\code{notequals}}{True if \code{sources} differs
  from this value.} \item{\code{and}}{True if all
  \code{sources} are true. The sources must be boolean, and
  the actual value here is irrelevant, so
  \code{mode=c(and="")} is valid.} \item{\code{or}}{True if
  any of the \code{sources} is true.  The sources must be
  boolean, and the actual value here is irrelevant, so
  \code{mode=c(or="")} is valid.} } or at least one of
  these elemets: \itemize{ \item{\code{min}}{True if
  \code{sources} is at least this value. They must be
  numeric.} \item{\code{max}}{True if \code{sources} is
  below this value. They must be numeric.} }}

  \item{required}{Logical, sets the state of the
  \code{required\_true} attribute. If \code{TRUE}, the
  plugin submit button is only enabled if this property is
  true.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"}, an ID will be generated
  automatically from the sources and mode value.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  Create XML node convert for RKWard plugins
}
\examples{
test.convert <- rk.XML.convert(c(string="foo"), mode=c(notequals="bar"))
cat(pasteXMLNode(test.convert, shine=1))
}

