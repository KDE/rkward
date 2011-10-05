\name{rk.XML.require}
\alias{rk.XML.require}
\title{Create XML "require" node for RKWard plugins}
\usage{
  rk.XML.require(file)
}
\arguments{
  \item{file}{Character string, file name of another
  .pluginmap file to be included.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a require node for .pluginmap
  files.
}
\examples{
test.require <- rk.XML.require("another.pluginmap")
cat(pasteXMLNode(test.require))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

