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


#' Create XML node "set" for RKWard plugins
#'
#' @param id Either a character string (the \code{id} of the property whose value should be set),
#'    or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used).
#' @param set Character string, a valid modifier.
#' @param to Character string or logical, the value the property should be set to.
#' @param check.modifiers Logical, if \code{TRUE} the given modifiers will be checked for validity. Should only be
#'    turned off if you know what you're doing.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'    \code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'    \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}},
#'    \code{\link[rkwarddev:rk.XML.set]{rk.XML.set}},
#'    \code{\link[rkwarddev:rk.XML.switch]{rk.XML.switch}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.set <- rk.XML.set(id="input_foo", set="required", to=TRUE)
#' cat(pasteXML(test.set))

rk.XML.set <- function(id, set=NULL, to, check.modifiers=TRUE){

  if(length(id) > 1 | length(to) > 1){
    stop(simpleError("'id' and 'to' must be of length 1!"))
  } else {}

  # check for container objects
  id <- stripXML(id)

  # let's see if we need to extract IDs first
  prop.id <- check.ID(id)

  if(!is.null(set)){
    if(isTRUE(check.modifiers)){
      modif.validity(id, modifier=set, ignore.empty=TRUE, warn.only=FALSE, bool=TRUE)
    } else {}
    prop.id <- paste(prop.id, set, sep=".")
  } else {}

  attr.list <- list(id=as.character(prop.id))

  if(is.logical(to)){
    attr.list[["to"]] <- ifelse(isTRUE(to), "true", "false")
  } else {
    attr.list[["to"]] <- as.character(to)
  }

  node <- XMLNode("set", attrs=attr.list)

  return(node)
}
