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


#' Create XML "attribute" node for RKWard plugins
#'
#' This function will create a attribute node for component sections in .pluginmap files.
#' Only meaningful for import plugins.
#'
#' @param id Either a character string (the \code{id} of the property whose attribute should be set),
#'    or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used).
#' @param value Character string, new value for the attribute.
#' @param label Character string, label associated with the attribute.
#' @param i18n Either a character string or a named list with the optional element \code{context},
#'    to give some \code{i18n_context} information for this node. If set to \code{FALSE},
#'    the attribute \code{label} will be renamed into \code{noi18n_label}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.components]{rk.XML.components}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.checkbox <- rk.XML.cbox(label="foo", value="foo1", chk=TRUE)
#' # re-set the attribute
#' test.attribute <- rk.XML.attribute(test.checkbox, value="bar2", label="bar")
#' cat(pasteXML(test.attribute))

rk.XML.attribute <- function(id, value=NULL, label=NULL, i18n=NULL){
  # let's see if we need to extract IDs first
  attr.list <- list(id=check.ID(id))

  if(all(is.null(value), is.null(label))){
    stop(simpleError("You must at least specity either one of 'value' or 'label'!"))
  } else {}
  
  if(!is.null(value)){
    attr.list[["value"]] <- value
  } else {}
  if(!is.null(label)){
    attr.list[["label"]] <- label
  } else {}

  # check for additional i18n info; if FALSE, "label" will be renamed to "noi18n_label"
  attr.list <- check.i18n(i18n=i18n, attrs=attr.list)
  
  node <- XMLNode("attribute", attrs=attr.list)

  return(node)
}
