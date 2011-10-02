\name{rk.XML.include}
\alias{rk.XML.include}
\title{Create XML node "include" for RKWard plugins}
\usage{
  rk.XML.include(file)
}
\arguments{
  \item{file}{A character string, the file name to be
  included.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  Create XML node "include" for RKWard plugins
}
\examples{
test.include <- rk.XML.include("../some_file.xml")
cat(pasteXMLNode(test.include))
}

