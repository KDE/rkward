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
#' \itemize{
#'		\item{\code{XMLName()}: }{get/set the XML node name (slot \code{name} of class \code{XiMpLe.node})}
#'		\item{\code{XMLAttrs()}: }{get/set the XML node attributes (slot \code{attrs} of class \code{XiMpLe.node})}
#'		\item{\code{XMLValue()}: }{get/set the XML node value (slot \code{value} of class \code{XiMpLe.node})}
#'		\item{\code{XMLChildren()}: }{get/set the XML child nodes (slot \code{children} of both classes \code{XiMpLe.node}
#'			and  \code{XiMpLe.doc})}
#'		\item{\code{XMLFile()}: }{get/set the XML document file name  (slot \code{file} of class \code{XiMpLe.doc})}
#'		\item{\code{XMLDecl()}: }{get/set the XML document declaration (slot \code{xml} of class \code{XiMpLe.doc})}
#'		\item{\code{XMLDTD()}: }{get/set the XML document doctype definition (slot \code{dtd} of class \code{XiMpLe.doc})}
#' }
#'
#' Another special method can scan a node/document tree object for appearances of nodes with a particular name:
#'
#' \itemize{
#'		\item{\code{XMLScan()}: }{get/set the XML nodes by name (recursively searches slot \code{name} of both classes
#'			\code{XiMpLe.node} and  \code{XiMpLe.doc})}
#' }
#'
#' @param obj An object of class \code{XiMpLe.node} or \code{XiMpLe.doc}
#' @seealso
#'		\code{\link[XiMpLe:node]{node}},
#'		\code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}},
#'		\code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#' @keywords methods
#' @docType methods
#' @rdname XMLGetters-methods
#' @exportMethod XMLName
#' @examples
#' xmlTestNode <- XMLNode("foo", XMLNode("testchild"))
#' XMLName(xmlTestNode) # returns "foo"
#' XMLName(xmlTestNode) <- "bar"
#' XMLName(xmlTestNode) # now returns "bar"
#'
#' # search for a child node
#' XMLScan(xmlTestNode, "testchild")
#' # remove nodes of that name
#' XMLScan(xmlTestNode, "testchild") <- NULL
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

## scan a tree for appearances of nodes
#' @rdname XMLGetters-methods
#' @exportMethod XMLScan
setGeneric("XMLScan", function(obj, name) standardGeneric("XMLScan"))

# internal helper function
find.nodes <- function(nodes, nName){
	res <- list()
	for (thisNode in nodes){
			if(identical(XMLName(thisNode), nName)){
				res <- append(res, thisNode)
			} else if(length(XMLChildren(thisNode)) > 0){
				res <- append(res, find.nodes(XMLChildren(thisNode), nName=nName))
			} else {}
		}
	return(res)
}

#' @rdname XMLGetters-methods
#' @aliases
#'		XMLScan,-methods
#'		XMLScan,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLScan",
	signature=signature(obj="XiMpLe.node"),
	function(obj, name){
		node.list <- find.nodes(
			nodes=child.list(obj),
			nName=name)
		if(identical(node.list, list())){
			return(NULL)
		} else if(length(node.list) == 1){
			return(node.list[[1]])
		} else {
			return(node.list)
		}
	}
)

#' @rdname XMLGetters-methods
#' @aliases
#'		XMLScan,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLScan",
	signature=signature(obj="XiMpLe.doc"),
	function(obj, name){
		node.list <- find.nodes(
			nodes=XMLChildren(obj),
			nName=name)
		if(identical(node.list, list())){
			return(NULL)
		} else if(length(node.list) == 1){
			return(node.list[[1]])
		} else {
			return(node.list)
		}
	}
)

#' @rdname XMLGetters-methods
#' @exportMethod XMLScan<-
setGeneric("XMLScan<-", function(obj, name, value) standardGeneric("XMLScan<-"))

# internal helper function
replace.nodes <- function(nodes, nName, replacement){
	nodes <- sapply(nodes, function(thisNode){
			if(identical(XMLName(thisNode), nName)){
				return(replacement)
			} else if(length(XMLChildren(thisNode)) > 0){
				XMLChildren(thisNode) <- replace.nodes(
						XMLChildren(thisNode),
						nName=nName,
						replacement=replacement)
				return(thisNode)
			} else {
				return(thisNode)
			}
		})
	# get rid of NULL in list
	nodes <- Filter(Negate(is.null), nodes)
	return(nodes)
}

#' @rdname XMLGetters-methods
#' @aliases
#'		XMLScan<-,-methods
#'		XMLScan<-,XiMpLe.node-method
#' @docType methods
#' @include XiMpLe.node-class.R
setMethod("XMLScan<-",
	signature=signature(obj="XiMpLe.node"),
	function(obj, name, value){
		# prevent the creation of invalid results
		stopifnot(is.XiMpLe.node(value) || is.null(value))
		obj <- replace.nodes(
			nodes=child.list(obj),
			nName=name,
			replacement=value)
		stopifnot(validObject(object=obj, test=TRUE, complete=TRUE))
		if(identical(obj, as.list(value))){
			# it seems the full object was replaced by value
			return(value)
		} else {
			return(obj[[1]])
		}
	}
)

#' @rdname XMLGetters-methods
#' @aliases
#'		XMLScan<-,XiMpLe.doc-method
#' @docType methods
#' @include XiMpLe.doc-class.R
setMethod("XMLScan<-",
	signature=signature(obj="XiMpLe.doc"),
	function(obj, name, value){
		# prevent the creation of invalid results
		stopifnot(is.XiMpLe.node(value) || is.null(value))
		XMLChildren(obj) <- replace.nodes(
				nodes=XMLChildren(obj),
				nName=name,
				replacement=value)[[1]]
		stopifnot(validObject(object=obj, test=TRUE, complete=TRUE))
		return(obj)
	}
)
