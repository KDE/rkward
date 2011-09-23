\name{rk.XML.formula}
\alias{rk.XML.formula}
\title{Create XML node "formula" for RKWard plugins}
\usage{
  rk.XML.formula(fixed, dependent)
}
\arguments{
  \item{fixed}{The \code{id} of the varslot holding the
  selected fixed factors.}

  \item{dependent}{The \code{id} of the varslot holding the
  selected dependent variable.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "formula" for RKWard plugins
}
\examples{
test.formula <- rk.XML.formula(fixed="vrs.one", dependent="vsr.two")
cat(pasteXMLNode(test.formula, shine=1))
}

