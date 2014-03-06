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


#' Create XML "snippet" node for RKWard plugins
#'
#' This function will create a snippet node for snippets sections.
#'
#' @param ... Objects of class \code{XiMpLe.node}.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the tag names and
#'    IDs of the given nodes.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.snippets]{rk.XML.snippets}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a formula section with varselector and varslots
#' test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
#' # define the snippet
#' test.snippet <- rk.XML.snippet(test.formula)
#' cat(pasteXML(test.snippet))

rk.XML.snippet <- function(..., id.name="auto"){
  nodes <- list(...)

  if(identical(id.name, "auto")){
    # try autogenerating some id
    id.name <- auto.ids(node.soup(nodes), prefix=ID.prefix("snippet"), chars=10)
  } else if(is.null(id.name)){
    stop(simpleError("Snippets need an ID!"))
  } else {}

  node <- XMLNode("snippet",
      attrs=list(id=id.name),
      .children=child.list(nodes, empty=FALSE)
    )

  return(node)
}
