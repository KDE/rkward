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
# produced by rk.JS.vars()

setClass("rk.JS.var",
  representation=representation(
    JS.var="character",
    XML.var="character",
    prefix="character",
    modifiers="list",
    default="logical",
    join="character",
    vars="list",
    getter="character"
  ),
  prototype(
    JS.var=character(),
    XML.var=character(),
    prefix=character(),
    modifiers=list(),
    default=FALSE,
    join="",
    vars=list(),
    getter="getValue" # for compatibility with earlier releases
  )
)

setValidity("rk.JS.var", function(object){
    # vars in this object must be of the same class
    sapply(slot(object, "vars"), function(this.var){
      if(!inherits(this.var, "rk.JS.var")){
        stop(simpleError("Slot 'vars' can only have a list of elements of class 'rk.JS.var'!"))
      } else {}
    })
  return(TRUE)
})
