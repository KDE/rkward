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


#' Create XML node "external" for RKWard plugins
#'
#' @param id Character string, the ID of the new property.
#' @param default Character string, initial value of the property if not connected.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'    \code{\link[rkwarddev:rk.XML.covert]{rk.XML.convert}},
#'    \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}},
#'    \code{\link[rkwarddev:rk.XML.set]{rk.XML.set}},
#'    \code{\link[rkwarddev:rk.XML.switch]{rk.XML.switch}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.external <- rk.XML.external(id="ext_property", default="none")
#' cat(pasteXML(test.external))

rk.XML.external <- function(id, default=NULL){
  attr.list <- list(id=id)

  if(!is.null(default)){
    attr.list[["default"]] <- as.character(default)
  } else {}

  node <- XMLNode("external", attrs=attr.list)

  return(node)
}
