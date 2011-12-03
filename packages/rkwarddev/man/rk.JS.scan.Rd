\name{rk.JS.scan}
\alias{rk.JS.scan}
\title{Create JavaScript variables from plugin XML}
\usage{
  rk.JS.scan(pXML, js = TRUE, add.abbrev = FALSE,
    indent.by = "\t")
}
\arguments{
  \item{pXML}{Either an object of class \code{XiMpLe.doc}
  or \code{XiMpLe.node}, or path to a plugin XML file.}

  \item{js}{Logical, if \code{TRUE} usable JavaScript code
  will be returned, otherwise a character vector with only
  the relevant ID names.}

  \item{add.abbrev}{Logical, if \code{TRUE} the JavaScript
  variables will all have a prefix with an three letter
  abbreviation of the XML tag type to improve the
  readability of the code. But it's probably better to add
  this in the XML code in the first place.}

  \item{indent.by}{Character string used to indent each
  entry if \code{js=TRUE}.}
}
\value{
  A character vector.
}
\description{
  Create JavaScript variables from plugin XML
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

