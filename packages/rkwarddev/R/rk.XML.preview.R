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


#' Create XML node "preview" for RKWard plugins
#'
#' @param label A character string, text label for the preview checkbox.
#' @param i18n Either a character string or a named list with the optional elements \code{context}
#'    or \code{comment}, to give some \code{i18n_context} information for this node. If set to \code{FALSE},
#'    the attribute \code{label} will be renamed into \code{noi18n_label}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.preview <- rk.XML.preview("See a preview?")
#' cat(pasteXML(test.preview))

rk.XML.preview <- function(label="Preview", i18n=NULL){
  if(!identical(label, "Preview")){
    attr.list <- list(label=label)
  } else {
    attr.list <- list()
  }

  # check for additional i18n info; if FALSE, "label" will be renamed to "noi18n_label"
  attr.list <- check.i18n(i18n=i18n, attrs=attr.list)

  node <- check.i18n(
    i18n=i18n,
    node=XMLNode("preview", attrs=attr.list),
    comment=TRUE
  )

  return(node)
}
