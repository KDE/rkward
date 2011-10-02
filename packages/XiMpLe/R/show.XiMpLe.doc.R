#' Show method for S4 objects of class \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#'
#' @title Show method for objects of class XiMpLe.doc
#' @param object An object of class \code{XiMpLe.doc}
#' @aliases show,-methods show,XiMpLe.doc-method
#' @seealso \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#' @keywords methods
#' @exportMethod show
#' @rdname show-methods
setGeneric("show")

#' @rdname show-methods
setMethod("show", signature(object="XiMpLe.doc"), function(object){
	cat(pasteXMLTree(object))
})
