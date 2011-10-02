# Class XiMpLe.node
#
# This class is used to create DOM trees of XML documents, like objects that are returned
# by \code{\link[XiMpLe:parseXMLTree]{parseXMLTree}}.
# 
# There are certain special values predefined for the \code{name} slot to easily create special XML elements:
# \describe{
# 		\item{\code{name=""}}{If the name is an empty character string, a pseudo node is created,
# 			\code{\link[XiMpLe:pasteXMLNode]{pasteXMLNode}} will paste its \code{value} as plain text.}
# 		\item{\code{name="!--"}}{Creates a comment tag, i.e., this will comment out all its \code{children}.}
# 		\item{\code{name="![CDATA["}}{Creates a CDATA section and places all its \code{children} in it.}
# }
#
# @title S4 class XiMpLe.node
# @slot name Name of the node (i.e., the XML tag identifier). For special names see details.
# @slot attributes A list of named character values, representing the attributes of this node.
# @slot children A list of further objects of class XiMpLe.node, representing child nodes of this node.
# @slot value Plain text to be used as the enclosed value of this node. Set to \code{value=""} if you
#		want a childless node to be forced into an non-empty pair of start and end tags by \code{\link[XiMpLe:pasteXMLNode]{pasteXMLNode}}.
# @name XiMpLe.node,-class
# @aliases XiMpLe.node-class XiMpLe.node,-class
#' @import methods
# @keywords classes
#' @noRd
# @rdname XiMpLe.node-class
# @exportClass XiMpLe.node
#' @export

setClass("XiMpLe.node",
	representation=representation(
		name="character",
		attributes="list",
		children="list",
		value="character"
	),
	prototype(
		name=character(),
		attributes=list(),
		children=list(),
		value=character()
	)
)

setValidity("XiMpLe.node", function(object){
		obj.name <- object@name
		obj.attributes <- object@attributes
		obj.children <- object@children
		obj.value <- object@value

		if(isTRUE(!nchar(obj.name) > 0) & isTRUE(!nchar(obj.value) > 0)){
			print(str(object))
			stop(simpleError("Invalid object: A node must at least have a name or a value!"))
		} else {}

		obj.attributes.names <- names(obj.attributes)
		# if there are attributes, check that they all have names
		if(length(obj.attributes) > 0){
			if(length(obj.attributes) != length(obj.attributes.names)){
				stop(simpleError("Invalid object: All attributes must have names!"))
			} else {}
		} else {}

		# check content of children
		if(length(obj.children) > 0){
			child.nodes <- sapply(obj.children, function(this.child){inherits(this.child, "XiMpLe.node")})
			if(!all(child.nodes)){
				stop(simpleError("Invalid object: All list elements of children must be of class XiMpLe.node!"))
			} else {}
		} else {}
	return(TRUE)
})
