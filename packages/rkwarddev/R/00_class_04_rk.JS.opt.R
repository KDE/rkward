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


#' S4 Class rk.JS.opt
#' 
#' This simple class is used for JavaScript generation and produced by
#' \code{\link{rk.JS.options}}. You shouldn't need to temper with this
#' type of class manually.
#' 
#' @slot var.name Character string, the name of the variable.
#' @slot opt.name Character string, the name of the option.
#' @slot collapse Character string, used to collapse several options into one string.
#' @slot ifs A list with objects of class rk.JS.ite.
#' @slot array Logical, whether to use an array for options.
#' @slot funct Character string, name of the R function to be called to combine the options.
#' @keywords Classes
#' @rdname rk.JS.opt-class
#' @export

setClass("rk.JS.opt",
  representation=representation(
    var.name="character",
    opt.name="character",
    collapse="character",
    ifs="list",
    array="logical",
    funct="character"
  ),
  prototype(
    var.name=character(),
    opt.name=character(),
    collapse=character(),
    ifs=list(),
    array=NULL,
    funct=character()
  )
)

setValidity("rk.JS.opt", function(object){
  sapply(object@ifs, function(thisIf){
    if(!inherits(thisIf, "rk.JS.ite")){
      stop(simpleError("All option rules in rk.JS.opt objects must be of class rk.JS.ite!"))
    }
  })
  return(TRUE)
})
