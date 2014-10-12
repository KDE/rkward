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


#' Create XML "entry" node for RKWard plugins
#'
#' This function will create a entry node for menu sections in .pluginmap files.
#' 
#' @param component A "component" object of class \code{XiMpLe.node}, or an ID.
#' @param index Integer number to influence the level of menu placement.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.menu]{rk.XML.menu}},
#'    \code{\link[rkwarddev:rk.XML.hierarchy]{rk.XML.hierarchy}},
#'    \code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
#'    \code{\link[rkwarddev:rk.XML.components]{rk.XML.components}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
#' test.entry <- rk.XML.entry(test.component)
#' cat(pasteXML(test.entry))

rk.XML.entry <- function(component, index=-1){
  if(length(component) > 1){
    stop(simpleError("'component' must be of length 1!"))
  } else {}

  # check the node names and allow only valid ones
  if(is.XiMpLe.node(component)){
    valid.parent(parent="component", node=component, warn=FALSE, see="rk.XML.component")
  } else {}

  attr.list <- list(component=check.ID(component))

  if(!identical(index, -1)){
    attr.list[["index"]] <- index
  } else {}

  node <- XMLNode("entry", attrs=attr.list)

  return(node)
}
