\name{id.os}
\alias{id.os}
\title{Create JavaScript variables from optionsets}
\usage{
  id.os(optionset, js = TRUE, add.abbrev = FALSE)
}
\arguments{
  \item{optionset}{An object of class \code{XiMpLe.node}
  containing a full \code{<optionset>}.}

  \item{js}{Logical, if \code{TRUE} valid JavaScript
  varaible names will be returned, otherwise the XML ID
  names, respectively.}

  \item{add.abbrev}{Logical, if \code{TRUE} the JavaScript
  variables will all have a prefix with an three letter
  abbreviation of the XML tag type to improve the
  readability of the code. But it's probably better to add
  this in the XML code in the first place.}
}
\value{
  A character vector. For each \code{<optioncolumn>} in the
  set, this vector will contain either its variable name or
  XML ID, depending on the \code{js} switch.
}
\description{
  Scans an \code{<optionset>} node and returns the relevant
  variable or ID names of each \code{<optioncolumn>}
  included.
}
\details{
  This function can be of assistance when dealing with
  \code{<optionset>} nodes, because what you need in the
  JavaScript part of your plugin is the correct variable
  name of its columns.
}
\seealso{
  \code{\link[rkwarddev:rk.XML.optioncolumn]{rk.XML.optioncolumn}},
  \code{\link[rkwarddev:rk.XML.optiondisplay]{rk.XML.optiondisplay}},
  and the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

