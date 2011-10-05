\name{rk.XML.set}
\alias{rk.XML.set}
\title{Create XML node "set" for RKWard plugins}
\usage{
  rk.XML.set(id, set = NULL, to)
}
\arguments{
  \item{id}{Either a character string (the \code{id} of the
  property whose value should be set), or an object of
  class \code{XiMpLe.node} (whose \code{id} will be
  extracted and used).}

  \item{set}{Character string, can be one of the following
  values: \itemize{ \item{\code{"enabled"}}{Set the
  \code{id} enabled state.} \item{\code{"visible"}}{Set the
  \code{id} visibility state.} \item{\code{"required"}}{Set
  the \code{id} requirement state.} }}

  \item{to}{Character string or logical, the value the
  property should be set to.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  Create XML node "set" for RKWard plugins
}
\examples{
test.set <- rk.XML.set(id="input_foo", set="required", to=TRUE)
cat(pasteXMLNode(test.set))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
  \code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
  \code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
  \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}}, and
  the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

