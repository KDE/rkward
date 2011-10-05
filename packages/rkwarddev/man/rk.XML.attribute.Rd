\name{rk.XML.attribute}
\alias{rk.XML.attribute}
\title{Create XML "attribute" node for RKWard plugins}
\usage{
  rk.XML.attribute(id, value = NULL, label = NULL)
}
\arguments{
  \item{id}{Either a character string (the \code{id} of the
  property whose attribute should be set), or an object of
  class \code{XiMpLe.node} (whose \code{id} will be
  extracted and used).}

  \item{value}{Character string, new value for the
  attribute.}

  \item{label}{Character string, label associated with the
  attribute.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a attribute node for component
  sections in .pluginmap files. Only meaningful for import
  plugins.
}
\examples{
# define a formula section with varselector and varslots
test.checkbox <- rk.XML.cbox(label="foo", value="foo1", chk=TRUE)
# re-set the attribute
test.attribute <- rk.XML.attribute(test.checkbox, value="bar2", label="bar")
cat(pasteXMLNode(test.attribute))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.components]{rk.XML.components}},
  and the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

