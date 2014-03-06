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


#' Create XML "components" node for RKWard plugins
#'
#' This function will create a components node for a .pluginmap file, with mandatory child nodes "component".
#'
#' @param ... Objects of class \code{XiMpLe.node}. They must all have the name "component".
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.pluginmap]{rk.XML.pluginmap}},
#'    \code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
#' test.components <- rk.XML.components(test.component)
#' cat(pasteXML(test.components))

rk.XML.components <- function(...){
  nodes <- list(...)

  # check the node names and allow only valid ones
  valid.child("components", children=nodes)

  node <- XMLNode("components", .children=child.list(nodes, empty=FALSE))

  return(node)
}
