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


#' Create XML "label" node for RKWard help pages
#'
#' @param id Either a character string (the \code{id} name of the element in the plugin, of
#'    which to copy the \code{label} attribute), or an object of class \code{XiMpLe.node}
#'    (whose \code{id} will be extracted and used).
#' @param i18n Either a character string or a named list with the optional elements \code{context}
#'    or \code{comment}, to give some \code{i18n_context} information for this node.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.cbox <- rk.XML.cbox(label="foo", value="bar")
#' (test.label <- rk.rkh.label(test.cbox))

rk.rkh.label <- function(id, i18n=NULL){
  # check for additional i18n info
  attr.list <- check.i18n(i18n=i18n, attrs=list(id=check.ID(id)))

  node <- check.i18n(
    i18n=i18n,
    node=XMLNode("label", attrs=attr.list),
    comment=TRUE
  )

  return(node)
}
