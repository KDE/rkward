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


#' Create XML wizard section for RKWard plugins
#'
#' This function will create a wizard section with optional child nodes "browser", "checkbox",
#' "column", "copy", "dropdown", "embed", "formula", "frame", "include", "input", "insert",
#' "page", "preview", "radio", "row", "saveobject", "select", "spinbox", "stretch", "tabbook", "text",
#' "valueselector", "valueslot", "varselector" and "varslot".
#'
#' @param ... Objects of class \code{XiMpLe.node}
#' @param label Character string, a text label for this plugin element.
#' @param recommended Logical, whether the wizard should be the recommended interface (unless the user has configured
#'    RKWard to default to a specific interface).
#' @param i18n Either a character string or a named list with the optional elements \code{context}
#'    or \code{comment}, to give some \code{i18n_context} information for this node. If set to \code{FALSE},
#'    the attribute \code{label} will be renamed into \code{noi18n_label}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}},
#'    \code{\link[rkwarddev:rk.plugin.skeleton]{rk.plugin.skeleton}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a checkbox for the actual dialog
#' test.cbox1 <- rk.XML.cbox(label="More than 30 subjects", val="true")
#' # define the wizard
#' test.text <- rk.XML.text("Did you test more than 30 subjects?")
#' test.copy <- rk.XML.copy(id=test.cbox1)
#' test.wizard <- rk.XML.wizard(rk.XML.page(list(test.text, test.copy)))
#' cat(pasteXML(test.wizard))

rk.XML.wizard <- function(..., label=NULL, recommended=FALSE, i18n=NULL){
  nodes <- list(...)

  # check the node names and allow only valid ones
  valid.child("wizard", children=nodes)

  if(!is.null(label)){
    attr.list <- list(label=label)
  } else {
    attr.list <- list()
  }

  if(isTRUE(recommended)){
    attr.list[["recommended"]] <- "true"
  } else {}
  
  # check for additional i18n info; if FALSE, "label" will be renamed to "noi18n_label"
  attr.list <- check.i18n(i18n=i18n, attrs=attr.list)

  node <- check.i18n(
    i18n=i18n,
    node= XMLNode("wizard", attrs=attr.list, .children=child.list(nodes, empty=FALSE)),
    comment=TRUE
  )

  return(node)
}
