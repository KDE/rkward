\name{rk.XML.insert}
\alias{rk.XML.insert}
\title{Create XML node "insert" for RKWard plugins}
\usage{
  rk.XML.insert(snippet)
}
\arguments{
  \item{snippet}{Either a character string (the \code{id}
  of the snippet to be inserted), or an object of class
  \code{XiMpLe.node} (whose \code{id} will be extracted and
  used; must be a snippet!).}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function creates an insert node to use snippets.
}
\examples{
# define a formula section with varselector and varslots
test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
# define the snippet
test.snippet <- rk.XML.snippet(test.formula)
# now to insert the snippet
test.insert <- rk.XML.insert(test.snippet)
cat(pasteXMLNode(test.insert))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.snippets]{rk.XML.snippets}},
  \code{\link[rkwarddev:rk.XML.snippet]{rk.XML.snippet}},
  and the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

