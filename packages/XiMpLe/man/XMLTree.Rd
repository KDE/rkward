\name{XMLTree}
\alias{XMLTree}
\alias{is.XiMpLe.doc}
\title{Constructor function for XiMpLe.doc objects}
\usage{
XMLTree(..., xml = NULL, dtd = NULL, .children = list(...))

is.XiMpLe.doc(x)
}
\arguments{
  \item{...}{Optional children for the XML tree. Must be
  either objects of class
  \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}} or
  character strings, which are treated as simple text
  values.}

  \item{xml}{A named list, XML declaration of the XML tree.
  Currently just pasted, no checking is done.}

  \item{dtd}{A named list, doctype definition of the XML
  tree. Valid elements are \code{doctype} (root element),
  \code{decl} ("PUBLIC" or "SYSTEM"), \code{id} (the
  identifier) and \code{refer} (URI to .dtd).  Currently
  just pasted, no checking is done.}

  \item{.children}{Alternative way of specifying children,
  if you have them already as a list.}

  \item{x}{An arbitrary \code{R} object.}
}
\value{
An object of class
\code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
}
\description{
Can be used to create full XML trees.
}
\examples{
sample.XML.a <- XMLNode("a",
  attrs=list(href="http://example.com", target="_blank"),
  .children="klick here!")
sample.XML.body <- XMLNode("body", .children=list(sample.XML.a))
sample.XML.html <- XMLNode("html", .children=list(XMLNode("head", ""),
  sample.XML.body))
sample.XML.tree <- XMLTree(sample.XML.html,
  xml=list(version="1.0", encoding="UTF-8"),
  dtd=list(doctype="html", decl="PUBLIC",
    id="-//W3C//DTD XHTML 1.0 Transitional//EN",
    refer="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"))
}
\seealso{
\code{\link[XiMpLe:XMLNode]{XMLNode}},
\code{\link[XiMpLe:pasteXML]{pasteXML}}
}

