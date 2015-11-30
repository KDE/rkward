# Copyright 2014 Meik Michalke <meik.michalke@hhu.de>
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


#' Create a XML node "valueslot" for RKWard plugins
#'
#' @param label Character string, a text label for the valueslot.
#' @param source Either a character string (the \code{id} name of the \code{valueselector} to select values
#'    from), or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used). If it is not
#'    a \code{<valueselector>} node, you must also specify a valid property for the node.
#' @param property Character string, valid property for a XiMpLe node defined by \code{source}. In the XML code, it
#'    will cause the use of \code{source_property} instead of \code{source}. Only used if \code{source} is not a
#'    \code{<valueselector>} node. 
#' @param required Logical, whether the selection of values is mandatory or not.
#' @param multi Logical, whether the valueslot holds only one or several objects.
#' @param duplicates Logical, if \code{multi=TRUE} defines whether the same entry may be added multiple times. Sets \code{multi=TRUE}.
#' @param min If \code{multi=TRUE} defines how many objects must be selected. Sets \code{multi=TRUE}.
#' @param any If \code{multi=TRUE} defines how many objects must be selected at least if any
#'    are selected at all. Sets \code{multi=TRUE}.
#' @param max If \code{multi=TRUE} defines how many objects can be selected in total
#'    (0 means any number). Sets \code{multi=TRUE}.
#' @param id.name Character vector, unique ID for the valueslot.
#'    If \code{"auto"}, the ID will be generated automatically from \code{label}.
#' @param help Character string or list of character values and XiMpLe nodes, will be used as the \code{text} value for a setting node in the .rkh file.
#'    If set to \code{FALSE}, \code{\link[rkwarddev:rk.rkh.scan]{rk.rkh.scan}} will ignore this node.
#'    Also needs \code{component} to be set accordingly!
#' @param component Character string, name of the component this node belongs to. Only needed if you
#'    want to use the scan features for automatic help file generation; needs \code{help} to be set
#'    accordingly, too!
#' @param i18n Either a character string or a named list with the optional elements \code{context}
#'    or \code{comment}, to give some \code{i18n_context} information for this node. If set to \code{FALSE},
#'    the attribute \code{label} will be renamed into \code{noi18n_label}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.values]{rk.XML.values}},
#'    \code{\link[rkwarddev:rk.XML.valueselector]{rk.XML.valueselector}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' \dontrun{
#' test.valueselector <- rk.XML.valueselector("Select some values")
#' test.valueslot <- rk.XML.valueslot("Vars go here", source=test.valueselector)
#' cat(pasteXML(test.valueslot))
#' }

rk.XML.valueslot <- function(label, source, property=NULL, required=FALSE, multi=FALSE, duplicates=FALSE, min=1, any=1, max=0,
  id.name="auto", help=NULL, component=rk.get.comp(), i18n=NULL){
  if(identical(id.name, "auto")){
    value.slot.attr <- list(id=auto.ids(label, prefix=ID.prefix("valueslot", length=4)))
  } else if(!is.null(id.name)){
    value.slot.attr <- list(id=id.name)
  } else {}
  
  value.slot.attr[["label"]] <- label

  if(is.XiMpLe.node(source)){
    source.name <- slot(source, "name")
    if(identical(source.name, "valueselector")){
      value.slot.attr[["source"]] <- check.ID(source)
    } else {
      if(is.null(property)){
        stop(simpleError(paste0("'source' must either be a <valueselector> node or come with an appropripate 'property' value!")))
      } else if(modif.validity(source, modifier=property)){
        value.slot.attr[["source_property"]] <- paste(check.ID(source), property, sep=".")
      } else {}
    }
  } else if(is.character(source)){
    var.slot.attr[["source"]] <- check.ID(source)
  } else {
    stop(simpleError("No valid 'source' value given!"))
  }

  if(isTRUE(required)){
    value.slot.attr[["required"]] <- "true"
  } else {}

  # "multi" is mandatory if min, max or any are set
  if(isTRUE(multi) | isTRUE(duplicates) | min > 1 | any > 1 | max > 0){
    value.slot.attr[["multi"]] <- "true"
    if(isTRUE(duplicates)){
      value.slot.attr[["allow_duplicates"]] <- "true"
    } else {}
    if(min > 1){
      value.slot.attr[["min_vars"]] <- min
    } else {}
    if(any > 1){
      value.slot.attr[["min_vars_if_any"]] <- any
    } else {}
    if(max > 0){
      value.slot.attr[["max_vars"]] <- max
    } else {}
  } else {}

  # check for additional i18n info; if FALSE, "label" will be renamed to "noi18n_label"
  value.slot.attr <- check.i18n(i18n=i18n, attrs=value.slot.attr)

  node <- check.i18n(
    i18n=i18n,
    node=XMLNode("valueslot", attrs=value.slot.attr),
    comment=TRUE
  )

  # check for .rkh content
  rk.set.rkh.prompter(component=component, id=value.slot.attr[["id"]], help=help)

  return(node)
}
