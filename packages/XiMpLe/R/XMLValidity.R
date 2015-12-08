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


#' Constructor function for XiMpLe.validity objects
#'
#' Create validity definitions for XiMpLe nodes, to be used by
#' \code{\link[XiMpLe:validXML]{validXML}}.
#'
#' @param children  Named list of character vectors, where the element name defines the parent node
#'   name and each character string a valid child node name.
#' @param attrs Named list of character vectors, where the element name defines the parent node
#'   name and each character string a valid attribute name.
#' @param allChildren Character vector, names of globally valid child nodes for all nodes, if any.
#' @param allAttrs Character vector, names of globally valid attributes for all nodes, if any.
#' @param empty Character vector, names of nodes that must be empty nodes (i.e., no closing tag), if any.
#' @return An object of class \code{\link[XiMpLe:XiMpLe.validity-class]{XiMpLe.validity}}
#' @seealso
#'    \code{\link[XiMpLe:validXML]{validXML}}
#' @export
#' @rdname XMLValidity
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
XMLValidity <- function(children=NULL, attrs=NULL, allChildren=NULL, allAttrs=NULL, empty=NULL){

  if(is.null(children)){
    children <- list()
  } else {}
  if(is.null(attrs)){
    attrs <- list()
  } else {}
  if(is.null(allChildren)){
    allChildren <- character()
  } else {}
  if(is.null(allAttrs)){
    allAttrs <- character()
  } else {}
  if(is.null(empty)){
    empty <- character()
  } else {}
  
  newValidity <- new("XiMpLe.validity",
    children=children,
    attrs=attrs,
    allChildren=allChildren,
    allAttrs=allAttrs,
    empty=empty
  )

  return(newValidity)
}