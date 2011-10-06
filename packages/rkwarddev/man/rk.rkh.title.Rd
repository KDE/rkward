\name{rk.rkh.title}
\alias{rk.rkh.title}
\title{Create XML "title" node for RKWard help pages}
\usage{
  rk.rkh.title(text = NULL)
}
\arguments{
  \item{text}{Character string, the text to be displayed.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML "title" node for RKWard help pages
}
\examples{
plugin.title <- rk.rkh.title("Spice")
cat(pasteXMLNode(plugin.title))
}
\seealso{
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

