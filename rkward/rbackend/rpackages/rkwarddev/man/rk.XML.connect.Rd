\name{rk.XML.connect}
\alias{rk.XML.connect}
\title{Create XML node "connect" for RKWard plugins}
\usage{
  rk.XML.connect(governor, client, set = "enabled",
  reconcile = FALSE)
}
\arguments{
  \item{governor}{Character string, the \code{id} if the
  property whose state should control the \code{client}.
  Usually a \code{<convert>} node defined earlier (see
  \code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}}),
  or the ".state" value of some apropriate node.}

  \item{client}{Character string, the \code{id} if the
  element to be controlled by \code{governor}.}

  \item{set}{Character string, one of the following values:
  \itemize{ \item{\code{"enabled"}}{If \code{governor}
  becomes true, \code{client} is enabled.}
  \item{\code{"visible"}}{If \code{governor} becomes true,
  \code{client} is visible.} \item{\code{"required"}}{If
  \code{governor} becomes true, \code{client} is required.}
  }}

  \item{reconcile}{Logical, forces the \code{governor} to
  only accept values which are valid for the \code{client}
  as well.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  Create XML node "connect" for RKWard plugins
}
\examples{
test.connect <- rk.XML.connect(governor="lgc_foobar", client="frame_bar")
cat(pasteXMLNode(test.connect, shine=1))
}

