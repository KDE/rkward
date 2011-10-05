#' Show methods for S4 objects of class \code{rk.JS.*}
#'
#' @title Show methods for objects of class rk.JS.S
#' @param object An object of class \code{rk.JS.*}
#' @aliases show,-methods show,rk.JS.ite-method show,rk.JS.arr-method show,rk.JS.opt-method
#' @keywords methods
#' @import methods
#' @include rk.JS.ite-class.R
#' @include rk.JS.arr-class.R
#' @include rk.JS.opt-class.R
#' @exportMethod show
#' @rdname show-methods
setGeneric("show")

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.ite"), function(object){
	cat(rk.paste.JS(object))
})

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.arr"), function(object){
	cat(rk.paste.JS(object))
})

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.opt"), function(object){
	cat(rk.paste.JS(object))
})
