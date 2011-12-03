\name{rk.XML.input}
\alias{rk.XML.input}
\title{Create XML node "input" for RKWard plugins}
\usage{
  rk.XML.input(label, initial = NULL, size = "medium",
    required = FALSE, id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for this
  plugin element.}

  \item{initial}{Character string, if not \code{NULL} will
  be used as the initial value of the input field.}

  \item{size}{One value of either "small", "medium" or
  "large".}

  \item{required}{Logical, whether an entry is mandatory or
  not.}

  \item{id.name}{Character string, a unique ID for this
  plugin element.  If \code{"auto"}, an ID will be
  generated automatically from the label.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "input" for RKWard plugins
}
\examples{
test.input <- rk.XML.input("Type some text")
cat(pasteXMLNode(test.input))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

