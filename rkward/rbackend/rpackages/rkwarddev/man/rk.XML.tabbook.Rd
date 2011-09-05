\name{rk.XML.tabbook}
\alias{rk.XML.tabbook}
\title{Create XML node "tabbook" for RKWard plugins}
\usage{
  rk.XML.tabbook(label, tab.labels, children = list(),
  id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for this
  plugin element.}

  \item{tab.labels}{Character vector, where each string
  defines the name of one tab. The number of
  \code{tab.labels} also defines the number of tabs.}

  \item{children}{An optional list with objects of class
  \code{XiMpLe.node} (or a list of these objects). You must
  provide one element for each tab. Use \code{NULL} for
  tabs without predefined children.}

  \item{id.name}{Character vector, unique IDs for the
  tabbook (first entry) and all tabs. If \code{"auto"}, IDs
  will be generated automatically from the labels. If
  \code{NULL}, no IDs will be given.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "tabbook" for RKWard plugins
}

