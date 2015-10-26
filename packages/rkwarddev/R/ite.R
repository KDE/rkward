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


#' Generate JavaScript if/then/else constructs
#' 
#' This function is very similar to \code{\link[base:ifelse]{ifelse}}. If you would like to use if conditions
#' directly, have a look at the \code{\link[rkwarddev:js]{js}} wrapper instead.
#' 
#' @param ifjs Either a character string to be placed in the brackets of an \code{if()} statement,
#'    or an object of class \code{XiMpLe.node}. \code{rk.JS.arr} or \code{rk.JS.opt} (whose identifier will be used).
#' @param thenjs Either a character string, the code to be executed in case the \code{if()} statement is true,
#'    or an object of class \code{XiMpLe.node}. \code{rk.JS.arr} or \code{rk.JS.opt} (whose identifier will be used).
#'    The latter is especially useful in combination with \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}}.
#'    You can also give another object of class \code{rk.JS.ite}.
#' @param elsejs Like \code{thenjs}, the code to be executed in case the \code{if()} statement is not true.
#' @return An object of class \code{rk.JS.ite}
#' @include 00_class_03_rk.JS.ite.R
#' @seealso \code{\link[rkwarddev:js]{js}},
#'    \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
#'    \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'    \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'    \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'    \code{\link[rkwarddev:echo]{echo}},
#'    \code{\link[rkwarddev:id]{id}},
#'    \code{\link[rkwarddev:qp]{qp}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' # first create an example checkbox XML node
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' # now some JavaScript generation
#' ite(cbox1, echo("bar <- \"", cbox1, "\""), echo("bar <- NULL"))

ite <- function(ifjs, thenjs, elsejs=NULL){
  #check for recursion
  if(inherits(thenjs, "rk.JS.ite")){
    thenifJS <- list(thenjs)
    thenjs <- ""
  } else {
    thenifJS <- list()
  }
  if(inherits(elsejs, "rk.JS.ite")){
    elifJS <- list(elsejs)
    elsejs <- ""
  } else {
    elifJS <- list()
    if(is.null(elsejs)){
      elsejs <- ""
    } else {}
  }
  result <- new("rk.JS.ite",
    ifJS=id(ifjs, js=TRUE),
    thenJS=id(thenjs, js=TRUE),
    thenifJS=thenifJS,
    elseJS=elsejs,
    elifJS=elifJS
  )
  return(result)
}
