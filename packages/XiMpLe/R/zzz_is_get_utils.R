## the name "zzz_*" is just to ensure roxygen doesn't parse it before XMLNode.R and XMLTree.R

#' @param x An arbitrary \code{R} object.
#' @rdname XMLNode
#' @export
is.XiMpLe.node <- function(x){
	inherits(x, "XiMpLe.node")
}

#' @param x An arbitrary \code{R} object.
#' @rdname XMLTree
#' @export
is.XiMpLe.doc <- function(x){
	inherits(x, "XiMpLe.doc")
}


#' Getter methods for S4 objects of XiMpLe XML classes
#'
#' Used to get certain slots from objects of class \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#' and \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}.
#' 
#' These are convenience methods to get slots from XML objects without using the \code{@@} operator.
#'
#' @param obj An object of class \code{XiMpLe.node} or \code{XiMpLe.doc}
#' @seealso
#'		\code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#'		\code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#' @keywords methods
#' @docType methods
#' @rdname XMLGetters-methods
#' @exportMethod getXMLName
setGeneric("getXMLName", function(obj) standardGeneric("getXMLName"))

#' @rdname XMLGetters-methods
#' @aliases
#'		getXMLName,-methods
#'		getXMLName,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("getXMLName",
	signature=signature(obj="XiMpLe.node"),
	function(obj){
		return(obj@name)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod getXMLAttrs
setGeneric("getXMLAttrs", function(obj) standardGeneric("getXMLAttrs"))

#' @rdname XMLGetters-methods
#' @aliases
#'		getXMLAttrs,-methods
#'		getXMLAttrs,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("getXMLAttrs",
	signature=signature(obj="XiMpLe.node"),
	function(obj){
		return(obj@attributes)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod getXMLChildren
setGeneric("getXMLChildren", function(obj) standardGeneric("getXMLChildren"))

#' @rdname XMLGetters-methods
#' @aliases
#'		getXMLChildren,-methods
#'		getXMLChildren,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("getXMLChildren",
	signature=signature(obj="XiMpLe.node"),
	function(obj){
		return(obj@children)
	}
)

#' @rdname XMLGetters-methods
#' @aliases
#'		getXMLChildren,-methods
#'		getXMLChildren,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("getXMLChildren",
	signature=signature(obj="XiMpLe.doc"),
	function(obj){
		return(obj@children)
	}
)


#' @rdname XMLGetters-methods
#' @exportMethod getXMLValue
setGeneric("getXMLValue", function(obj) standardGeneric("getXMLValue"))

#' @rdname XMLGetters-methods
#' @aliases
#'		getXMLValue,-methods
#'		getXMLValue,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("getXMLValue",
	signature=signature(obj="XiMpLe.node"),
	function(obj){
		return(obj@value)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod getXMLFile
setGeneric("getXMLFile", function(obj) standardGeneric("getXMLFile"))

#' @rdname XMLGetters-methods
#' @aliases
#'		getXMLFile,-methods
#'		getXMLFile,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("getXMLFile",
	signature=signature(obj="XiMpLe.doc"),
	function(obj){
		return(obj@file)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod getXMLDecl
setGeneric("getXMLDecl", function(obj) standardGeneric("getXMLDecl"))

#' @rdname XMLGetters-methods
#' @aliases
#'		getXMLDecl,-methods
#'		getXMLDecl,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("getXMLDecl",
	signature=signature(obj="XiMpLe.doc"),
	function(obj){
		return(obj@xml)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod getXMLDTD
setGeneric("getXMLDTD", function(obj) standardGeneric("getXMLDTD"))

#' @rdname XMLGetters-methods
#' @aliases
#'		getXMLDTD,-methods
#'		getXMLDTD,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("getXMLDTD",
	signature=signature(obj="XiMpLe.doc"),
	function(obj){
		return(obj@dtd)
	}
)

