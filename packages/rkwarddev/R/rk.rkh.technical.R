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


#' Create XML "technical" node for RKWard help pages
#'
#' @param text Character string, the text to be displayed.
#' @param i18n Either a character string or a named list with the optional elements \code{context}
#'    or \code{comment}, to give some \code{i18n_context} information for this node.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' plugin.technical <- rk.rkh.technical("<b>TODO</b>: Implement sandworm detector.")
#' cat(pasteXML(plugin.technical))

rk.rkh.technical <- function(text=NULL, i18n=NULL){
  if(is.null(text)){
    text <- ""
  } else {}

  # check for additional i18n info
  attr.list <- check.i18n(i18n=i18n, attrs=list())

  node <- check.i18n(
    i18n=i18n,
    node=XMLNode("technical", text, attrs=attr.list),
    comment=TRUE
  )

  return(node)
}
