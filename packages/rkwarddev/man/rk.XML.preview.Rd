\name{rk.XML.preview}
\alias{rk.XML.preview}
\title{Create XML node "preview" for RKWard plugins}
\usage{
  rk.XML.preview(label = "Preview")
}
\arguments{
  \item{label}{A character string, text label for the
  preview checkbox.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  Create XML node "preview" for RKWard plugins
}
\examples{
test.preview <- rk.XML.preview("See a preview?")
cat(pasteXMLNode(test.preview))
}

