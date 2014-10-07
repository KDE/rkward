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


#' Create RKWard help nodes from plugin XML
#'
#' @param pXML Either an object of class \code{XiMpLe.doc} or \code{XiMpLe.node}, or path to a plugin XML file.
#' @param help Logical, if \code{TRUE} a list of XiMpLe.node objects will be returned, otherwise a character
#'    vector with only the relevant ID names.
#' @param captions Logical, if \code{TRUE} captions will be generated for all "page", "tab" and "frame" nodes.
#' @param component Character string, name of the scanned component. Only needed if you want to search for
#'    help text provided by \code{\link[rkwarddev:rk.set.rkh.prompter]{rk.set.rkh.prompter}}.
#' @return A character vector or a list of XiMpLe.node objects.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.rkh.scan <- function(pXML, help=TRUE, captions=TRUE, component=NULL){

  settings.tags <- c("radio", "varslot", "browser", "dropdown",
    "checkbox", "saveobject", "input", "spinbox", "optioncolumn", "matrix")
  if(isTRUE(captions)){
    caption.tags <- c("page", "tab", "frame")
  } else {
    caption.tags <- NULL
  }
  help.relevant.tags <- c(settings.tags, caption.tags)

  single.tags <- get.single.tags(XML.obj=pXML, drop=c("comments","cdata", "declarations", "doctype"))

  help.id <- get.IDs(single.tags=single.tags, relevant.tags=help.relevant.tags, add.abbrev=FALSE, tag.names=TRUE)

  if("id" %in% colnames(help.id)){
    if(isTRUE(help)){
      help.nodes <- unlist(sapply(1:nrow(help.id), function(this.id){
          if(help.id[this.id,"tag"] %in% caption.tags){
            return(rk.rkh.caption(id=help.id[this.id,"id"]))
          } else {
            if(!is.null(component)){
              rkh.text <- rk.get.rkh.prompter(component=component, id=help.id[this.id,"id"])
              # check if the component is to be omitted
              if(is.logical(rkh.text[["help"]]) & !isTRUE(rkh.text[["help"]])){
                return(NULL)
              } else {}
            } else {
              rkh.text <- NULL
            }
            return(rk.rkh.setting(id=help.id[this.id,"id"], text=rkh.text))
          }
        }))
    } else {
      help.nodes <- help.id[,"id"]
    }
  } else {
    help.nodes <- NULL
  }

  return(help.nodes)
}
