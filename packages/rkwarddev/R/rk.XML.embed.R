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


#' Create XML node "embed" for RKWard plugins
#'
#' @param component A character string, registered name (\code{id} in pluginmap file) of the component to be embedded.
#' @param button Logical, whether the plugin should be embedded as a button and appear if it's pressed.
#' @param label A character string, text label for the button (only used if \code{button=TRUE}).
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the label and component strings.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.embed <- rk.XML.embed("someComponent")
#' cat(pasteXML(test.embed))

rk.XML.embed <- function(component, button=FALSE, label="Options", id.name="auto"){
  attr.list <- list(component=component)

  if(isTRUE(button)){
    attr.list[["as_button"]] <- "true"
  } else {}
  
  if(!identical(label, "Options") && isTRUE(button)){
    attr.list[["label"]] <- label
  } else {}

  if(identical(id.name, "auto")){
    attr.list[["id"]] <- auto.ids(paste0(label, component), prefix=ID.prefix("embed"))
  } else {
    attr.list[["id"]] <- id.name
  }

  node <- XMLNode("embed", attrs=attr.list)

  return(node)
}
