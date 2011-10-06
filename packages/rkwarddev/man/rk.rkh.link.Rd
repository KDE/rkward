\name{rk.rkh.link}
\alias{rk.rkh.link}
\title{Create XML "link" node for RKWard help pages}
\usage{
  rk.rkh.link(href, text = NULL, type = "R")
}
\arguments{
  \item{href}{Character string, either the URL to link to,
  name of an R package or ID of another plugin (see
  \code{type}).}

  \item{text}{Character string, optional link text.}

  \item{type}{Character string, one of the following valid
  entries: \itemize{ \item{\code{"url"}}{\code{href} is
  assumend to be the actual URL.}
  \item{\code{"R"}}{\code{href} is assumend to be the name
  of an R package, i.e., the link generated will look like
  \code{rkward://rhelp/<href>}.}
  \item{\code{"RK"}}{\code{href} is assumend to be the ID
  of another RKWard plugin, i.e., the link generated will
  look like \code{rkward://component/<href>}.} }}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML "link" node for RKWard help pages
}
\examples{
package.link <- rk.rkh.link("Spice")
cat(pasteXMLNode(package.link))
}
\seealso{
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

