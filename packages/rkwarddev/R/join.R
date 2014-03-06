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


#' Generate JavaScript to join an array object
#'
#' This function pastes an object of class \code{rk.JS.arr} similar to \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
#' but was specifically written for elements like \code{<optionset>} or \code{<matrix>}, whose values must be queried
#' by \code{getList()} rather than \code{getValue()}. This means, the resulting variable is already an array an merely
#' needs to be joined in as R code output (e.g., an \code{<optioncolumn>}).
#'
#' @param var Either a character string (the name of the variable to combine to a vector or list),
#'    or an object of class \code{XiMpLe.node} (whose ID will be extracted and used). Also
#'    accepts objects of class \code{rk.JS.arr}.
#' @param by Character string by which the values ought to be joined.
#' @return An object of class \code{rk.JS.echo}.
#' @export
#' @seealso \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
#'    \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'    \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'    \code{\link[rkwarddev:echo]{echo}},
#'    \code{\link[rkwarddev:id]{id}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
# @examples

join <- function(var, by="\", \""){

  if(inherits(var, "rk.JS.arr")){
    arr.name  <- slot(object, "opt.name")
  } else {
    arr.name <- id(var)
  }

  JS.join <- new("rk.JS.echo",
      value=paste0(camelCode(arr.name), ".join(", qp(by) ,")")
    )

  return(JS.join)
}
