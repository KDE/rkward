\name{XMLTree}
\alias{XMLTree}
\title{Constructor function for XiMpLe.doc objects}
\usage{
  XMLTree(..., xml = NULL, dtd = NULL,
    .children = list(...))
}
\arguments{
  \item{...}{Optional children for the XML tree. Must be
  either objects of class XiMpLe.node or character strings,
  which are treated as simple text values.}

  \item{xml}{A named list, XML declaration of the XML tree.
  Currently just pasted, no checking is done.}

  \item{dtd}{A named list, doctype definition of the XML
  tree. Valid elements are \code{doctype}, \code{id} and
  \code{refer}.  Currently just pasted, no checking is
  done.}

  \item{.children}{Alternative way of specifying children,
  if you have them already as a list.}
}
\value{
  An object of class XiMpLe.doc
}
\description{
  Can be used to create full XML trees.
}

