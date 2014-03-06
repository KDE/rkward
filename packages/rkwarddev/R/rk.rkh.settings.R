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


#' Create XML "settings" node for RKWard help pages
#'
#' This function will create a settings node for the document section, with optional child nodes "setting" and "caption".
#'
#' @param ... Objects of class \code{XiMpLe.node}. They must all have the name "setting" or "caption".
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}},
#'    \code{\link[rkwarddev:rk.rkh.setting]{rk.rkh.setting}},
#'    \code{\link[rkwarddev:rk.rkh.caption]{rk.rkh.caption}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a sample frame
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' test.frame <- rk.XML.frame(test.dropdown, label="Some options")
#' # create the caption
#' test.caption <- rk.rkh.caption(test.frame)
#' test.setting <- rk.rkh.setting(test.dropdown, text="Chose one of the options.")
#' test.settings <- rk.rkh.settings(list(test.caption, test.setting))
# cat(pasteXML(test.settings))

rk.rkh.settings <- function(...){
  nodes <- list(...)

  # check the node names and allow only valid ones
  valid.child("settings", children=nodes)

  node <- XMLNode("settings", .children=child.list(nodes, empty=FALSE))

  return(node)
}
