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


#' Create XML "require" node for RKWard plugins
#'
#' This function will create a require node for .pluginmap files.
#' 
#' Note that only one of the values can be set at a time. \code{file} should be preferred whenever
#' possible.
#' 
#' @param file Character string, file name of another .pluginmap file to be included. Should be
#'    preferred over \code{map} if that file is in the same package.
#' @param map Character string, should be \code{"namespace::id"} of another .pluginmap to be included.
#'    Can be used to address plugin maps which are not part of the same plugin package.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.require <- rk.XML.require("another.pluginmap")
#' cat(pasteXML(test.require))

rk.XML.require <- function(file=NULL, map=NULL){
  # one of file or map *must* be used
  if(is.null(file) && is.null(map)){
    stop(simpleError("'file' or 'map' must be specified!"))
  } else {}
  # but only *one* of file or map can be used
  if(!is.null(file) && !is.null(map)){
    stop(simpleError("'file' and 'map' cannot be used both at the same time!"))
  } else {}

  # now that we know one of both is set
  if(!is.null(file)){
    if(length(file) > 1 || !is.character(file)){
      stop(simpleError("'file' must be a character string!"))
    } else {}
  } else {
    if(length(map) > 1 || !is.character(map)){
      stop(simpleError("'map' must be a character string!"))
    } else {}
  }

  node <- XMLNode("require", attrs=list(file=file, map=map))

  return(node)
}
