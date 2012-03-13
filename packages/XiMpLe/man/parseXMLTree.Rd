\name{parseXMLTree}
\alias{parseXMLTree}
\title{Read an XML file into an R object}
\usage{
  parseXMLTree(file, drop = NULL, object = FALSE)
}
\arguments{
  \item{file}{Character string, valid path to the XML file
  which should be parsed.}

  \item{drop}{Character vector with the possible values
  \code{"comments"}, \code{"cdata"} \code{"declarations"}
  and \code{"doctype"}, defining element classes to be
  dropped from the resulting object.}

  \item{object}{Logical, if \code{TRUE}, \code{file} will
  not be treated as a path name but as a character vector
  to be parsed as XML directly.}
}
\value{
  An object of class \code{XiMpLe.doc} with four slots:
  \describe{ \item{\code{file}:}{Full path to the parsed
  file, or \code{"object"} if \code{object=TRUE}.}
  \item{\code{xml}:}{XML declaration, if found.}
  \item{\code{dtd}:}{Doctype definition, if found.}
  \item{\code{children}:}{A list of objects of class
  \code{XiMpLe.node}, with the elements \code{"name"} (the
  node name), \code{"attributes"} (list of attributes, if
  found), \code{"children"} (list of \code{XiMpLe.node}
  object, if found) and \code{"value"} (text value between
  a pair of start/end tags, if found).} }
}
\description{
  Read an XML file into an R object
}

