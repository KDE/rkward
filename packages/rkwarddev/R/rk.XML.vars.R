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


#' Create a variable selector for RKWard plugins
#'
#' This function will create a <frame> node including a <varselector> and a <varslot> node. It is
#' actually a wrapper for \code{\link[rkwarddev:rk.XML.varslot]{rk.XML.varslot}} and
#' \code{\link[rkwarddev:rk.XML.varselector]{rk.XML.varselector}}, since you usually won't define one
#' without the other.
#'
#' @param label Character string, a text label for the variable browser.
#' @param slot.text Character string, a text label for the variable selection slot.
#' @param required Logical, whether the selection of variables is mandatory or not.
#' @param multi Logical, whether the varslot holds only one or several objects.
#' @param min If \code{multi=TRUE} defines how many objects must be selected.
#' @param any If \code{multi=TRUE} defines how many objects must be selected at least if any
#'    are selected at all.
#' @param max If \code{multi=TRUE} defines how many objects can be selected in total
#'    (0 means any number).
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
#' @param horiz Logical. If \code{TRUE}, the varslot will be placed next to the selector,
#'    if \code{FALSE} below it.
#' @param add.nodes A list of objects of class \code{XiMpLe.node} to be placed after the varslot.
#' @param frame.label Character string, a text label for the whole frame.
#' @param formula.dependent Character string, if not \code{NULL} will cause the addition of a second
#'    varslot for the dependent variable(s), using the text of \code{formula.dependent} as its label. Also
#'    a \code{<formula>} node will be added, using both varslots for \code{fixed_factors} and \code{dependent}
#'    respectively.
#' @param dep.options A named list with optional attributes for the \code{dependent} varslot, if \code{formula.dependent}
#'    is not \code{NULL}. Valid options are \code{required}, \code{multi}, \code{min}, \code{any}, \code{max},
#'    \code{dim}, \code{min.len}, \code{max.len}, \code{classes} and \code{types}. If an options is undefined, it defaults
#'    to the same values like the main options of this function.
#' @param id.name Character vector, unique IDs for the frame (first entry), the varselector (second entry)
#'    and varslot (third entry). If \code{formula.dependent} is not \code{NULL}, a fourth and fifth entry is needed as well,
#'    for the dependent varslot and the formula node, respectively.
#'    If \code{"auto"}, IDs will be generated automatically from \code{label} and \code{slot.text}.
#' @param help Character string, will be used as the \code{text} value for a setting node in the .rkh file.
#'    If set to \code{FALSE}, \code{\link[rkwarddev:rk.rkh.scan]{rk.rkh.scan}} will ignore this node.
#'    Also needs \code{component} to be set accordingly!
#' @param component Character string, name of the component this node belongs to. Only needed if you
#'    want to use the scan features for automatic help file generation; needs \code{help} to be set
#'    accordingly, too!
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.varslot]{rk.XML.varslot}},
#'    \code{\link[rkwarddev:rk.XML.varselector]{rk.XML.varselector}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.vars <- rk.XML.vars("Select some vars", "Vars go here")
#' cat(pasteXML(test.vars))

rk.XML.vars <- function(label, slot.text, required=FALSE, multi=FALSE, min=1, any=1, max=0,
  dim=0, min.len=0, max.len=NULL, classes=NULL, types=NULL, horiz=TRUE, add.nodes=NULL,
  frame.label=NULL, formula.dependent=NULL, dep.options=list(), id.name="auto",
  help=NULL, component=rk.get.comp()){

  if(identical(id.name, "auto")){
    ## if this ID generation get's changed, change it in rk.XML.varslot(), too!
    var.sel.attr <- list(id=auto.ids(label, prefix=ID.prefix("varselector", length=3)))
    var.slot.id <- auto.ids(slot.text, prefix=ID.prefix("varslot", length=4))
    if(!is.null(formula.dependent)){
      var.dep.id <- auto.ids(formula.dependent, prefix=ID.prefix("varslot", length=4))
      frml.id <- auto.ids(formula.dependent, prefix=ID.prefix("formula", length=3))
    } else {}
  } else if(!is.null(id.name)){
    var.sel.attr <- list(id=id.name[[2]])
    var.slot.id <- id.name[[3]]
    if(!is.null(formula.dependent)){
      var.dep.id <- id.name[[4]]
      frml.id <- id.name[[5]]
    } else {}
  } else {}

  var.sel.attr[["label"]] <- label

  v.selector <- rk.XML.varselector(
    label=label,
    id.name=var.sel.attr[["id"]])

  v.slot <- rk.XML.varslot(
    label=slot.text,
    source=v.selector,
    required=required,
    multi=multi,
    min=min,
    any=any,
    max=max,
    dim=dim,
    min.len=min.len,
    max.len=max.len,
    classes=classes,
    types=types,
    id.name=var.slot.id,
    help=help,
    component=component)

  slot.content <- list(v.slot)

  if(!is.null(formula.dependent)){
    dep.opt.names <- names(dep.options)
    dep.slot <- rk.XML.varslot(
      label=formula.dependent,
      source=v.selector,
      required=if ("required" %in% dep.opt.names) {dep.options[["required"]]} else {FALSE},
      multi=if ("multi" %in% dep.opt.names) {dep.options[["multi"]]} else {FALSE},
      min=if ("min" %in% dep.opt.names) {dep.options[["min"]]} else {1},
      any=if ("any" %in% dep.opt.names) {dep.options[["any"]]} else {1},
      max=if ("max" %in% dep.opt.names) {dep.options[["max"]]} else {0},
      dim=if ("dim" %in% dep.opt.names) {dep.options[["dim"]]} else {0},
      min.len=if ("min.len" %in% dep.opt.names) {dep.options[["min.len"]]} else {0},
      max.len=if ("max.len" %in% dep.opt.names) {dep.options[["max.len"]]} else {NULL},
      classes=if ("classes" %in% dep.opt.names) {dep.options[["classes"]]} else {NULL},
      types=if ("types" %in% dep.opt.names) {dep.options[["types"]]} else {NULL},
      id.name=var.dep.id,
      help=help,
      component=component)
    slot.content[[length(slot.content) + 1]] <- dep.slot
    formula.node <- rk.XML.formula(fixed=v.slot, dependent=dep.slot, id.name=frml.id)
    slot.content[[length(slot.content) + 1]] <- formula.node
  } else {}

  # do we need to add extra nodes to the varslot?
  if(!is.null(add.nodes)){
    for (this.node in add.nodes) {
      slot.content[[length(slot.content)+1]] <- this.node
    }
  } else {}

  if(isTRUE(horiz)){
    aligned.chld <- rk.XML.row(list(rk.XML.col(v.selector), rk.XML.col(slot.content)))
  } else {
    aligned.chld <- list(v.selector, unlist(slot.content))
  }

  vars.frame <- rk.XML.frame(
    children=child.list(aligned.chld),
    label=frame.label,
    id.name=id.name[[1]])

  return(vars.frame)
}
