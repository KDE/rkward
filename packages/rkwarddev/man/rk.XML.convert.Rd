\name{rk.XML.convert}
\alias{rk.XML.convert}
\title{Create XML node convert for RKWard plugins}
\usage{
  rk.XML.convert(sources, mode = c(), required = FALSE,
  id.name = "auto")
}
\arguments{
  \item{sources}{A list with at least one value, either
  resembling the \code{id} of an existing element to be
  queried as a character string, or a previously defined
  object of class \code{XiMpLe.node} (whose \code{id} will
  be extracted and used). If you want to examine e.g. the
  state or string value specificly, just name the value
  accoringly, e.g., \code{sources=list("vars0",
  string="input1", state="chkbx2")}.}

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
  \code{required_true} attribute. If \code{TRUE}, the
  plugin submit button is only enabled if this property is
  true.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"}, an ID will be generated
  automatically from the \code{sources} and \code{mode}
  value.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  The recognized property names for \code{sources} are the
  following: \code{string}, \code{state}, \code{text},
  \code{selected}, \code{root}, \code{available},
  \code{source}, \code{number}, \code{enabled},
  \code{checked}, \code{selection}, \code{parent},
  \code{objectname}, \code{active}, \code{int},
  \code{real}, \code{model}, \code{table}, \code{labels},
  \code{fixed_factors}, \code{dependent} and \code{code}.
  They are not globally valid for all XML elements, see the
  section on "Properties of plugin elements" to see which
  is useful for what tag. If \code{sources} holds
  \code{XiMpLe.node} objects, the validity of properties is
  automatically checked for that tag.
}
\examples{
test.convert <- rk.XML.convert(list(string="foo"), mode=c(notequals="bar"))
cat(pasteXMLNode(test.convert))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
  \code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
  \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}},
  \code{\link[rkwarddev:rk.XML.set]{rk.XML.set}}, and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

