\name{rk.XML.component}
\alias{rk.XML.component}
\title{Create XML "component" node for RKWard plugins}
\usage{
  rk.XML.component(label, file, id.name = "auto", type =
  "standard")
}
\arguments{
  \item{label}{A label.}

  \item{file}{A file name.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"}, an ID will be generated
  automatically from the tag names and IDs of the given
  nodes.}

  \item{type}{Character string, type of component. As of
  now, only "standard" is supported. The option is just
  implemented for completeness.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a component node for components
  sections.
}
\note{
  NOT WORKING YET
}
\seealso{
  \code{\link[rkwarddev:rk.XML.components]{rk.XML.components}}
}

