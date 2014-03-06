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


#' Create XML node "frame" for RKWard plugins
#'
#' @param ... Objects of class \code{XiMpLe.node}.
#' @param label Character string, a text label for this plugin element.
#' @param checkable Logical, if \code{TRUE} the frame can be switched on and off.
#' @param chk Logical, if \code{TRUE} and \code{checkable=TRUE} the frame is checkable and active by default.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"} and a label was provided, an ID will be generated automatically from the label
#'    if presen, otherwise from the objects in the frame.
#'    If \code{NULL}, no ID will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @seealso
#'    \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' cat(pasteXML(rk.XML.frame(test.dropdown, label="Some options")))

rk.XML.frame <- function(..., label=NULL, checkable=FALSE, chk=TRUE, id.name="auto"){
  nodes <- list(...)

  if(!is.null(label)){
    attr.list <- list(label=label)
  } else {
    attr.list <- list()
  }

  if(isTRUE(checkable)){
    attr.list[["checkable"]] <- "true"
    if(!isTRUE(chk)){
      attr.list[["checked"]] <- "false"
    } else {}
  } else {}

  if(identical(id.name, "auto")){
    if(!is.null(label)){
      attr.list[["id"]] <- auto.ids(label, prefix=ID.prefix("frame"))
    } else {
      # try autogenerating some id
      attr.list[["id"]] <- auto.ids(node.soup(nodes), prefix=ID.prefix("frame"), chars=10)
    }
  } else if(!is.null(id.name)){
    attr.list[["id"]] <- id.name
  } else {}

  frame <- XMLNode("frame",
      attrs=attr.list,
      .children=child.list(nodes, empty=FALSE)
    )

  return(frame)
}
