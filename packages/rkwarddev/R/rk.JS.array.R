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


#' Create a simple JavaScript array
#'
#' If you need to combine multiple options (like values of several checkboxes) into one vector or list,
#' this function can help with that task. All relevant variables will become part of an array and
#' then joined into the desired argument type.
#'
#' @param option A character string, naming, e.g., an option of an R function which should be
#'    constructed from several variables.
#' @param variables A list with either character strings (the names of the variables to combine to a vector or list),
#'    or objects of class \code{XiMpLe.node} with plugin XML nodes (whose ID will be extracted and used).
#' @param funct Character string, name of the R function to be called to combine the options, e.g. "list" for \code{list()},
#'    or "c" for \code{c()}.
#' @param var.prefix A character string. sets a global string to be used as a prefix for the JS variable names.
#' @param quote Logical, if \code{TRUE}, the values will be quoted in the resulting R code (might be neccessary
#'    for character values).
#' @param opt.sep Character string, will be printed in the resulting R code before the option name.
#' @return An object of class \code{rk.JS.arr}.
#' @export
#' @seealso \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
#'    \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'    \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'    \code{\link[rkwarddev:echo]{echo}},
#'    \code{\link[rkwarddev:id]{id}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # create three checkboxes for independent options
#' checkA <- rk.XML.cbox(label="Run Test A", value="A")
#' checkB <- rk.XML.cbox(label="Run Test B", value="B")
#' checkC <- rk.XML.cbox(label="Run Test C", value="C")
#' # combine them into one list of options via JavaScript
#' rk.JS.array("run.tests", variables=list(checkA, checkB, checkC), funct="list")

rk.JS.array <- function(option, variables=list(), funct="c", var.prefix=NULL, quote=FALSE, opt.sep=", "){
  arr.name <- camelCode(c("arr", option))
  opt.name <- camelCode(c("opt", option))

  JS.array <- new("rk.JS.arr",
    arr.name=arr.name,
    opt.name=opt.name,
    IDs=check.ID(variables),
    variables=unlist(sapply(child.list(variables), function(this.var){get.JS.vars(
              JS.var=this.var,
              JS.prefix=var.prefix,
              names.only=TRUE)
          })),
    funct=funct,
    quote=quote,
    option=option,
    opt.sep=opt.sep
  )

  return(JS.array)
}
