\name{rk.XML.wizard}
\alias{rk.XML.wizard}
\title{Create XML wizard section for RKWard plugins}
\usage{
  rk.XML.wizard(nodes, label = NULL)
}
\arguments{
  \item{nodes}{A (list of) objects of class
  \code{XiMpLe.node}}

  \item{label}{Character string, a text label for this
  plugin element.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  This function will create a wizard section with optional
  child nodes "browser", "checkbox", "column", "copy",
  "dropdown", "embed", "formula", "frame", "include",
  "input", "insert", "page", "preview", "radio", "row",
  "saveobject", "spinbox", "stretch", "tabbook", "text",
  "varselector" and "varslot".
}
\examples{
# define a checkbox for the actual dialog
test.cbox1 <- rk.XML.cbox(label="More than 30 subjects", val="true")
# define the wizard
test.text <- rk.XML.text("Did you test more than 30 subjects?")
test.copy <- rk.XML.copy(id=test.cbox1)
test.wizard <- rk.XML.wizard(rk.XML.page(list(test.text, test.copy)))
cat(pasteXMLNode(test.wizard))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}},
  \code{\link[rkwarddev:rk.plugin.skeleton]{rk.plugin.skeleton}},
  and the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

