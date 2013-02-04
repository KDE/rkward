#' Show method for S4 objects of XiMpLe XML classes
#' 
#' Used to display objects of class \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#' and \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#'
#' @param object An object of class \code{XiMpLe.doc} or \code{XiMpLe.node}
#' @aliases
#'		show,-methods
#'		show,XiMpLe.doc-method
#'		show,XiMpLe.node-method
#'		show,XiMpLe.XML-method
#' @seealso	
#'		\code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#'		\code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#' @keywords methods
#' @exportMethod show
#' @rdname show-methods
setGeneric("show")

#' @rdname show-methods
#' @include XiMpLe.node-class.R
#' @include XiMpLe.doc-class.R
setMethod("show", signature(object="XiMpLe.XML"), function(object){
	cat(pasteXML(object))
})
