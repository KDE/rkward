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


#' Create XML "matrix" node for RKWard plugins
#' 
#' @note The \code{<matrix>} node was introduced with RKWard 0.6.1, please set the dependencies
#'    of your component/plugin accordingly.
#'
#' @param label Character string, a label for the matrix.
#' @param mode Character string, one of "integer", "real" or "string". The type of data that will
#'    be accepted in the table (required)
#' @param rows Number of rows in the matrix. Has no effect if \code{allow_user_resize_rows=TRUE}.
#' @param columns Number of columns in the matrix. Has no effect if \code{allow_user_resize_columns=TRUE}.
#' @param min Minimum acceptable value (if \code{type} is "integer" or "real"). Defaults to the
#'    smallest representable value.
#' @param max Maximum acceptable value (if \code{type} is "integer" or "real"). Defaults to the
#'    largest representable value.
#' @param min_rows Minimum number of rows, matrix will refuse shrink below this size.
#' @param min_columns Minimum number of columns, matrix will refuse shrink below this size.
#' @param allow_missings Logical, whether missing (empty) values are allowed in the matrix
#'    (if \code{type} is "string").
#' @param allow_user_resize_columns Logical, if \code{TRUE}, the user can add columns by typing
#'    on the rightmost (inactive) cells.
#' @param allow_user_resize_rows Logical, if \code{TRUE}, the user can add rows by typing on the
#'    bottommost (inactive) cells.
#' @param fixed_width Logical, assume the column count will not change. The last (or typically only)
#'    column will be stretched to take up the available width. Do not use in combination with matrices,
#'    where the number of columns may change in any way. Useful, esp. when creating a vector input element (rows="1").
#' @param fixed_height Logical, force the GUI element to stay at its initial height. Do not use in
#'    combindation with matrices, where the number of rows may change in any way.
#'    Useful, esp. when creating a vector input element (columns="1").
#' @param horiz_headers Character vector to use for the horiztonal header. Defaults to column number.
#' @param vert_headers Character vector to use for the vertical header. Defaults to row number.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the label.
#' @param help Character string or list of character values and XiMpLe nodes, will be used as the \code{text} value for a setting node in the .rkh file.
#'    If set to \code{FALSE}, \code{\link[rkwarddev:rk.rkh.scan]{rk.rkh.scan}} will ignore this node.
#'    Also needs \code{component} to be set accordingly!
#' @param component Character string, name of the component this node belongs to. Only needed if you
#'    want to use the scan features for automatic help file generation; needs \code{help} to be set
#'    accordingly, too!
#' @param i18n Either a character string or a named list with the optional element \code{context},
#'    to give some \code{i18n_context}
#'    information for this node. If set to \code{FALSE}, the attribute \code{label} will be renamed into 
#'    \code{noi18n_label}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.matrix <- rk.XML.matrix("A matrix")

rk.XML.matrix <- function(label, mode="real", rows=2, columns=2, min=NULL, max=NULL, min_rows=0, min_columns=0,
  allow_missings=FALSE, allow_user_resize_columns=TRUE,
  allow_user_resize_rows=TRUE, fixed_width=FALSE, fixed_height=FALSE,
  horiz_headers=NULL, vert_headers=NULL, id.name="auto", help=NULL, component=rk.get.comp(), i18n=NULL){
  if(identical(id.name, "auto")){
    # try autogenerating some id
    id.name <- auto.ids(label, prefix=ID.prefix("matrix"), chars=10)
  } else if(is.null(id.name)){
    stop(simpleError("Matrices need an ID!"))
  } else {}
  attr.list <- list(id=check.ID(id.name), label=label)

  if(!mode %in% c("integer", "real", "string")){
    stop(simpleError(paste0("Invalid mode: ", mode)))
  } else {
    attr.list[["mode"]] <- mode
  }

  if(mode %in% c("string")){
    if(isTRUE(allow_missings)){
      attr.list[["allow_missings"]] <- "true"
    } else {}
  } else {}

  if(mode %in% c("integer", "real")){
    if(!is.null(min)){
      attr.list[["min"]] <- min
    } else {}
    if(!is.null(max)){
      attr.list[["max"]] <- max
    } else {}
  } else {}

  if(!isTRUE(allow_user_resize_rows)){
    attr.list[["allow_user_resize_rows"]] <- "false"
    if(rows != 2){
      attr.list[["rows"]] <- rows
    } else {}
  } else {}
  if(min_rows != 0){
      attr.list[["min_rows"]] <- min_rows
  } else {}

  if(!isTRUE(allow_user_resize_columns)){
    attr.list[["allow_user_resize_columns"]] <- "false"
    if(columns != 2){
      attr.list[["columns"]] <- columns
    } else {}
  } else {}
  if(min_columns != 0){
      attr.list[["min_columns"]] <- min_columns
  } else {}

  if(isTRUE(fixed_width)){
    attr.list[["fixed_width"]] <- "true"
  } else {}
  if(isTRUE(fixed_height)){
    attr.list[["fixed_height"]] <- "true"
  } else {}
  
  if(!is.null(horiz_headers)){
    if(is.character(horiz_headers)){
      attr.list[["horiz_headers"]] <- paste(horiz_headers, collapse=";")
    } else {
      stop(simpleError("'horiz_headers' must be a character vector!"))
    }
  } else {}

  if(!is.null(vert_headers)){
    if(is.character(vert_headers)){
      attr.list[["vert_headers"]] <- paste(vert_headers, collapse=";")
    } else {
      stop(simpleError("'vert_headers' must be a character vector!"))
    }
  } else {}

  # check for additional i18n info; if FALSE, "label" will be renamed to "noi18n_label"
  attr.list <- check.i18n(i18n=i18n, attrs=attr.list)

  node <- XMLNode("matrix", attrs=attr.list)

  # check for .rkh content
  rk.set.rkh.prompter(component=component, id=attr.list[["id"]], help=help)

  return(node)
}
