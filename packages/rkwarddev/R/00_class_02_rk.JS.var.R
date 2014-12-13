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


#' S4 Class rk.JS.var
#' 
#' This simple class is used for JavaScript generation and produced by
#' \code{\link{rk.JS.vars}}. You shouldn't need to temper with this
#' type of class manually.
#' 
#' @slot JS.var Character string, name of the JavaScript variable.
#' @slot XML.var Character string, name of the XML variable.
#' @slot prefix Character string, an optional prefix for variable names.
#' @slot modifiers A list of modifiers to apply to the XML node property.
#' @slot default Logical, whether the default value (no special modifier) of the node should also be defined.
#' @slot append.modifier Logical, if a modifier is given, should that become part of the variable name?
#' @slot join Character string, if set is used to collapse multiple values into one string.
#' @slot vars A list of objects of class rk.JS.var.
#' @slot getter Character string, the JavaScript function which should be used to fetch the values from the plugin.
#' @keywords Classes
#' @rdname rk.JS.var-class
#' @export

setClass("rk.JS.var",
  representation=representation(
    JS.var="character",
    XML.var="character",
    prefix="character",
    modifiers="list",
    default="logical",
    append.modifier="logical",
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
    append.modifier=TRUE,
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
