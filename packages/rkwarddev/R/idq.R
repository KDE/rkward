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

#' Get a quoted element ID
#' 
#' This is actually a convenience wrapper for \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}}
#' and returns the XML ID of XiMpLe nodes in quoted format, optionally with an attached modifier.
#' 
#' You can use this function to write almost literal JavaScript code, but still be able to extract IDs from
#' generated R objects.
#' 
#' @note You should always nest this function inside an \code{\link[rkwarddev:id]{id}} call (or a similar wrapper)
#'   to prevent \code{rk.paste.JS} from appending newline characters -- see the example section.
#' 
#' @param obj An object of class \code{XiMpLe.node} containig an ID to extract.
#' @param modifiers A character vector with modifiers you'd like to apply to the XML node property.
#' @param check.modifiers Logical, if \code{TRUE} the given modifiers will be checked for validity. Should only be
#'    turned off if you know what you're doing.
#' @return A character string.
#' @export
#' @expamples
#' myCheckbox <- rk.XML.cbox("Check for action")
#' rk.paste.JS(id("var x = getBoolean(", idq(myCheckbox, modifiers="state"), ");"))

idq <- function(obj, modifiers=NULL, check.modifiers=TRUE, quote="\""){
  # first give the object to rk.JS.vars(), as this already does all the modifier checking magic etc.
  # rk.JS.vars() should return the variable object in the vars slot as a one item list
  varObj <- slot(rk.JS.vars(obj, modifiers=modifiers, check.modifiers=check.modifiers), "vars")[[1]]
  obj.XML.var <- slot(varObj, "XML.var")
  obj.modifiers <- slot(varObj, "modifiers")
  
  result <- paste0(quote, slot(varObj, "XML.var"))

  if(length(obj.modifiers) > 0){
    result <- paste0(result, ".", paste0(unlist(obj.modifiers), collapse="."), quote)
  } else {
    result <- paste0(result, quote)
  }

  return(result)
}
