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


#' Generate JavaScript echo command call
#' 
#' This function will take several elements, either character strings, or objects of class \code{XiMpLe.node}
#' which hold an XML node of some plugin GUI definition, or objects of classes \code{rk.JS.arr} or \code{rk.JS.opt}.
#' From those, it will generate a ready-to-run JavaScript \code{echo();} call from it.
#' 
#' @param ... One or several character strings and/or \code{XiMpLe.node} objects with plugin nodes,
#'    and/or objects of classes \code{rk.JS.arr} or \code{rk.JS.opt}, simply separated by comma.
#' @param newline Character string, can be set to e.g. \code{"\n"} to force a newline after the call.
#' @return A character string.
#' @seealso \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'    \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'    \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'    \code{\link[rkwarddev:ite]{ite}},
#'    \code{\link[rkwarddev:id]{id}},
#'    \code{\link[rkwarddev:id]{qp}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' echo("bar <- \"", cbox1, "\"")

echo <- function(..., newline=""){
  ID.content <- qp(...)
  result <- paste0("echo(", ID.content, ");", newline)
  return(result)
}

## internal class rk.JS.echo
# this is a quick fix to be able to add values into echo() without quotes
setClass("rk.JS.echo",
  representation=representation(
    value="character"
  ),
  prototype(
    value=character()
  )
)
