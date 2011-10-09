\name{pasteXMLTree}
\alias{pasteXMLTree}
\title{Paste an XML tree structure from a XiMpLe.doc object}
\usage{
  pasteXMLTree(obj, shine = 1, indent.by = "\t", tidy =
  TRUE)
}
\arguments{
  \item{obj}{An object of class \code{XiMpLe.doc}.}

  \item{shine}{Integer, controlling if the output should be
  formatted for better readability. Possible values:
  \describe{ \item{0}{No formatting.} \item{1}{Nodes will
  be indented.} \item{2}{Nodes will be indented and each
  attribute gets a new line.} }}

  \item{indent.by}{A charachter string defining how
  indentation should be done. Defaults to tab.}

  \item{tidy}{Logical, if \code{TRUE} the special
  characters "<" and ">" will be replaced with the entities
  "&lt;" and "gt;" in attributes and text values.}
}
\description{
  Paste an XML tree structure from a XiMpLe.doc object
}

