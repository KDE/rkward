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

#' Set up an environment to store .rkh related information
#' 
#' By using an environment like this, you are able to write information for RKWard help files
#' directly into your script code of certrain functions, like for radio buttons or checkboxes.
#' 
#' The information is temporarily stored in an internal environment, using the plugin name
#' you specify. Each entry is named after the ID of its respective node. If you later call
#' \code{\link[rkwarddev:rk.plugin.component]{rk.plugin.component}} (or it is called by other
#' functions) and you activate the \code{scan} option for rkh files, the scanning process
#' will try to find a match for each relevant XML node. That is, the info which is stored in the
#' environment will magically be written into the help file.
#' 
#' @param plugin Character string, should be a unique name to identify the current plugin.
#' @param id Either a character string (the \code{id} of the node to store the help information for),
#'    or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used).
#' @param rm Logical, If \code{TRUE} will remove all information stored by the name of \code{plugin} (if
#'    \code{id=NULL}) or of the given \code{id=NULL}, respectively.
#' @export
#' @examples
#' rk.set.rkh.prompter("rk.myPlugin", "someID", "CLick this to feel funny.")

rk.set.rkh.prompter <- function(plugin, id, help=NULL, rm=FALSE){
  rkh.prompter <- get.rkh.prompter()

  if(!is.null(id)){
    # let's see if we need to extract IDs first
    id <- check.ID(id)
  } else {}
  if(isTRUE(rm)){
    if(is.null(id)){
      rkh.prompter[[plugin]] <- NULL
      message(paste0("removed ", plugin, " from rkh.prompter!"))
    } else {
      rkh.prompter[[plugin]][[id]] <- NULL
      message(paste0("removed ", plugin, "[[\"", id, "\"]] from rkh.prompter!"))
    }
  } else {
    if(is.null(rkh.prompter[[plugin]])){
      rkh.prompter[[plugin]] <- list()
    } else {}
    if(is.null(rkh.prompter[[plugin]][[id]])){
      rkh.prompter[[plugin]][[id]] <- list()
    } else {}
   rkh.prompter[[plugin]][[id]][["help"]] <- help
  }

  assign("rkh.prompter", rkh.prompter, envir=.rkdev.env)
  return(invisible(NULL))
}
