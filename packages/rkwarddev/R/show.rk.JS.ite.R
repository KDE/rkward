#' Show method for S4 objects of class \code{rk.JS.ite}
#'
#' @title Show method for objects of class rk.JS.ite
#' @param object An object of class \code{rk.JS.ite}
#' @aliases show,-methods show,rk.JS.ite-method
#' @keywords methods
#' @import methods
#' @include rk.JS.ite-class.R
#' @exportMethod show
#' @rdname show-methods
setGeneric("show")

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.ite"), function(object){
	cat(rk.paste.JS(object))
})
