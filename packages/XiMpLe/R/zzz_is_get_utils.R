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


#' Getter/setter methods for S4 objects of XiMpLe XML classes
#'
#' Used to get/set certain slots from objects of class \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#' and \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}.
#' 
#' These are convenience methods to get or set slots from XML objects without using the \code{@@} operator.
#'
#' @param obj An object of class \code{XiMpLe.node} or \code{XiMpLe.doc}
#' @seealso
#'		\code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#'		\code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#' @keywords methods
#' @docType methods
#' @rdname XMLGetters-methods
#' @exportMethod XMLName
setGeneric("XMLName", function(obj) standardGeneric("XMLName"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLName,-methods
#'		XMLName,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLName",
	signature=signature(obj="XiMpLe.node"),
	function(obj){
		return(obj@name)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLName<-
setGeneric("XMLName<-", function(obj, value) standardGeneric("XMLName<-"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLName<-,-methods
#'		XMLName<-,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLName<-",
	signature=signature(obj="XiMpLe.node"),
	function(obj, value){
		obj@name <- value
		return(obj)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLAttrs
setGeneric("XMLAttrs", function(obj) standardGeneric("XMLAttrs"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLAttrs,-methods
#'		XMLAttrs,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLAttrs",
	signature=signature(obj="XiMpLe.node"),
	function(obj){
		return(obj@attributes)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLAttrs<-
setGeneric("XMLAttrs<-", function(obj, value) standardGeneric("XMLAttrs<-"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLAttrs<-,-methods
#'		XMLAttrs<-,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLAttrs<-",
	signature=signature(obj="XiMpLe.node"),
	function(obj, value){
		obj@attributes <- value
		return(obj)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLChildren
setGeneric("XMLChildren", function(obj) standardGeneric("XMLChildren"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLChildren,-methods
#'		XMLChildren,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLChildren",
	signature=signature(obj="XiMpLe.node"),
	function(obj){
		return(obj@children)
	}
)
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLChildren,-methods
#'		XMLChildren,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLChildren",
	signature=signature(obj="XiMpLe.doc"),
	function(obj){
		return(obj@children)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLChildren<-
setGeneric("XMLChildren<-", function(obj, value) standardGeneric("XMLChildren<-"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLChildren<-,-methods
#'		XMLChildren<-,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLChildren<-",
	signature=signature(obj="XiMpLe.node"),
	function(obj, value){
		obj@children <- child.list(value)
		return(obj)
	}
)
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLChildren<-,-methods
#'		XMLChildren<-,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLChildren<-",
	signature=signature(obj="XiMpLe.doc"),
	function(obj, value){
		obj@children <- child.list(value)
		return(obj)
	}
)


#' @rdname XMLGetters-methods
#' @exportMethod XMLValue
setGeneric("XMLValue", function(obj) standardGeneric("XMLValue"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLValue,-methods
#'		XMLValue,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLValue",
	signature=signature(obj="XiMpLe.node"),
	function(obj){
		return(obj@value)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLValue<-
setGeneric("XMLValue<-", function(obj, value) standardGeneric("XMLValue<-"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLValue<-,-methods
#'		XMLValue<-,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLValue<-",
	signature=signature(obj="XiMpLe.node"),
	function(obj, value){
		obj@value <- value
		return(obj)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLFile
setGeneric("XMLFile", function(obj) standardGeneric("XMLFile"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLFile,-methods
#'		XMLFile,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLFile",
	signature=signature(obj="XiMpLe.doc"),
	function(obj){
		return(obj@file)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLFile<-
setGeneric("XMLFile<-", function(obj, value) standardGeneric("XMLFile<-"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLFile<-,-methods
#'		XMLFile<-,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLFile<-",
	signature=signature(obj="XiMpLe.doc"),
	function(obj, value){
		obj@file <- value
		return(obj)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLDecl
setGeneric("XMLDecl", function(obj) standardGeneric("XMLDecl"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLDecl,-methods
#'		XMLDecl,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLDecl",
	signature=signature(obj="XiMpLe.doc"),
	function(obj){
		return(obj@xml)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLDecl<-
setGeneric("XMLDecl<-", function(obj, value) standardGeneric("XMLDecl<-"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLDecl<-,-methods
#'		XMLDecl<-,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLDecl<-",
	signature=signature(obj="XiMpLe.doc"),
	function(obj, value){
		obj@xml <- value
		return(obj)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLDTD
setGeneric("XMLDTD", function(obj) standardGeneric("XMLDTD"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLDTD,-methods
#'		XMLDTD,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLDTD",
	signature=signature(obj="XiMpLe.doc"),
	function(obj){
		return(obj@dtd)
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLDTD<-
setGeneric("XMLDTD<-", function(obj, value) standardGeneric("XMLDTD<-"))
#' @rdname XMLGetters-methods
#' @aliases
#'		XMLDTD<-,-methods
#'		XMLDTD<-,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLDTD<-",
	signature=signature(obj="XiMpLe.doc"),
	function(obj, value){
		obj@dtd <- value
		return(obj)
	}
)
