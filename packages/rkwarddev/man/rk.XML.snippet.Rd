\name{rk.XML.snippet}
\alias{rk.XML.snippet}
\title{Create XML "snippet" node for RKWard plugins}
\usage{
  rk.XML.snippet(nodes, id.name = "auto")
}
\arguments{
  \item{nodes}{A (list of) objects of class
  \code{XiMpLe.node}.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"}, an ID will be generated
  automatically from the tag names and IDs of the given
  nodes.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a snippet node for snippets
  sections.
}
\examples{
# define a formula section with varselector and varslots
test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
# define the snippet
test.snippet <- rk.XML.snippet(test.formula)
cat(pasteXMLNode(test.snippet))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.snippets]{rk.XML.snippets}}
}

