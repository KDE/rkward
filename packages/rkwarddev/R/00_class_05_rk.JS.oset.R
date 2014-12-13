# Copyright 2014 Meik Michalke <meik.michalke@hhu.de>
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


#' S4 Class rk.JS.oset
#' 
#' This simple class is used for JavaScript generation and produced by
#' \code{\link{rk.JS.optionset}}. You shouldn't need to temper with this
#' type of class manually.
#' 
#' @slot vars An object of class \code{rk.JS.var}.
#' @slot loopvar Character string, name of the index variable used in the for loop.
#' @slot columns A list of <optioncolumn> nodes.
#' @slot body A list of JavaScript code, the body of the for loop.
#' @slot collapse Character string, how all optioncolumns should be concatenated on the R code level.
#' @keywords Classes
#' @rdname rk.JS.oset-class
#' @include 00_class_02_rk.JS.var.R
#' @export

setClass("rk.JS.oset",
  representation=representation(
    vars="rk.JS.var",
    loopvar="character",
    columns="list",
    body="list",
    collapse="character"
  ),
  prototype(
    vars=new("rk.JS.var"),
    loopvar="i",
    columns=list(),
    body=list(),
    collapse=",\\n\\t"
  )
)

#' @include 00_class_02_rk.JS.var.R
setValidity("rk.JS.oset", function(object){
    sapply(slot(object, "columns"), function(this.col){
      if(!inherits(this.col, "XiMpLe.node")){
        stop(simpleError("Slot 'columns' can only have a list of elements of class 'XiMpLe.node'!"))
      } else {}
      if(!identical(XMLName(this.col), "optioncolumn")){
        stop(simpleError("Slot 'columns' can only have a list of <optioncolumn> nodes!"))
      } else {}
    })
  return(TRUE)
})
