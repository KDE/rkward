\name{rk.XML.pluginmap}
\alias{rk.XML.pluginmap}
\title{Write a pluginmap file for RKWard}
\usage{
  rk.XML.pluginmap(name, about = NULL, components,
    hierarchy = "test", require = NULL, x11.context = NULL,
    import.context = NULL, clean.name = TRUE,
    hints = FALSE, gen.info = TRUE, dependencies = NULL,
    namespace = name, priority = "medium")
}
\arguments{
  \item{name}{Character string, name of the plugin.}

  \item{about}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<about>} section, See
  \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for
  details. Skipped if \code{NULL}.}

  \item{components}{Either an object of class
  \code{XiMpLe.node} to be pasted as the
  \code{<components>} section (see
  \code{\link[rkwarddev:rk.XML.components]{rk.XML.components}}
  for details). Or a character vector with at least one
  plugin component file name, relative path from the
  pluginmap file and ending with ".xml".}

  \item{hierarchy}{Either an object of class
  \code{XiMpLe.node} to be pasted as the \code{<hierarchy>}
  section (see
  \code{\link[rkwarddev:rk.XML.hierarchy]{rk.XML.hierarchy}}
  for details). Or a character vector with instructions
  where to place the plugin in the menu hierarchy, one list
  or string for each included component. Valid single
  values are \code{"file"}, \code{"edit"}, \code{"view"},
  \code{"workspace"}, \code{"run"}, \code{"data"},
  \code{"analysis"}, \code{"plots"},
  \code{"distributions"}, \code{"windows"},
  \code{"settings"} and \code{"help"}, anything else will
  place it in a "test" menu. If \code{hierarchy} is a list,
  each entry represents the label of a menu level.}

  \item{require}{Either a (list of) objects of class
  \code{XiMpLe.node} to be pasted as a \code{<require>}
  section (see
  \code{\link[rkwarddev:rk.XML.require]{rk.XML.require}}
  for details). Or a character vector with at least one
  .pluginmap filename to be included in this one.}

  \item{x11.context}{An object of class \code{XiMpLe.node}
  to be pasted as a \code{<context id="x11">} section, see
  \code{\link[rkwarddev:rk.XML.context]{rk.XML.context}}
  for details.}

  \item{import.context}{An object of class
  \code{XiMpLe.node} to be pasted as the \code{<context
  id="import">} section, see
  \code{\link[rkwarddev:rk.XML.context]{rk.XML.context}}
  for details.}

  \item{clean.name}{Logical, if \code{TRUE}, all
  non-alphanumeric characters except the underscore
  (\code{"_"}) will be removed from \code{name}.}

  \item{hints}{Logical, if \code{TRUE} and you leave out
  optional entries (like \code{about=NULL}), dummy sections
  will be added as comments.}

  \item{gen.info}{Logical, if \code{TRUE} a comment note
  will be written into the document, that it was generated
  by \code{rkwarddev} and changes should be done to the
  script.}

  \item{dependencies}{An object of class \code{XiMpLe.node}
  to be pasted as the \code{<dependencies>} section, See
  \code{\link[XiMpLe:rk.XML.dependencies]{rk.XML.dependencies}}
  for details. Skipped if \code{NULL}.}

  \item{namespace}{Character string, the namespace
  attribute of the \code{<document>} node, defaults to the
  plugin name (which you probably shouldn't touch...)}

  \item{priority}{Character string, the priority attribute
  of the \code{<document>} node. Must be either "hidden",
  "low", "medium", or "high", defaults to "medium".}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Write a pluginmap file for RKWard
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

