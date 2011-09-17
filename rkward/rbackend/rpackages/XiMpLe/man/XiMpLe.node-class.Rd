\name{XiMpLe.node-class}
\title{S4 class XiMpLe.node}
\description{Class XiMpLe.node}
\details{This class is used to create DOM trees of XML documents, like objects that are returned
by \code{\link[XiMpLe:parseXMLTree]{parseXMLTree}}.

There are certain special values predefined for the \code{name} slot to easily create special XML elements:
\describe{
\item{\code{name=""}}{If the name is an empty character string, a pseudo node is created,
\code{\link[XiMpLe:pasteXMLNode]{pasteXMLNode}} will paste its \code{value} as plain text.}
\item{\code{name="!--"}}{Creates a comment tag, i.e., this will comment out all its \code{children}.}
\item{\code{name="![CDATA["}}{Creates a CDATA section and places all its \code{children} in it.}
}}
\alias{XiMpLe.node-class}
\alias{XiMpLe.node,-class}
\keyword{classes}
\section{Slots}{\describe{\item{\code{name}:}{(\code{\link{character}}) Name of the node (i.e., the XML tag identifier). For special names see details.}\item{\code{attributes}:}{(\code{\link{list}}) A list of named character values, representing the attributes of this node.}\item{\code{children}:}{(\code{\link{list}}) A list of further objects of class XiMpLe.node, representing child nodes of this node.}\item{\code{value}:}{(\code{\link{character}}) Plain text to be used as the enclosed value of this node. Set to \code{value=""} if you
want a childless node to be forced into an non-empty pair of start and end tags by \code{\link[XiMpLe:pasteXMLNode]{pasteXMLNode}}.}}}

