\name{rk.XML.plugin}
\alias{rk.XML.plugin}
\title{Create XML document for RKWard plugins}
\usage{
  rk.XML.plugin(name, label, dialog = NULL, wizard = NULL,
  logic = NULL, provides = c("logic", "dialog"), help =
  TRUE, pluginmap = NULL)
}
\arguments{
  \item{name}{Character string, the name of the plugin.}

  \item{label}{Character string, a text label for the
  plugin's top level, i.e. the window title of the dialog.}

  \item{dialog}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<dialog>} section (but only if
  \code{provides} includes \code{"dialog"} as well). See
  \code{\link[rkwarddev:rk.XML.dialog]{rk.XML.dialog}} for
  details.}

  \item{wizard}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<wizard>} section (but only if
  \code{provides} includes \code{"wizard"} as well). See
  \code{\link[rkwarddev:rk.XML.wizard]{rk.XML.wizard}} for
  details.}

  \item{logic}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<logic>} section (but only if
  \code{provides} includes \code{"logic"} as well). See
  \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}} for
  details.}

  \item{provides}{Character vector with at least one entry
  of \code{"logic"}, \code{"dialog"} or \code{"wizard"},
  defining what the document provides. If \code{"logic"} is
  specified, a logic section will be added to the document.
  If \code{logic=NULL}, must be edited manually and is
  therefore commented out.}

  \item{help}{Logical, if \code{TRUE} an include tag for a
  help file named \emph{"<name>.rkh"} will be added to the
  header.}

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
"Second Tab"), dialog=list(test.checkboxes, test.dropdown))
# make a plugin with that tabbook
test.plugin <- rk.XML.plugin("My test", label="Check this out", dialog=test.tabbook)
cat(pasteXMLTree(test.plugin, shine=1))
}

