# Copyright 2015 Meik Michalke <meik.michalke@hhu.de>
#
# This file is part of the R package XiMpLe.
#
# XiMpLe is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# XiMpLe is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with XiMpLe.  If not, see <http://www.gnu.org/licenses/>.


#' Class XiMpLe.validity
#'
#' Used for objects that describe valid child nodes and attributes of XiMpLe.nodes.
#' 
#' You should use \code{\link[XiMpLe:XMLValidity]{XMLValidity}} to create objects of this class.
#'
#' @slot children Named list of vectors or lists. The element name defines the parent node
#'   name and each character string a valid child node name. If a value is in turn of class XiMpLe.validity,
#'   this object will be used for recursive validation of deeper nodes.
#' @slot attrs Named list of vectors or lists. The element name defines the parent node
#'   name and each character string a valid attribute name. If a value is in turn of class XiMpLe.validity,
#'   this object will be used for recursive validation of deeper nodes.
#' @slot allChildren Character vector, names of globally valid child nodes for all nodes, if any.
#' @slot allAttrs Character vector, names of globally valid attributes for all nodes, if any.
#' @slot empty Character vector, names of nodes that must be empty nodes (i.e., no closing tag), if any.
#' @slot ignore Character vector, names of nodes that should be ignored, if any.
#' @name XiMpLe.validity,-class
#' @aliases XiMpLe.validity-class XiMpLe.validity,-class
#' @import methods
#' @keywords classes
#' @seealso
#'    \code{\link[XiMpLe:XMLValidity]{XMLValidity}},
#'    \code{\link[XiMpLe:validXML]{validXML}}
#' @rdname XiMpLe.validity-class
#' @export

setClass("XiMpLe.validity",
  representation=representation(
    children="list",
    attrs="list",
    allChildren="character",
    allAttrs="character",
    empty="character",
    ignore="character"
  ),
  prototype(
    children=list(),
    attrs=list(),
    allChildren=character(),
    allAttrs=character(),
    empty=character(),
    ignore=character()
  )
)

setValidity("XiMpLe.validity", function(object){
  obj.children <- slot(object, "children")
  obj.attrs <- slot(object, "attrs")

  for (thisChild in obj.children){
    if(is.list(thisChild)){
      # check for XiMpLe.validity object
      isValidity <- which(sapply(thisChild, is.XiMpLe.validity))
      if(length(isValidity) == 1){
        thisChild[[isValidity]] <- NULL
        thisChild <- unlist(thisChild)
      } else if(length(isValidity) > 1){
        stop(simpleError("Invalid object: all \"children\" can only have one value of class XiMpLe.validity for recursion!"))
      } else {}
    } else {}
    if(!is.character(thisChild)){
      stop(simpleError("Invalid object: all \"children\" must be of class character or XiMpLe.validity!"))
    } else {}
  }
  for (thisAttr in obj.attrs){
    if(!is.character(thisAttr)){
      stop(simpleError("Invalid object: all \"attrs\" must be of class character or XiMpLe.validity!"))
    } else {}
  }
  return(TRUE)
})
