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


#' Create XML node "row" for RKWard plugins
#'
#' @param ... Objects of class \code{XiMpLe.node}.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the objects in \code{...}.
#'    If \code{NULL}, no ID will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.checkboxes <- rk.XML.row(rk.XML.col(
#'     rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
#'     rk.XML.cbox(label="bar", val="bar2")))
#' cat(pasteXML(test.checkboxes))

rk.XML.row <- function(..., id.name="auto"){
  nodes <- list(...)

  if(identical(id.name, "auto")){
    # try autogenerating some id
    attr.list <- list(id=auto.ids(node.soup(nodes), prefix=ID.prefix("row"), chars=10))
  } else if(is.null(id.name)){
    attr.list <- list()
  } else {
    attr.list <- list(id=id.name)
  }

  row <- XMLNode("row",
      attrs=attr.list,
      .children=child.list(nodes, empty=FALSE)
    )

  return(row)
}
