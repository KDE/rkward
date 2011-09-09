\name{rk.XML.text}
\alias{rk.XML.text}
\title{Create XML node "text" for RKWard plugins}
\usage{
  rk.XML.text(text, id.name = NULL)
}
\arguments{
  \item{id.name}{Character string, a unique ID for this
  plugin element.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "text" for RKWard plugins
}
\examples{
test.text <- rk.XML.text("Added this text.")
cat(pasteXMLNode(test.text, shine=1))
}

