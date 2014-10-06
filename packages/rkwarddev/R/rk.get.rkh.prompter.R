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

#' Get .rkh related information stored internally
#' 
#' @param plugin Character string, the name under which you stored information. If \code{NULL},
#'    returns all information stored in the internal \code{rkh.prompter} list.
#' @param id Character string, the node ID if a given plugin to search for.  If \code{NULL}, returns
#'    the full list of the given plugin, otherwise only the help information for the requested node.
#' @export
#' @examples
#' rk.get.rkh.prompter("rk.myPlugin", "someID")

rk.get.rkh.prompter <- function(plugin=NULL, id=NULL){
  rkh.prompter <- get.rkh.prompter()

  if(is.null(plugin)){
    return(rkh.prompter)
  } else {
    rkh.prompter <- rkh.prompter[[plugin]]
    if(is.null(id)){
      return(rkh.prompter)
    } else {
      # let's see if we need to extract IDs first
      id <- check.ID(id)
      return(rkh.prompter[[id]])
    }
  }
}
