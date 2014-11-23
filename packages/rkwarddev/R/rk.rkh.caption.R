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


#' Create XML "caption" node for RKWard help pages
#'
#' This function will create a caption node for settings sections in RKWard help files.
#'
#' @param id Either a character string (the \code{id} of the XML element to explain),
#'    or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used).
#' @param title Character string, title to be displayed. If \code{NULL}, the \code{label}
#'    of the element will be shown.
#' @param i18n Either a character string or a named list with the optional element \code{context},
#'    to give some \code{i18n_context}
#'    information for this node. If set to \code{FALSE}, the attribute \code{title} will be renamed into 
#'    \code{noi18n_title}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}},
#'    \code{\link[rkwarddev:rk.rkh.settings]{rk.rkh.settings}}
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a sample frame
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' test.frame <- rk.XML.frame(test.dropdown, label="Some options")
#' # create the caption
#' test.caption <- rk.rkh.caption(test.frame)
#' cat(pasteXML(test.caption))

rk.rkh.caption <- function(id, title=NULL, i18n=NULL){
  # let's see if we need to extract IDs first
  attr.list <- list(id=check.ID(id))

  if(!is.null(title)){
    attr.list[["title"]] <- title
  } else {}

  # check for additional i18n info; if FALSE, "title" will be renamed to "noi18n_title"
  attr.list <- check.i18n(i18n=i18n, attrs=attr.list)

  node <- XMLNode(name="caption", attrs=attr.list)

  return(node)
}
