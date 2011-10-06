\name{rk.XML.page}
\alias{rk.XML.page}
\title{Create XML "page" node for RKWard plugins}
\usage{
  rk.XML.page(nodes, id.name = NULL)
}
\arguments{
  \item{nodes}{A (list of) objects of class
  \code{XiMpLe.node}.}

  \item{id.name}{Character string, a unique ID for this
  plugin element.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  This function will create a page node for wizard
  sections, with optional child nodes "browser",
  "checkbox", "column", "copy", "dropdown", "formula",
  "frame", "input", "page", "radio", "row", "saveobject",
  "spinbox", "stretch", "tabbook", "text", "varselector"
  and "varslot".
}
\examples{
# define a checkbox for the actual dialog
test.cbox1 <- rk.XML.cbox(label="More than 30 subjects", val="true")
# define the wizard
test.text <- rk.XML.text("Did you test more than 30 subjects?")
test.copy <- rk.XML.copy(id.name=test.cbox1)
test.wizard <- rk.XML.wizard(rk.XML.page(list(test.text, test.copy)))
cat(pasteXMLNode(test.wizard))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.wizard]{rk.XML.wizard}}, and
  the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

