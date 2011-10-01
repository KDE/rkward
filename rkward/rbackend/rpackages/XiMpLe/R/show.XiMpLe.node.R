#' Show method for S4 objects of class \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#'
#' @title Show method for objects of class XiMpLe.node
#' @param object An object of class \code{XiMpLe.node}
#' @aliases show,-methods show,XiMpLe.node-method
#' @seealso \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#' @keywords methods
#' @exportMethod show
#' @rdname show-methods
setGeneric("show")

#' @rdname show-methods
setMethod("show", signature(object="XiMpLe.node"), function(object){
	cat(pasteXMLNode(object))
})
