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


#' Validate S4 objects of XiMpLe XML classes
#' 
#' Check whether objects of class \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#' or \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}} have valid child nodes.
#' 
#' XiMpLe can't handle DOM specifications yet, but this method can be used to construct
#' validation schemes.
#'
#' @param obj An object of class \code{XiMpLe.doc} or \code{XiMpLe.node}. If \code{parent=NULL}, this object
#'    will be checked for validity, including its child nodes. If \code{parent} is either a character string
#'    or another XiMpLe node, it will be checked whether \code{obj} is a valid child node of \code{parent}.
## TODO: validity class objects
#' @param validity A list with validity information.
#' @param parent Either a character string (name of the parent node) or a XiMpLe node, whose name will be used
#'    as name of the parent node.
#' @param warn Logical, whether invalid objects should cause a warning or stop with an error.
#' @param section Either a character string (name of the section) or a XiMpLe node, whose name will be used
#'    as name of the XML section this check refers to. This is only relevant for warnings and error messages,
#'    in case you want to use something different than the actual parent node name.
#' @return Returns \code{TRUE} if tests pass, and depending on the setting of \code{warn} either \code{FALSE} or
#'    an error if a test fails.
#' @aliases
#'    validXML,-methods
#'    validXML,XiMpLe.doc-method
#'    validXML,XiMpLe.node-method
#'    validXML,XiMpLe.XML-method
#' @seealso  
#'    \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#'    \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#' @keywords methods
#' @docType methods
#' @export
#' @rdname validXML
#' @include 00_class_01_XiMpLe.node.R
#' @include 00_class_02_XiMpLe.doc.R
setGeneric("validXML", function(obj, validity, parent=NULL, children=TRUE, attributes=TRUE, warn=FALSE, section=parent){standardGeneric("validXML")})

#' @rdname validXML
#' @export
setMethod("validXML", signature(obj="XiMpLe.XML"), function(obj, validity, parent=NULL, children=TRUE, attributes=TRUE, warn=FALSE, section=parent){
  childValidity <- NULL
  if(!is.XiMpLe.validity(validity)){
    stop(simpleError(paste0(
      "Invalid value for \"validity\": Got class ",
      class(valid),
      ", should be XiMpLe.validity!"))
    )
  }
  # see if we're checking the parent node or child node for a given parent
  if(is.null(parent)){
    parentName <- XMLName(obj)
    # are there any children to check in the first place?
    nodeChildren <- XMLChildren(obj)
    if(length(nodeChildren) == 0){
      children <- FALSE
    } else {
      childValidity <- all(sapply(
        nodeChildren,
        function(thisChild){
          validXML(thisChild, validity=validity, parent=parentName, children=children, attributes=attributes, warn=warn, section=parentName)
        }
      ))
      children <- FALSE
    }
  } else if(is.XiMpLe.node(parent)){
    parentName <- XMLName(parent)
  } else if(is.character(parent) & length(parent) == 1){
    parentName <- parent
  } else {
    stop(simpleError(paste0(
      "Invalid value for \"parent\": Got class \"",
      class(parent),
      "\", should be XiMpLe.node or single character string!"))
    )
  }

  if(is.null(section)){
    section <- parentName
  } else if(is.XiMpLe.node(section)){
    section <- XMLName(section)
  } else if(!is.character(section) | length(section) != 1){
    stop(simpleError(paste0(
      "Invalid value for \"section\": Got class \"",
      class(section),
      "\", should be XiMpLe.node or single character string!"))
    )
  } else {}
  
  
  ## more checks

  if(isTRUE(children)){
    childValidity <- valid.child(parent=parentName, children=obj, validity=validity, warn=warn, section=section)
  } else {}

  return(childValidity)
})
