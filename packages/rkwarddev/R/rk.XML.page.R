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


#' Create XML "page" node for RKWard plugins
#'
#' This function will create a page node for wizard sections, with optional child nodes "browser", "checkbox",
#' "column", "copy", "dropdown", "formula", "frame", "input", "page", "radio", "row", "saveobject",
#' "select", "spinbox", "stretch", "tabbook", "text", "valueselector", "valueslot", "varselector" and "varslot".
#' @param ... Objects of class \code{XiMpLe.node}.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the objects in \code{...}.
#'    If \code{NULL}, no ID will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.wizard]{rk.XML.wizard}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a checkbox for the actual dialog
#' test.cbox1 <- rk.XML.cbox(label="More than 30 subjects", val="true")
#' # define the wizard
#' test.text <- rk.XML.text("Did you test more than 30 subjects?")
#' test.copy <- rk.XML.copy(id=test.cbox1)
#' test.wizard <- rk.XML.wizard(rk.XML.page(test.text, test.copy))
#' cat(pasteXML(test.wizard))

rk.XML.page <- function(..., id.name="auto"){
  nodes <- list(...)

  # check the node names and allow only valid ones
  valid.child("page", children=nodes, section="page/wizard")

  if(identical(id.name, "auto")){
    # try autogenerating some id
    attr.list <- list(id=auto.ids(node.soup(nodes), prefix=ID.prefix("page"), chars=10))
  } else if(is.null(id.name)){
    attr.list <- list()
  } else {
    attr.list <- list(id=id.name)
  }

  node <- XMLNode("page",
      attrs=attr.list,
      .children=child.list(nodes, empty=FALSE)
    )

  return(node)
}
