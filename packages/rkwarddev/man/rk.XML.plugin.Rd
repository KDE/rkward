\name{rk.XML.plugin}
\alias{rk.XML.plugin}
\title{Create XML document for RKWard plugins}
\usage{
  rk.XML.plugin(name, dialog = NULL, wizard = NULL, logic =
  NULL, snippets = NULL, provides = NULL, help = TRUE,
  pluginmap = NULL, label = NULL, clean.name = TRUE, about
  = NULL)
}
\arguments{
  \item{name}{Character string, the name of the plugin.
  Will be used for the names of the JavaScript and help
  files to be included, following the scheme
  \emph{"<name>.<ext>"}.}

  \item{dialog}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<dialog>} section. See
  \code{\link[rkwarddev:rk.XML.dialog]{rk.XML.dialog}} for
  details.}

  \item{wizard}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<wizard>} section. See
  \code{\link[rkwarddev:rk.XML.wizard]{rk.XML.wizard}} for
  details.}

  \item{logic}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<logic>} section. See
  \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}} for
  details.}

  \item{snippets}{An object of class \code{XiMpLe.node} to
  be pasted as the \code{<snippets>} section. See
  \code{\link[rkwarddev:rk.XML.snippets]{rk.XML.snippets}}
  for details.}

  \item{provides}{Character vector with possible entries of
  \code{"logic"}, \code{"dialog"} or \code{"wizard"},
  defining what sections the document should provide even
  if \code{dialog}, \code{wizard} and \code{logic} are
  \code{NULL}. These sections must be edited manually and
  some parts are therefore commented out.}

  \item{help}{Logical, if \code{TRUE} an include tag for a
  help file named \emph{"<name>.rkh"} will be added to the
  header.}

  \item{pluginmap}{Character string, relative path to the
  pluginmap file, which will then be included in the head
  of this document.}

  \item{label}{Character string, a text label for the
  plugin's top level, i.e. the window title of the dialog.
  Will only be used if \code{dialog} or \code{wizard} are
  \code{NULL}.}

  \item{clean.name}{Logical, if \code{TRUE}, all
  non-alphanumeric characters except the underscore
  (\code{"_"}) will be removed from \code{name}.}

  \item{about}{An object of class \code{XiMpLe.node} with
  descriptive information on the plugin, its authors and
  dependencies, see
  \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for
  details. Only useful for information that differs from
  the \code{<about>} section of the \code{.pluginmap} file.
  Skipped if \code{NULL}.}
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
test.plugin <- rk.XML.plugin("My test", dialog=test.tabbook)
cat(pasteXMLTree(test.plugin))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

