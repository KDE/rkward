\name{pasteXMLNode}
\alias{pasteXMLNode}
\title{Paste an XML node from a XiMpLe.node object}
\usage{
  pasteXMLNode(node, level = 1, shine = 1, indent.by =
  "\t")
}
\arguments{
  \item{node}{An object of class \code{XiMpLe.node}.}

  \item{level}{Indentation level.}

  \item{shine}{Integer, controlling if the output should be
  formatted for better readability. Possible values:
  \describe{ \item{0}{No formatting.} \item{1}{Nodes will
  be indented.} \item{2}{Nodes will be indented and each
  attribute gets a new line.} }}

  \item{indent.by}{A charachter string defining how
  indentation should be done. Defaults to tab.}
}
\description{
  Paste an XML node from a XiMpLe.node object
}

