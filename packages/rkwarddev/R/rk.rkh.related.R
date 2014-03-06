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


#' Create XML "related" node for RKWard help pages
#'
#' @param ... Objects of class \code{XiMpLe.node}. They must all have the name "link".
#' @param text Character string, the text to be displayed.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' package.link <- rk.rkh.link("Spice")
#' plugin.related <- rk.rkh.related(package.link)
#' cat(pasteXML(plugin.related))

rk.rkh.related <- function(..., text=NULL){
  links <- list(...)

  # check the node names and allow only valid ones
  li.elements <- sapply(child.list(links), function(this.node){
      if(!identical(slot(this.node, "name"), "link")){
        stop(simpleError(paste0("Invalid XML nodes for links section: ", this.node@name)))
      } else {
        li.element <- XMLNode("li", .children=child.list(this.node, empty=FALSE))
      }
      return(li.element)
    })

  ul <- XMLNode("ul", .children=child.list(li.elements, empty=FALSE))

  node <- XMLNode("related", text, ul)

  return(node)
}
