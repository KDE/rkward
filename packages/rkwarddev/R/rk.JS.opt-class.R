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


#' @export

# this simple class is for JavaScript generation,
# produced by rk.JS.options()

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
