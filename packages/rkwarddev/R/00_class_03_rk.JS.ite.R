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


#' S4 Class rk.JS.ite
#' 
#' This simple class is used for JavaScript generation and produced by
#' \code{\link{ite}}. You shouldn't need to temper with this
#' type of class manually.
#' 
#' @slot ifJS Character string, the "if" clause.
#' @slot thenJS Character string, the "then" body.
#' @slot thenifJS A list with exactly one optional object of class rk.JS.ite.
#' @slot elseJS Character string, the "else" body.
#' @slot elifJS A list with exactly one optional object of class rk.JS.ite.
#' @keywords Classes
#' @rdname rk.JS.ite-class
#' @export

setClass("rk.JS.ite",
  representation=representation(
    ifJS="character",
    thenJS="character",
    thenifJS="list",
    elseJS="character",
    elifJS="list"
  ),
  prototype(
    ifJS=character(),
    thenJS=character(),
    thenifJS=list(),
    elseJS=character(),
    elifJS=list()
  )
)

setValidity("rk.JS.ite", function(object){
    if(length(slot(object, "thenifJS")) > 1){
      stop(simpleError("Slot 'thenifJS' can only have one list element!"))
    } else {}
    if(length(slot(object, "thenifJS")) == 1){
      if(!inherits(slot(object, "thenifJS")[[1]], "rk.JS.ite")){
        stop(simpleError("Slot 'thenifJS' can only have one list element of class 'rk.JS.ite'!"))
      } else {}
    } else {}
    if(length(slot(object, "elifJS")) > 1){
      stop(simpleError("Slot 'elifJS' can only have one list element!"))
    } else {}
    if(length(slot(object, "elifJS")) == 1){
      if(!inherits(slot(object, "elifJS")[[1]], "rk.JS.ite")){
        stop(simpleError("Slot 'elifJS' can only have one list element of class 'rk.JS.ite'!"))
      } else {}
    } else {}
  return(TRUE)
})
