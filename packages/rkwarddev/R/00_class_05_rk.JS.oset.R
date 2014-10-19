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


# this simple class is for JavaScript generation,
# produced by rk.JS.optionset()

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
