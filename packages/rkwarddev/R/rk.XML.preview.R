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


#' Create XML node "preview" for RKWard plugins
#'
#' @param label A character string, text label for the preview checkbox.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.preview <- rk.XML.preview("See a preview?")
#' cat(pasteXML(test.preview))

rk.XML.preview <- function(label="Preview"){
  if(!identical(label, "Preview")){
    attr.list <- list(label=label)
  } else {
    attr.list <- list()
  }

  node <- XMLNode("preview", attrs=attr.list)

  return(node)
}
