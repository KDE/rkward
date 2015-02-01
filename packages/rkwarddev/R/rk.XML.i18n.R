# Copyright 2010-2015 Meik Michalke <meik.michalke@hhu.de>
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


#' Create XML node "i18n" for RKWard plugins
#'
#' @param label Character string, the label which is to be translated.
#' @param id.name Character string, a unique ID for the new property.
#'    If \code{"auto"}, an ID will be generated automatically from the label.
#' @param i18n Either a character string or a named list with the optional elements \code{context}
#'    or \code{comment}, to give some \code{i18n_context} information for this node.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}}
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.i18n <- rk.XML.i18n(label="Label test")

rk.XML.i18n <- function(label, id.name="auto", i18n=NULL){

  if(identical(id.name, "auto")){
    # try autogenerating some id
    attr.list <- list(id=auto.ids(label, prefix=ID.prefix("i18n", length=4), chars=10))
  } else {
    attr.list <- list(id=id.name)
  }

  attr.list[["label"]] <- label

  # check for additional i18n info
  attr.list <- check.i18n(i18n=i18n, attrs=attr.list)

  node <- check.i18n(
    i18n=i18n,
    node=XMLNode("i18n", attrs=attr.list),
    comment=TRUE
  )

  return(node)
}
