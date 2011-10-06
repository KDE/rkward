\name{rk.XML.connect}
\alias{rk.XML.connect}
\title{Create XML node "connect" for RKWard plugins}
\usage{
  rk.XML.connect(governor, client, set = "enabled", not =
  FALSE, reconcile = FALSE)
}
\arguments{
  \item{governor}{Either a character string (the \code{id}
  of the property whose state should control the
  \code{client}), or an object of class \code{XiMpLe.node}
  (whose \code{id} will be extracted and used). Usually a
  \code{<convert>} node defined earlier (see
  \code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}}),
  or the ".state" value of some apropriate node.}

  \item{client}{Character string, the \code{id} if the
  element to be controlled by \code{governor}.}

  \item{not}{Logical, if \code{TRUE}, the state of
  \code{governor} (\code{TRUE/FALSE}) will be inversed.}

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
  An object of class \code{XiMpLe.node}.
}
\description{
  If you define a \code{XiMpLe.node} object as
  \code{governor} which is not a \code{<convert>} node and
  \code{not=FALSE}, the function will automatically append
  ".state" to its \code{id}.
}
\examples{
test.connect <- rk.XML.connect(governor="lgc_foobar", client="frame_bar")
cat(pasteXMLNode(test.connect))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
  \code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
  \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}}
  \code{\link[rkwarddev:rk.XML.set]{rk.XML.set}}, and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

