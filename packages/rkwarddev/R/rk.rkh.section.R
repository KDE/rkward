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


#' Create XML "section" node for RKWard help pages
#'
#' This function will create a section node for settings sections in RKWard help files.
#'
#' @param title Character string, title to be displayed.
#' @param text Character string, the text to be displayed.
#' @param short Character string, short title for the menu for links to this section.
#' @param id.name Character string, a unique ID for this element.
#'    If \code{"auto"}, an ID will be generated automatically from the \code{title} value.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.section <- rk.rkh.section("General background", text="Some important notes...",
#' short="Background")
#' cat(pasteXML(test.section))

rk.rkh.section <- function(title, text=NULL, short=NULL, id.name="auto"){
  if(identical(id.name, "auto")){
    attr.list <- list(id=auto.ids(title, prefix=ID.prefix("section")),
      title=title)
  } else if(!is.null(id.name)){
    attr.list <- list(id=id.name, title=title)
  } else {
    stop(simpleError("'id.name' must have a value!"))
  }

  if(!is.null(short)){
    attr.list[["short_title"]] <- short
  } else {}

  if(is.null(text)){
    text <- ""
  } else {}

  node <- XMLNode("section", text, attrs=attr.list)

  return(node)
}
