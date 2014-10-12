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


#' Create XML node "insert" for RKWard plugins
#'
#' This function creates an insert node to use snippets.
#'
#' @param snippet Either a character string (the \code{id} of the snippet to be inserted),
#'    or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used; must be a snippet!).
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.snippets]{rk.XML.snippets}},
#'    \code{\link[rkwarddev:rk.XML.snippet]{rk.XML.snippet}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a formula section with varselector and varslots
#' test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
#' # define the snippet
#' test.snippet <- rk.XML.snippet(test.formula)
#' # now to insert the snippet
#' test.insert <- rk.XML.insert(test.snippet)
#' cat(pasteXML(test.insert))

rk.XML.insert <- function(snippet){
  if(length(snippet) > 1){
    stop(simpleError("'snippet' must be of length 1!"))
  } else {}

  if(is.XiMpLe.node(snippet)){
    valid.parent(parent="snippet", node=snippet, warn=FALSE, see="rk.XML.snippet")
  } else {}

  # let's see if we need to extract IDs first
  attr.list <- list(snippet=check.ID(snippet))

  node <- XMLNode("insert", attrs=attr.list)

  return(node)
}
