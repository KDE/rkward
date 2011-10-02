\name{rk.XML.context}
\alias{rk.XML.context}
\title{Create XML "context" node for RKWard plugins}
\usage{
  rk.XML.context(nodes, id = "x11")
}
\arguments{
  \item{nodes}{A (list of) objects of class
  \code{XiMpLe.node}, must all be "menu".}

  \item{id}{Character string, either "x11" or "import".}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a context node for .pluginmap
  files, with mandatory child nodes "menu".
}
\examples{
test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
test.entry <- rk.XML.entry(test.component)
test.menu <- rk.XML.menu("Analysis", nodes=test.entry, id.name="analysis")
test.context <- rk.XML.context(test.menu)
cat(pasteXMLNode(test.context))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.menu]{rk.XML.menu}},
  \code{\link[rkwarddev:rk.XML.entry]{rk.XML.entry}},
  \code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
}

