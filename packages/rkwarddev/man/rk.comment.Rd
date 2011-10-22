\name{rk.comment}
\alias{rk.comment}
\title{Create comment for RKWard plugin code}
\usage{
  rk.comment(text)
}
\arguments{
  \item{text}{Character string, the text to be displayed.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create comment for RKWard plugin code
}
\examples{
test.comment <- rk.comment("Added this text.")
cat(pasteXMLNode(test.comment))
}

