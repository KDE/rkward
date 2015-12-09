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


#' Combine several options in one JavaScript variable
#' 
#' @param var Character string, name of the JavaScript variable to use in the script code.
#' @param ... A list of objects of class \code{rk.JS.ite} (see \code{\link[rkwarddev:ite]{ite}}).
#'    Use the \code{thenjs} element to define only the value to add to the option, without commas
#'    (e.g., \code{"paired=TRUE"} or \code{qp("conf.level=\"", conflevel, "\"")}.
#' @param collapse Character string, how all options should be concatenated on the R code level
#'    (if \code{array=FALSE}), or how \code{option} should be added to the generated R code. Hint:
#'    To place each option in a new line with tab indentation, set \code{collapse=",\\n\\t"}.
#' @param option A character string, naming, e.g., an option of an R function which should be
#'    constructed from several variables. Only used if \code{array=TRUE}.
#' @param funct Character string, name of the R function to be called to combine the options,
#'    e.g. "list" for \code{list()}, or "c" for \code{c()}. Set to \code{NULL} to drop.
#'    Only used if \code{array=TRUE}.
#' @param array Logical, if \code{TRUE} will generate the options as an array, otherwise in one
#'    concatenated character string (probably only useful for mandatory options).
#' @param opt.sep Character string, will be printed in the resulting R code before the option name.
#' @param .ite Like \code{...}, if you have all objects in a list already.
#' @return An object of class \code{rk.JS.opt}, use \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}}
#'    on that.
#' @seealso
#'    \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' # create two checkboxes for independent options
#' checkA <- rk.XML.cbox(label="Run Test A", value="A")
#' checkB <- rk.XML.cbox(label="Run it fast", value="true")
#' # combine them into one JavaScript options variable
#' rk.JS.options("optionsTestA",
#'   ite(checkA, qp("test=\"", checkA, "\"")),
#'   ite(checkB, "fast=TRUE")
#' )

rk.JS.options <- function(var, ..., collapse=", ", option=NULL, funct=NULL, array=TRUE, opt.sep=", ", .ite=list(...)){
  if(is.null(option)){
    option <- ""
  } else {}
  if(is.null(funct)){
    funct <- ""
  } else {}

  result <- new("rk.JS.opt",
    var.name=var,
    opt.name=option,
    collapse=collapse,
    ifs=.ite,
    array=array,
    funct=funct,
    opt.sep=opt.sep
  )

  return(result)
}