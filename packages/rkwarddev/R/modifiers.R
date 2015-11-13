# Copyright 2015 Meik Michalke <meik.michalke@hhu.de>
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

#' Get all valid modifiers for a given XML node
#' 
#' In case you want to see which modifiers are definied for a certain XML node,
#' just call this helper function.
#' 
#' @param obj An object of class \code{XiMpLe.node}, or a character string containing
#'   the XML node name you're interested in. If set to \code{"all"} returns all defined
#'   modifiers.
#' @return A character vector of valid modifiers.
#' @export
#' @expamples
#' myCheckbox <- rk.XML.cbox("Check for action")
#' modifiers(myCheckbox)

modifiers <- function(obj="all"){
  if(is.XiMpLe.node(obj)){
    nodeName <- XMLName(obj)
  } else if(is.character(obj) & length(obj) == 1){
    nodeName <- obj
  } else {
    stop(simpleError("'obj' must either be a XiMpLe node or a character string!"))
  }

  if(identical(nodeName, "all")){
    result <- all.valid.modifiers
  } else if(nodeName %in% names(all.valid.modifiers)){
      result <- all.valid.modifiers[c("all", nodeName)]
  } else {
    stop(simpleError(paste0("There are no modifiers defined for node '", nodeName,"'!")))
  }

  return(result)
}
