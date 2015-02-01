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


#' Create node "varselector" for RKWard plugins
#'
#' @param label Character string, a text label for the variable selection slot.
#'    Must be set if \code{id.name="auto"}.
#' @param id.name Character vector, unique ID for this element.
#' @param i18n Either a character string or a named list with the optional elements \code{context}
#'    or \code{comment}, to give some \code{i18n_context} information for this node. If set to \code{FALSE},
#'    the attribute \code{label} will be renamed into \code{noi18n_label}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.varslot]{rk.XML.varslot}},
#'    \code{\link[rkwarddev:rk.XML.vars]{rk.XML.vars}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.varselector <- rk.XML.varselector("Select some vars")
#' cat(pasteXML(test.varselector))

rk.XML.varselector <- function(label=NULL, id.name="auto", i18n=NULL){
  if(identical(id.name, "auto")){
    ## if this ID generation get's changed, change it in rk.XML.vars(), too!
    attr.list <- list(id=auto.ids(label, prefix=ID.prefix("varselector", length=3)))
  } else if(!is.null(id.name)){
    attr.list <- list(id=id.name)
  } else {}

  if(!is.null(label)){
    attr.list[["label"]] <- label
  } else {
    if(identical(id.name, "auto")){
      stop(simpleError("If id.name=\"auto\", then 'label' must have a value!"))
    } else {}
  }

  # check for additional i18n info; if FALSE, "label" will be renamed to "noi18n_label"
  attr.list <- check.i18n(i18n=i18n, attrs=attr.list)

  node <- check.i18n(
    i18n=i18n,
    node=XMLNode("varselector", attrs=attr.list),
    comment=TRUE
  )

  return(node)
}
