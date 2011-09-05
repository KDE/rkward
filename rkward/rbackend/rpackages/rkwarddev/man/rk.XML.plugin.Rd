\name{rk.XML.plugin}
\alias{rk.XML.plugin}
\title{Create XML document for RKWard plugins}
\usage{
  rk.XML.plugin(name, label, children = list(), help =
  TRUE, logic = TRUE, provides = c("dialog"), pluginmap =
  NULL)
}
\arguments{
  \item{name}{Character string, the name of the plugin.}

  \item{label}{Character string, a text label for the
  plugin's top level, i.e. the window title of the dialog.}

  \item{children}{An optional list with objects of class
  \code{XiMpLe.node}. Will be included inside the top level
  node.}

  \item{help}{Logical, if \code{TRUE} an include tag for a
  help file named \emph{"<name>.rkh"} will be added to the
  header.}

  \item{logic}{Logical, if \code{TRUE} a logic section will
  be added to the document. This must be edited manually,
  it is therefore commented out.}

  \item{provides}{Character vector with at least one entry
  of \code{"dialog"} or \code{"wizard"}, defining what the
  document provides.}

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

