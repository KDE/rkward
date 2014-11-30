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


#' Create XML "snippets" node for RKWard plugins
#'
#' This function will create a snippets node for the document section, with optional child nodes
#' \code{<snippet>} and \code{<include>}.
#'
#' @param ... Objects of class \code{XiMpLe.node}. Accepts only \code{<snippet>} and \code{<include>}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}}
#'    \code{\link[rkwarddev:rk.XML.snippet]{rk.XML.snippet}},
#'    \code{\link[rkwarddev:rk.XML.include]{rk.XML.include}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a formula section with varselector and varslots
#' test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
#' # define the snippets section
#' test.snippet <- rk.XML.snippet(test.formula)
#' test.snippets <- rk.XML.snippets(test.snippet)
#' cat(pasteXML(test.snippets))

rk.XML.snippets <- function(...){
  nodes <- list(...)

  # check the node names and allow only valid ones
  valid.child("snippets", children=nodes)

  node <- XMLNode("snippets", .children=child.list(nodes, empty=FALSE))

  return(node)
}
