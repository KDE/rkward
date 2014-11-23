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


#' Create XML "menu" node for RKWard plugins
#'
#' This function will create a menu node for hierarchy sections.
#' Use same \code{id} values to place entries in the same menu.
#' 
#' @param label Character string, a label for the menu.
#' @param ... Eithet objects of class \code{XiMpLe.node}, must be either
#'    "menu" or "entry", or a list of character strings representing the menu path,
#'    with the last element being the \code{component} value for \code{\link[rkwarddev:rk.XML.entry]{rk.XML.entry}}.
#' @param index Integer number to influence the level of menu placement. If \code{...} is a list,
#'    \code{index} can also be a vector of the same length + 1, so indices will be set in the same order to the
#'    menu levels, the last value is for the entry.
#' @param id.name Character, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the label. Otherwise, if \code{...} is a list,
#'    \code{id.name} must have the same length and will be set in the same order to the menu levels.
#'    Used to place the menu in the global menu hierarchy.
#' @param i18n Either a character string or a named list with the optional element \code{context},
#'    to give some \code{i18n_context}
#'    information for this node. If set to \code{FALSE}, the attribute \code{label} will be renamed into 
#'    \code{noi18n_label}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.hierarchy]{rk.XML.hierarchy}},
#'    \code{\link[rkwarddev:rk.XML.entry]{rk.XML.entry}},
#'    \code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
#'    \code{\link[rkwarddev:rk.XML.components]{rk.XML.components}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
#' test.entry <- rk.XML.entry(test.component)
#' test.menu <- rk.XML.menu("Analysis", test.entry, id.name="analysis")
#' cat(pasteXML(test.menu))
#' # manual definition of a menu path by a list:
#' test.menu <- rk.XML.menu("Analysis", list("Level 1", "Level 2", test.component))

rk.XML.menu <- function(label, ..., index=-1, id.name="auto", i18n=NULL){
  nodes <- list(...)
  num.nodes <- length(nodes)

  # if nodes is a list, create menu structure of it
  if(is.list(nodes[[1]])){
    menu.levels <- nodes[[1]]
    num.levels <- length(menu.levels)

    # work your way down, but for safety resons (and sanity...), don't go deeper than 10 levels
    if(num.levels > 10){
      stop(simpleError("Automatic menu recursion won't go deeper than 10 levels, sorry!"))
    } else if(num.levels > 1){
      # check ID names
      if(!identical(id.name, "auto")){
        if(length(id.name) == num.levels){
          new.id.name <- id.name[2:num.levels]
          id.name <- id.name[1]
        } else {
          stop(simpleError("If 'id.name' is not \"auto\" it must have the same length as the menu levels!"))
        }
      } else {
        new.id.name <- id.name
      }
      # check indexing
      num.index <- length(index)
      if(num.index > 1){
      cat(num.index)
        if(num.index == num.levels + 1){
          new.index <- index[2:(num.levels + 1)]
          index <- index[1]
        } else {
          stop(simpleError("If 'index' is not a single value it must have the length of the menu levels + 1 for the entry!"))
        }
      } else {
        new.index <- -1
      }
      new.label <- menu.levels[[1]]
      new.list <- menu.levels[2:num.levels]
      # we still go deeper, so call ourselves recursively
      nodes <- rk.XML.menu(label=new.label, new.list, index=new.index, id.name=new.id.name)
    } else {
      if(length(index) == 2){
        entry.index <- index[2]
        index <- index[1]
      } else {
        entry.index <- -1
      }
      nodes <- rk.XML.entry(component=menu.levels[[1]], index=entry.index)
      # this is the last entry, i.e. *the* entry so we can stop here
    }
  } else {}

  # check the node names and allow only valid ones
  valid.child("menu", children=nodes)

  if(identical(id.name, "auto")){
    # try autogenerating some id
    id.name <- auto.ids(label, prefix=ID.prefix("menu"), chars=10)
  } else if(is.null(id.name)){
    stop(simpleError("Menu needs an ID!"))
  } else {}

  attr.list <- list(id=id.name, label=label)

  if(!identical(index, -1)){
    attr.list[["index"]] <- index
  } else {}

  # check for additional i18n info; if FALSE, "label" will be renamed to "noi18n_label"
  attr.list <- check.i18n(i18n=i18n, attrs=attr.list)

  node <- XMLNode("menu",
      attrs=attr.list,
      .children=child.list(nodes)
    )

  return(node)
}
