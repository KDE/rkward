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


#' Replace XiMpLe.node objects with their ID value
#' 
#' This function is a shortcut for \code{\link[rkwarddev:id]{id}} which sets some useful defaults
#' (\code{quote=TRUE, collapse=" + ", js=TRUE}). The abbreviation stands for "quote + plus".
#' 
#' @param ... One or several character strings and/or \code{XiMpLe.node} objects with plugin nodes,
#'   and/or objects of classes \code{rk.JS.arr} or \code{rk.JS.opt}, simply separated by comma.
#' @param .objects Alternative way of specifying objects, if you already have them as a list.
#' @return A character string.
#' @export
#' @seealso \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'    \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'    \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'    \code{\link[rkwarddev:echo]{echo}},
#'    \code{\link[rkwarddev:id]{id}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # an example checkbox XML node
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' qp("The variable name is: ", cbox1, "!")

qp <- function(..., .objects=list(...)){
  result <- id(quote=TRUE, collapse=" + ", js=TRUE, .objects=.objects)
  return(result)
}
