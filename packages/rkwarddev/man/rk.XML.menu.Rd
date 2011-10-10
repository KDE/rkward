\name{rk.XML.menu}
\alias{rk.XML.menu}
\title{Create XML "menu" node for RKWard plugins}
\usage{
  rk.XML.menu(label, ..., index = -1, id.name = "auto")
}
\arguments{
  \item{label}{Character string, a label for the menu.}

  \item{...}{Objects of class \code{XiMpLe.node}, must be
  either "menu" or "entry".}

  \item{index}{Integer number to influence the level of
  menu placement.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"}, an ID will be generated
  automatically from the label. Used to place the menu in
  the global menu hierarchy.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  This function will create a menu node for hierarchy
  sections. Use same \code{id} values to place entries in
  the same menu.
}
\examples{
test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
test.entry <- rk.XML.entry(test.component)
test.menu <- rk.XML.menu("Analysis", test.entry, id.name="analysis")
cat(pasteXMLNode(test.menu))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.hierarchy]{rk.XML.hierarchy}},
  \code{\link[rkwarddev:rk.XML.entry]{rk.XML.entry}},
  \code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
  \code{\link[rkwarddev:rk.XML.components]{rk.XML.components}},
  and the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

