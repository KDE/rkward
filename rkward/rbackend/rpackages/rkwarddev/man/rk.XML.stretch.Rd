\name{rk.XML.stretch}
\alias{rk.XML.stretch}
\title{Create XML empty node "stretch" for RKWard plugins}
\usage{
  rk.XML.stretch(before = NULL, after = NULL)
}
\arguments{
  \item{before}{A list of objects of class
  \code{XiMpLe.node}.}

  \item{after}{A list of objects of class
  \code{XiMpLe.node}.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  The simplest way to use \code{rk.XML.stretch} is to call
  it without arguments. If you provide \code{before} and/or
  \code{after}, a "<stretch />" will be put between the XML
  elements defined there.
}
\examples{
cat(pasteXMLNode(rk.XML.stretch()))
}

