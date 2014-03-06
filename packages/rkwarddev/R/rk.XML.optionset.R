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


#' Create XML node "optionset" for RKWard plugins
#'
#' Note that if you want to refer to the optioncolumns in your JavaScript code, the \code{id}
#' you need is a combination of \code{<optionset id>.<optioncolumn id>.<modifier>}. that is,
#' you must always prefix it with the sets' \code{id}. For JavaScript code generating with
#' \code{rkwarddev}, you can use the ID that functions like \code{\link[rkwarddev:id]{id}} return,
#' because the JavaScript variable name will only contain a constant prefix ("ocol") an the column ID.
#'
#' @note The \code{<optionset>} node was introduced with RKWard 0.6.1, please set the dependencies
#'    of your component/plugin accordingly.
#'
#' @param content A list of XiMpLe.nodes to be placed inside the \code{<content>} node of this \code{<optionset>}.
#' @param optioncolumn A list of \code{<optioncolumn>} XiMpLe.nodes.
#' @param min_rows Numeric (integer), if specified, the set will be marked invalid, unless it has
#'    at least this number of rows. Ignored if set to 0.
#' @param min_rows_if_any Numeric (integer), like min_rows, but will only be tested, if there is at
#'    least one row. Ignored if set to 0.
#' @param max_rows Numeric (integer), if specified, the set will be marked invalid, unless it has
#'    at most this number of rows. Ignored if set to 0.
#' @param keycolumn Character
#' @param logic A valid \code{<logic>} node.
#' @param optiondisplay Logical value, can be used to automatically add an \code{<optiondisplay>} node on top
#'    of the \code{<content>} section. Depending on whether it's \code{TRUE} or \code{FALSE}, its \code{index}
#'    argument will be set to \code{"true"} or \code{"false"}, respectively. Set to \code{NULL} to deactivate.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the <content> nodes.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.optioncolumn]{rk.XML.optioncolumn}},
#'    \code{\link[rkwarddev:rk.XML.optiondisplay]{rk.XML.optiondisplay}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' firstname <- rk.XML.input("Given name(s)")
#' lastname <- rk.XML.input("Family name")
#' genderselect <- rk.XML.radio("Gender", options=list(
#'   Male = c(val="m"),
#'   Female = c(val="f")))
#' (myOptionset <- rk.XML.optionset(
#'   content = list(
#'     rk.XML.row(
#'       firstname,
#'       lastname,
#'       genderselect)),
#'   optioncolumn = list(
#'     rk.XML.optioncolumn(firstname, modifier="text"),
#'     rk.XML.optioncolumn(lastname, modifier="text"),
#'     rk.XML.optioncolumn(genderselect)
#'   )
#' ))
rk.XML.optionset <- function(content, optioncolumn, min_rows=0, min_rows_if_any=0, max_rows=0,
  keycolumn=NULL, logic=NULL, optiondisplay=TRUE, id.name="auto"){

  if(identical(id.name, "auto")){
    # try autogenerating some id
    attr.list <- list(id=auto.ids(node.soup(content), prefix=ID.prefix("oset"), chars=10))
  } else if(is.null(id.name)){
    attr.list <- list()
  } else {
    attr.list <- list(id=id.name)
  }

  if(min_rows > 0){
    attr.list[["min_rows"]] <- min_rows
  } else {}
  if(min_rows_if_any > 0){
    attr.list[["min_rows_if_any"]] <- min_rows_if_any
  } else {}
  if(max_rows > 0){
    attr.list[["max_rows"]] <- max_rows
  } else {}

  ## TODO: do some checking -- and add support to supply XiMpLe nodes
  if(!is.null(keycolumn)){
    attr.list[["keycolumn"]] <- keycolumn
  } else {}

  if(!is.null(logic)){
    if(is.XiMpLe.node(logic) && identical(XMLName(logic), "logic")){
      valid.child("logic", children=XMLChildren(logic))
    } else {
      stop(simpleError("'logic' must be a <logic> node!"))
    }
    # checks go here
  } else {}

  # this list will carry all child nodes of the full set
  all.children <- list(logic)

  content <- child.list(content)
  optioncolumn <- child.list(optioncolumn)

  # auto-add optiondisplay
  if(!is.null(optiondisplay)){
    content <- append(content, rk.XML.optiondisplay(index=optiondisplay), after=0)
  } else {}

  content.node <- XMLNode("content",
    .children=content)

  # append content node
  all.children <- append(all.children, content.node)
  # append optioncolumns
  all.children <- append(all.children, optioncolumn)

  node <- XMLNode("optionset",
    attrs=attr.list,
    .children=all.children)

  return(node)
}
