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


#' Create XML "component" node for RKWard plugins
#'
#' This function will create a component node for components sections of .pluginmap files.
#' 
#' @param label Character string, a label for the component.
#' @param file Character string, file name of a plugin XML file defining the GUI.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the label.
#' @param type Character string, type of component. As of now, only "standard" is supported. The option is
#'    just implemented for completeness.
#' @param dependencies An object of class \code{XiMpLe.node} to define \code{<dependencies>} for this component.
#'    See \code{\link[XiMpLe:rk.XML.dependencies]{rk.XML.dependencies}} for details. Skipped if \code{NULL}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.components]{rk.XML.components}},
#'    \code{\link[rkwarddev:rk.XML.dependencies]{rk.XML.dependencies}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")

rk.XML.component <- function(label, file, id.name="auto", type="standard", dependencies=NULL){
  if(identical(id.name, "auto")){
    # try autogenerating some id
    id.name <- auto.ids(label, prefix=ID.prefix("component"), chars=10)
  } else if(is.null(id.name)){
    stop(simpleError("Components need an ID!"))
  } else {}
  attr.list <- list(id=check.ID(id.name), label=label)

  # once there are more types supported, this will make much more sense...
  if(!type %in% c("standard")){
    stop(simpleError(paste0("Invalid type: ", type)))
  } else {
    attr.list[["type"]] <- type
  }
  if(!is.null(file)){
    attr.list[["file"]] <- file
  } else {}

  # does this component hava additional dependencies?
  if(!is.null(dependencies)){
    # check if this is *really* a dependencies section
    valid.parent("dependencies", node=dependencies, see="rk.XML.dependencies", comment.ok=TRUE)
    dependencies <- child.list(dependencies)
  } else {
    dependencies <- list("")
  }

  node <- XMLNode("component", attrs=attr.list, .children=dependencies)

  return(node)
}
