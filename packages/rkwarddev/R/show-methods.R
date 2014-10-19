# Copyright 2010-2014 Meik Michalke <meik.michalke@hhu.de>
#
# This file is part of the R package rkwarddev.
#
# rkwarddev is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rkwarddev is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rkwarddev.  If not, see <http://www.gnu.org/licenses/>.


#' Show methods for S4 objects of class \code{rk.JS.*}
#'
#' @param object An object of class \code{rk.JS.*}
#' @aliases show,-methods show,rk.JS.ite-method show,rk.JS.arr-method show,rk.JS.opt-method show,rk.JS.oset-method show,rk.JS.var-method
#' @keywords methods
#' @import methods
#' @include 00_class_01_rk.JS.arr.R
#' @include 00_class_02_rk.JS.var.R
#' @include 00_class_03_rk.JS.ite.R
#' @include 00_class_04_rk.JS.opt.R
#' @include 00_class_05_rk.JS.oset.R
#' @include echo.R
#' @exportMethod show
#' @rdname show-methods
setGeneric("show")

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.arr"), function(object){
  cat(rk.paste.JS(object))
})

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.ite"), function(object){
  cat(rk.paste.JS(object))
})

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.opt"), function(object){
  cat(rk.paste.JS(object))
})

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.oset"), function(object){
  cat(rk.paste.JS(object))
})

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.var"), function(object){
  cat(rk.paste.JS(object))
})

#' @rdname show-methods
setMethod("show", signature(object="rk.JS.echo"), function(object){
  cat(rk.paste.JS(object))
})
