\docType{methods}
\name{XMLName}
\alias{XMLAttrs}
\alias{XMLAttrs<-}
\alias{XMLAttrs<-,-methods}
\alias{XMLAttrs,-methods}
\alias{XMLAttrs<-,XiMpLe.node-method}
\alias{XMLAttrs,XiMpLe.node-method}
\alias{XMLChildren}
\alias{XMLChildren<-}
\alias{XMLChildren<-,-methods}
\alias{XMLChildren,-methods}
\alias{XMLChildren<-,XiMpLe.doc-method}
\alias{XMLChildren,XiMpLe.doc-method}
\alias{XMLChildren<-,XiMpLe.node-method}
\alias{XMLChildren,XiMpLe.node-method}
\alias{XMLDecl}
\alias{XMLDecl<-}
\alias{XMLDecl<-,-methods}
\alias{XMLDecl,-methods}
\alias{XMLDecl<-,XiMpLe.doc-method}
\alias{XMLDecl,XiMpLe.doc-method}
\alias{XMLDTD}
\alias{XMLDTD<-}
\alias{XMLDTD<-,-methods}
\alias{XMLDTD,-methods}
\alias{XMLDTD<-,XiMpLe.doc-method}
\alias{XMLDTD,XiMpLe.doc-method}
\alias{XMLFile}
\alias{XMLFile<-}
\alias{XMLFile<-,-methods}
\alias{XMLFile,-methods}
\alias{XMLFile<-,XiMpLe.doc-method}
\alias{XMLFile,XiMpLe.doc-method}
\alias{XMLName}
\alias{XMLName<-}
\alias{XMLName<-,-methods}
\alias{XMLName,-methods}
\alias{XMLName<-,XiMpLe.node-method}
\alias{XMLName,XiMpLe.node-method}
\alias{XMLScan}
\alias{XMLScan<-}
\alias{XMLScan<-,-methods}
\alias{XMLScan,-methods}
\alias{XMLScan<-,XiMpLe.doc-method}
\alias{XMLScan,XiMpLe.doc-method}
\alias{XMLScan<-,XiMpLe.node-method}
\alias{XMLScan,XiMpLe.node-method}
\alias{XMLValue}
\alias{XMLValue<-}
\alias{XMLValue<-,-methods}
\alias{XMLValue,-methods}
\alias{XMLValue<-,XiMpLe.node-method}
\alias{XMLValue,XiMpLe.node-method}
\title{Getter/setter methods for S4 objects of XiMpLe XML classes}
\arguments{
  \item{obj}{An object of class \code{XiMpLe.node} or
  \code{XiMpLe.doc}}
}
\description{
  Used to get/set certain slots from objects of class
  \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}} and
  \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}.
}
\details{
  These are convenience methods to get or set slots from
  XML objects without using the \code{@} operator.

  \itemize{ \item{\code{XMLName()}: }{get/set the XML node
  name (slot \code{name} of class \code{XiMpLe.node})}
  \item{\code{XMLAttrs()}: }{get/set the XML node
  attributes (slot \code{attrs} of class
  \code{XiMpLe.node})} \item{\code{XMLValue()}: }{get/set
  the XML node value (slot \code{value} of class
  \code{XiMpLe.node})} \item{\code{XMLChildren()}:
  }{get/set the XML child nodes (slot \code{children} of
  both classes \code{XiMpLe.node} and \code{XiMpLe.doc})}
  \item{\code{XMLFile()}: }{get/set the XML document file
  name (slot \code{file} of class \code{XiMpLe.doc})}
  \item{\code{XMLDecl()}: }{get/set the XML document
  declaration (slot \code{xml} of class \code{XiMpLe.doc})}
  \item{\code{XMLDTD()}: }{get/set the XML document doctype
  definition (slot \code{dtd} of class \code{XiMpLe.doc})}
  }

  Another special method can scan a node/document tree
  object for appearances of nodes with a particular name:

  \itemize{ \item{\code{XMLScan()}: }{get/set the XML nodes
  by name (recursively searches slot \code{name} of both
  classes \code{XiMpLe.node} and \code{XiMpLe.doc})} }
}
\examples{
xmlTestNode <- XMLNode("foo", XMLNode("testchild"))
XMLName(xmlTestNode) # returns "foo"
XMLName(xmlTestNode) <- "bar"
XMLName(xmlTestNode) # now returns "bar"

# search for a child node
XMLScan(xmlTestNode, "testchild")
# remove nodes of that name
XMLScan(xmlTestNode, "testchild") <- NULL
}
\seealso{
  \code{\link[XiMpLe:node]{node}},
  \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}},
  \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
}
\keyword{methods}

