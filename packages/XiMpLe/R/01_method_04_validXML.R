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
#' @note: If no \code{parent} is specified, \code{obj} will be checked recursively. If 
#'
#' @param obj An object of class \code{XiMpLe.doc} or \code{XiMpLe.node}. If \code{parent=NULL}, this object
#'    will be checked for validity, including its child nodes. If \code{parent} is either a character string
#'    or another XiMpLe node, it will be checked whether \code{obj} is a valid child node of \code{parent}.
#' @param validity An object of class \code{\link[XiMpLe:XiMpLe.validity-class]{XiMpLe.validity}},
#'    see \code{\link[XiMpLe:XMLValidity]{XMLValidity}}.
#' @param parent Either a character string (name of the parent node) or a XiMpLe node, whose name will be used
#'    as name of the parent node.
#' @param children Logical, whether child node names should be checked for validity.
#' @param attributes Logical, whether attributes should be checked for validity.
#' @param warn Logical, whether invalid objects should cause a warning or stop with an error.
#' @param section Either a character string (name of the section) or a XiMpLe node, whose name will be used
#'    as name of the XML section this check refers to. This is only relevant for warnings and error messages,
#'    in case you want to use something different than the actual parent node name.
#' @param caseSens Logical, whether checks should be case sensitive or not.
#' @return Returns \code{TRUE} if tests pass, and depending on the setting of \code{warn} either \code{FALSE} or
#'    an error if a test fails.
#' @aliases
#'    validXML,-methods
#'    validXML,XiMpLe.doc-method
#'    validXML,XiMpLe.node-method
#'    validXML,XiMpLe.XML-method
#' @seealso
#'    \code{\link[XiMpLe:validXML]{validXML}},
#'    \code{\link[XiMpLe:XMLValidity]{XMLValidity}},
#'    \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}, and
#'    \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#' @keywords methods
#' @docType methods
#' @export
#' @rdname validXML
#' @include 00_class_01_XiMpLe.node.R
#' @include 00_class_02_XiMpLe.doc.R
setGeneric("validXML", function(obj, validity=XMLValidity(), parent=NULL, children=TRUE, attributes=TRUE, warn=FALSE, section=parent, caseSens=TRUE){standardGeneric("validXML")})

#' @rdname validXML
#' @export
#' @examples
#' HTMLish <- XMLValidity(
#'    children=list(
#'      body=c("a", "p", "ol", "ul", "strong"),
#'      head=c("title"),
#'      html=c("head", "body"),
#'      li=c("a", "br", "strong"),
#'      ol=c("li"),
#'      p=c("a", "br", "ol", "ul", "strong"),
#'      ul=c("li")
#'    ),
#'    attrs=list(
#'      a=c("href", "name"),
#'      p=c("align")
#'    ),
#'    allChildren=c("!--"),
#'    allAttrs=c("id", "class"),
#'    empty=c("br")
#' )
#' # make XML object
#' validChildNodes <- XMLNode("html",
#'   XMLNode("head",
#'     XMLNode("!--", "comment always passes"),
#'     XMLNode("title", "test")
#'   ),
#'   XMLNode("body",
#'     XMLNode("p",
#'       XMLNode("a", "my link"),
#'       XMLNode("br"),
#'       "text goes on"
#'     )
#'   )
#' )
#' invalidChildNodes <- XMLNode("html",
#'   XMLNode("head",
#'     XMLNode("title", 
#'       XMLNode("body", "test")
#'     )
#'   )
#' )
#'
#' # do validity checks
#' # the first should pass
#' validXML(
#'   validChildNodes,
#'   validity=HTMLish
#' )
#' 
#' # now this one should cause a warning and return FALSE
#' validXML(
#'   invalidChildNodes,
#'   validity=HTMLish,
#'   warn=TRUE
#' )
setMethod("validXML", signature(obj="XiMpLe.XML"), function(obj, validity=XMLValidity(), parent=NULL, children=TRUE, attributes=TRUE,
  warn=FALSE, section=parent, caseSens=TRUE){
  childValidity <- attributeValidity <- emptyValidity <- NULL
  if(!is.XiMpLe.validity(validity)){
    stop(simpleError(paste0(
      "Invalid value for \"validity\": Got class ",
      class(validity),
      ", should be XiMpLe.validity!"))
    )
  }
  # two possibilities:
  # a) there's no "parent" value
  #    we're checking "obj" as the parent node itself
  #    - check attributes of "obj" directly
  #    - check child nodes of "obj" for valid node names
  #    - check if "obj" should be empty but is not
  #    - recursion: check attributes of child nodes etc.
  # b) "parent" is given
  #    we're checking "obj" as child node for a given parent
  #    - check if "obj" node name is valid for parent node
  #    - check attributes of "obj"
  #    - no recursion
  recursion <- FALSE
  if(is.null(parent)){
    parentName <- XMLName(obj)
    nodeChildren <- XMLChildren(obj)
    # check for violations of mandatory empty nodes
    emptyNodes <- slot(validity, "empty")
    if(!isTRUE(caseSens)){
      emptyNodes <- tolower(emptyNodes)
    } else {}
    if(parentName %in% emptyNodes){
      if(length(nodeChildren) > 0 | !identical(XMLValue(obj), character())){
        return.message <- paste0("Invalid XML node <", parentName, " />: Should be empty, but it isn't!")
        if(isTRUE(warn)){
          warning(return.message, call.=FALSE)
          emptyValidity <- FALSE
        } else {
          stop(simpleError(return.message))
        }
      } else {}
    } else {}
    recursion <- TRUE
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

  if(isTRUE(children)){
    if(isTRUE(recursion)){
      # are there any children to check in the first place?
      if(length(nodeChildren) > 0){
        childValidity <- all(sapply(
          nodeChildren,
          function(thisChild){
            # check child itself
            thisChildValidity <- valid.child(
              parent=parentName,
              children=thisChild,
              validity=validity,
              warn=warn,
              section=section,
              caseSens=caseSens
            )
            # check grandchildren
            grandChildValidity <- validXML(
              thisChild,
              validity=validity,
              children=children,
              attributes=attributes,
              warn=warn,
              section=thisChild,
              caseSens=caseSens
            )
            return(all(thisChildValidity, grandChildValidity))
          }
        ))
      } else {
        childValidity <- NULL
      }
    } else {
      childValidity <- valid.child(
        parent=parentName,
        children=obj,
        validity=validity,
        warn=warn,
        section=section,
        caseSens=caseSens
      )
    }
  } else {}
  if(isTRUE(attributes)){
    # we only check attributes of "obj"
    attributeValidityObj <- valid.attribute(
      node=XMLName(obj),
      attrs=XMLAttrs(obj),
      validity=validity,
      warn=warn,
      caseSens=caseSens
    )
    if(isTRUE(recursion) & !isTRUE(children)){
      # we can skip this if children was TRUE, because attributes were
      # already checked recursively, then. but if not:
      # are there any children to check in the first place?
      if(length(nodeChildren) > 0){
        attributeValidityRecursive <- all(sapply(
          nodeChildren,
          function(thisChild){
            # because of the recursion this checks the attributes of "thisChild"
            thisChildValidity <- validXML(
              thisChild,
              validity=validity,
              children=FALSE,
              attributes=TRUE,
              warn=warn,
              section=thisChild,
              caseSens=caseSens
            )
            return(thisChildValidity)
          }
        ))
      } else {
        attributeValidityRecursive <- NULL
      }
    } else {
      attributeValidityRecursive <- NULL
    }
    attributeValidity <- all(attributeValidityObj, attributeValidityRecursive)
  } else {}

  return(all(childValidity, attributeValidity, emptyValidity))
})
