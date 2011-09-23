\name{rk.XML.plugin}
\alias{rk.XML.plugin}
\title{Create XML document for RKWard plugins}
\usage{
  rk.XML.plugin(name, label, children = list(),
  wiz.children = list(), help = TRUE, logic = NULL,
  provides = c("logic", "dialog"), pluginmap = NULL)
}
\arguments{
  \item{name}{Character string, the name of the plugin.}

  \item{label}{Character string, a text label for the
  plugin's top level, i.e. the window title of the dialog.}

  \item{children}{An optional list with objects of class
  \code{XiMpLe.node}. Will be included inside the top level
  node as the dialog code.}

  \item{wiz.children}{An optional list with objects of
  class \code{XiMpLe.node}. Will be included inside the top
  level node as the wizard code (but only if
  \code{provides} includes \code{"wizard"} as well).}

  \item{help}{Logical, if \code{TRUE} an include tag for a
  help file named \emph{"<name>.rkh"} will be added to the
  header.}

  \item{logic}{Character string, will be pasted as-is
  inside the \code{<logic>} section (but only if
  \code{provides} includes \code{"logic"} as well).}

  \item{provides}{Character vector with at least one entry
  of \code{"logic"}, \code{"dialog"} or \code{"wizard"},
  defining what the document provides. If \code{"logic"} is
  specified, a logic section will be added to the document.
  If \code{logic=NULL}, must be edited manually and is
  therefore commented out.}

  \item{pluginmap}{Character string, relative path to the
  pluginmap file, which will then be included in the head
  of this document.}
}
\value{
  An object of class \code{XiMpLe.doc}.
}
\description{
  Create XML document for RKWard plugins
}
\examples{
test.checkboxes <- rk.XML.row(rk.XML.col(
list(
rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
rk.XML.cbox(label="bar", val="bar2"))))
test.dropdown <- rk.XML.dropdown("mydrop",
opts=list("First Option"=c(val="val1"),
"Second Option"=c(val="val2", chk=TRUE)))
# combine the above into a tabbook
test.tabbook <- rk.XML.tabbook("My Tabbook", tab.labels=c("First Tab",
"Second Tab"), children=list(test.checkboxes, test.dropdown))
# make a plugin with that tabbook
test.plugin <- rk.XML.plugin("My test", label="Check this out", children=test.tabbook)
cat(pasteXMLTree(test.plugin, shine=1))
}

