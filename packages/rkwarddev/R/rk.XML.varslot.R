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


#' Create a XML node "varslot" for RKWard plugins
#'
#' @param label Character string, a text label for the varslot.
#' @param source Either a character string (the \code{id} name of the \code{varselector} to select variables
#'    from), or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used, must be
#'    a \code{<varselector>} node).
#' @param required Logical, whether the selection of variables is mandatory or not.
#' @param multi Logical, whether the varslot holds only one or several objects.
#' @param min If \code{multi=TRUE} defines how many objects must be selected. Sets \code{multi=TRUE}.
#' @param any If \code{multi=TRUE} defines how many objects must be selected at least if any
#'    are selected at all. Sets \code{multi=TRUE}.
#' @param max If \code{multi=TRUE} defines how many objects can be selected in total
#'    (0 means any number). Sets \code{multi=TRUE}.
#' @param dim The number of dimensions, an object needs to have. If \code{dim=0} any number
#'    of dimensions is acceptable.
#' @param min.len The minimum length, an object needs to have.
#' @param max.len The maximum length, an object needs to have. If \code{NULL}, defaults to the largest
#'    integer number representable on the system.
#' @param classes An optional character vector, defining class names to which the selection must be limited.
#' @param types If you specify one or more variables types here, the varslot will only accept objects of those
#'    types. Valid types are "unknown", "number", "string", "factor", "invalid". Optional, use with great care,
#'    the user should not be prevented from making valid choices, and rkward does not always know the type
#'    of a variable!
#' @param id.name Character vector, unique ID for the varslot.
#'    If \code{"auto"}, the ID will be generated automatically from \code{label}.
#' @param help Character string, will be used as the \code{text} value for a setting node in the .rkh file.
#'    If set to \code{FALSE}, \code{\link[rkwarddev:rk.rkh.scan]{rk.rkh.scan}} will ignore this node.
#'    Also needs \code{component} to be set accordingly!
#' @param component Character string, name of the component this node belongs to. Only needed if you
#'    want to use the scan features for automatic help file generation; needs \code{help} to be set
#'    accordingly, too!
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.vars]{rk.XML.vars}},
#'    \code{\link[rkwarddev:rk.XML.varselector]{rk.XML.varselector}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' \dontrun{
#' test.varselector <- rk.XML.varselector("Select some vars")
#' test.varslot <- rk.XML.varslot("Vars go here", source=test.varselector)
#' cat(pasteXML(test.varslot))
#' }

rk.XML.varslot <- function(label, source, required=FALSE, multi=FALSE, min=1, any=1, max=0,
  dim=0, min.len=0, max.len=NULL, classes=NULL, types=NULL, id.name="auto", help=NULL, component=rk.get.comp()){
  if(is.XiMpLe.node(source)){
    source.name <- slot(source, "name")
    if(!identical(source.name, "varselector")){
      stop(simpleError(paste0("'source' must be a <varselector> node! You provided: <", source.name, ">")))
    } else {}
  } else {}

  if(identical(id.name, "auto")){
    var.slot.attr <- list(id=auto.ids(label, prefix=ID.prefix("varslot", length=4)))
  } else if(!is.null(id.name)){
    var.slot.attr <- list(id=id.name)
  } else {}
  
  var.slot.attr[["label"]] <- label

  var.slot.attr[["source"]] <- check.ID(source)

  if(!is.null(classes)){
    var.slot.attr[["classes"]] <- paste(classes, collapse=" ")
  } else {}
  if(!is.null(types)){
    valid.types <- c("unknown", "number", "string", "factor", "invalid")
    invalid.type <- !types %in% valid.types
    if(invalid.type){
      warning(paste0("You provided invalid types for varslot, they were ignored: ", paste(types, collapse=", ")))
      types <- ""
    } else {}
    var.slot.attr[["types"]] <- paste(types, collapse=" ")
  } else {}
  if(isTRUE(required)){
    var.slot.attr[["required"]] <- "true"
  } else {}

  # "multi" is mandatory if min, max or any are set
  if(isTRUE(multi) | min > 1 | any > 1 | max > 0){
    var.slot.attr[["multi"]] <- "true"
    if(min > 1){
      var.slot.attr[["min_vars"]] <- min
    } else {}
    if(any > 1){
      var.slot.attr[["min_vars_if_any"]] <- any
    } else {}
    if(max > 0){
      var.slot.attr[["max_vars"]] <- max
    } else {}
  } else {}

  if(dim > 0){
    var.slot.attr[["num_dimensions"]] <- dim
  } else {}
  if(min.len > 0){
    var.slot.attr[["min_length"]] <- min.len
  } else {}
  if(!is.null(max.len)){
    var.slot.attr[["max_length"]] <- max.len
  } else {}

  v.slot <- XMLNode("varslot", attrs=var.slot.attr)

  # check for .rkh content
  rk.set.rkh.prompter(component=component, id=var.slot.attr[["id"]], help=help)

  return(v.slot)
}
