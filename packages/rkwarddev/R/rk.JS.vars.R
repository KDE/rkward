# Copyright 2010-2015 Meik Michalke <meik.michalke@hhu.de>
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


#' Define variables in JavaScript code
#' 
#' @note To get a list of the implemented modifiers in this package see \code{\link[rkwarddev:modifiers]{modifiers}}.
#'
#' @param ... Either one or more character strings (the names of the variables to define),
#'    or objects of class \code{XiMpLe.node} with plugin XML nodes (whose ID will be extracted and used).
#' @param var.prefix A character string. will be used as a prefix for the JS variable names.
#' @param modifiers A character vector with modifiers you'd like to apply to the XML node property.
#' @param default Logical, if \code{TRUE} the default value (no special modifier) of the node will
#'    also be defined. Does nothing if \code{modifiers=NULL}.
#' @param join A character string, useful for GUI elements which accept multiple objects (e.g., multi-varslots).
#'    If \code{join} is something other than \code{""}, these objects will be collapsed into one string when pasted,
#'    joined by this string.
#' @param check.modifiers Logical, if \code{TRUE} the given modifiers will be checked for validity. Should only be
#'    turned off if you know what you're doing.
#' @param getter A character string, naming the JavaScript function which should be used to get the values in the
#'    actual plugin. Depending on the XML element, \code{"getString"}, \code{"getBool"} or \code{"getList"} can be
#'    useful alternatives. For backwards compatibility, the default is set to \code{"getValue"}.
#' @param guess.getter Logical, if \code{TRUE} try to get a good default getter function for JavaScript
#'    variable values.
#' @param object.name Logical, if \code{TRUE} the JS variable name will roughly match the R object name. If the
#'    object name contains dots, they will be removed and the JS name printed in camel code instead. Use this option
#'    with great caution and do not combine it with \code{\link[rkwarddev:rk.JS.scan]{rk.JS.scan}}, as it will likely result
#'    in unusable code. \code{rk.JS.scan} examines XML nodes and therefore does not know any R object names.
#' @return An object of class \code{rk.JS.var}.
#' @export
#' @seealso \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'    \code{\link[rkwarddev:echo]{echo}},
#'    \code{\link[rkwarddev:id]{id}},
#'    \code{\link[rkwarddev:modifiers]{modifiers}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # create three checkboxes
#' checkA <- rk.XML.cbox(label="Run Test A", value="A")
#' checkB <- rk.XML.cbox(label="Run Test B", value="B")
#' checkC <- rk.XML.cbox(label="Run Test C", value="C")
#' # define them by their ID in JavaScript
#' cat(rk.paste.JS(rk.JS.vars(list(checkA, checkB, checkC))))

rk.JS.vars <- function(..., var.prefix=NULL, modifiers=NULL, default=FALSE, join="", check.modifiers=TRUE,
  getter="getValue", guess.getter=FALSE, object.name=FALSE){
  variables <- child.list(list(...))
  if(isTRUE(object.name)){
    var.alist <- eval(substitute(alist(...)))
    JS.var.names <- lapply(
      1:length(variables),
      function(this.var.num){
        this.var <- var.alist[[this.var.num]]
        if(is.name(this.var)){
          return(deparse(this.var))
        } else if(is.character(this.var)){
          return(this.var)
        } else {
          # fall back to the original input if we can't clearly make sense of it here
          return(variables[[this.var.num]])
        }
      }
    )
  } else {}

  JS.vars <- new("rk.JS.var",
      vars=sapply(1:length(variables), function(this.var.num){
        this.var <- this.JS.var <- variables[[this.var.num]]
        if(isTRUE(object.name)){
          this.JS.var <- JS.var.names[[this.var.num]]
        } else {}
        get.JS.vars(
          JS.var=this.JS.var,
          XML.var=this.var,
          JS.prefix=var.prefix,
          modifiers=modifiers,
          default=default,
          join=join,
          check.modifiers=check.modifiers,
          getter=getter,
          guess.getter=guess.getter)
      })
    )

  return(JS.vars)
}
