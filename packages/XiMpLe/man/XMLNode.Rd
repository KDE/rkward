\name{XMLNode}
\alias{XMLNode}
\title{Constructor function for XiMpLe.node objects}
\usage{
  XMLNode(name, ..., attrs = NULL, namespace = "",
    namespaceDefinitions = NULL, .children = list(...))
}
\arguments{
  \item{name}{Character string, the tag name.}

  \item{...}{Optional children for the tag. Must be either
  objects of class XiMpLe.node or character strings, which
  are treated as simple text values. If this is empty, the
  tag will be treated as an empty tag. To force a closing
  tag, supply an empty string, i.e. \code{""}.}

  \item{attrs}{An optional named list of attributes.}

  \item{namespace}{Currently ignored.}

  \item{namespaceDefinitions}{Currently ignored.}

  \item{.children}{Alternative way of specifying children,
  if you have them already as a list.}
}
\value{
  An object of class XiMpLe.node
}
\description{
  Can be used to create XML nodes.
}

