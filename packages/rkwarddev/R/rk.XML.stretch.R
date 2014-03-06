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


#' Create XML empty node "stretch" for RKWard plugins
#'
#' The simplest way to use \code{rk.XML.stretch} is to call it without arguments.
#' If you provide \code{before} and/or \code{after}, a "<stretch />" will be put between
#' the XML elements defined there.
#'
#' @param before A list of objects of class \code{XiMpLe.node}.
#' @param after A list of objects of class \code{XiMpLe.node}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' cat(pasteXML(rk.XML.stretch()))

#<stretch />
rk.XML.stretch <- function(before=NULL, after=NULL){
  strch <- XMLNode("stretch")

  # if called without furter objects, just return the node
  if(is.null(c(before, after))){
    return(strch)
  } else {}

  if(!is.null(before)){
    strch.lst <- child.list(before)
    strch.lst[[length(strch.lst)+1]] <- strch
  } else {
    strch.lst <- list(strch)
  }

  if(!is.null(after)){
    for(this.element in child.list(after)){
        strch.lst[[length(strch.lst)+1]] <- this.element
      }
  } else {}

  return(strch.lst)
}
