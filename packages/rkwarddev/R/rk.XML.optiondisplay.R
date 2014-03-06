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


#' Create XML node "optiondisplay" for RKWard plugins
#' 
#' This node is only allowed once insinde the \code{<content>} node of an \code{<optionset>}.
#'
#' @note The \code{<optionset>} node was introduced with RKWard 0.6.1, please set the dependencies
#'    of your component/plugin accordingly.
#'
#' @param index Logical, whether to show a column with a numeric index in the optiondisplay.
#' @param id.name Character string, a unique ID for this plugin element (optional).
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.optionset]{rk.XML.optionset}},
#'    \code{\link[rkwarddev:rk.XML.optioncolumn]{rk.XML.optioncolumn}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
rk.XML.optiondisplay <- function(index=TRUE, id.name=NULL){

  if(is.null(id.name)){
    attr.list <- list()
  } else {
    attr.list <- list(id=id.name)
  }

  if(!isTRUE(index)){
    attr.list[["index"]] <- "false"
  } else {}

  node <- XMLNode("optiondisplay",
    attrs=attr.list)

  return(node)
}
