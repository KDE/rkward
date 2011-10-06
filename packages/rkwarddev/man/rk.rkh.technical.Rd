\name{rk.rkh.technical}
\alias{rk.rkh.technical}
\title{Create XML "technical" node for RKWard help pages}
\usage{
  rk.rkh.technical(text = NULL)
}
\arguments{
  \item{text}{Character string, the text to be displayed.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML "technical" node for RKWard help pages
}
\examples{
plugin.technical <- rk.rkh.technical("<b>TODO</b>: Implement sandworm detector.")
cat(pasteXMLNode(plugin.technical))
}
\seealso{
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

