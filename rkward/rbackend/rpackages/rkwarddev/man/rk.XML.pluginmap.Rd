\name{rk.XML.pluginmap}
\alias{rk.XML.pluginmap}
\title{Write a pluginmap file for RKWard}
\usage{
  rk.XML.pluginmap(name, about = NULL, components,
  plugin.dir = "plugins", hierarchy = "analysis",
  clean.name = TRUE)
}
\arguments{
  \item{name}{Character string, name of the plugin.}

  \item{about}{A list with descriptive information on the
  plugin,its authors and dependencies. See
  \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for
  details! Skipped if \code{NULL}.}

  \item{components}{A character vector with at least one
  plugin component file name, ending with ".xml".}

  \item{plugin.dir}{Character string, relative path to the
  component XML and JS files.}

  \item{hierarchy}{A character vector with instructions
  where to place the plugin in the menu hierarchy, one
  string for each included component. Valid values are
  \code{"analysis"}, \code{"plots"} and \code{"data"}. To
  place your dialogs somewhere else, edit the pluginmap
  manually.}

  \item{clean.name}{Logical, if \code{TRUE}, all
  non-alphanumeric characters except the underscore
  (\code{"_"}) will be removed from \code{name}.}
}
\description{
  Write a pluginmap file for RKWard
}

