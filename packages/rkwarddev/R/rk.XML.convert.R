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


#' Create XML node convert for RKWard plugins
#'
#' If \code{sources} holds \code{XiMpLe.node} objects, the validity of modifiers is automatically checked for that tag.
#'
#' @note To get a list of the implemented modifiers for \code{sources} in this package, call \code{rkwarddev:::all.valid.modifiers}.
#'
#' @param sources A list with at least one value, either resembling the \code{id} of
#'    an existing element to be queried as a character string, or a previously defined object
#'    of class \code{XiMpLe.node} (whose \code{id} will be extracted and used). If you want
#'    to examine e.g. the state or string value specificly, just name the value accoringly, e.g.,
#'    \code{sources=list("vars0", string="input1", state="chkbx2")}.
#' @param mode A named vector with either exactly one of the following elements:
#'    \itemize{
#'      \item{\code{equals}}{True if \code{sources} equals this value.}
#'      \item{\code{notequals}}{True if \code{sources} differs from this value.}
#'      \item{\code{and}}{True if all \code{sources} are true. The sources must be boolean,
#'        and the actual value here is irrelevant, so \code{mode=c(and="")} is valid.}
#'      \item{\code{or}}{True if any of the \code{sources} is true.  The sources must be boolean,
#'        and the actual value here is irrelevant, so \code{mode=c(or="")} is valid.}
#'    }
#'    or at least one of these elemets:
#'    \itemize{
#'      \item{\code{min}}{True if \code{sources} is at least this value. They must be numeric.}
#'      \item{\code{max}}{True if \code{sources} is below this value. They must be numeric.}
#'    }
#' @param required Logical, sets the state of the \code{required_true} attribute. If \code{TRUE},
#'    the plugin submit button is only enabled if this property is true.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the \code{sources}
#'    and \code{mode} value.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'    \code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'    \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}},
#'    \code{\link[rkwarddev:rk.XML.set]{rk.XML.set}},
#'    \code{\link[rkwarddev:rk.XML.switch]{rk.XML.switch}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.convert <- rk.XML.convert(list(string="foo"), mode=c(notequals="bar"))
#' cat(pasteXML(test.convert))

rk.XML.convert <- function(sources, mode=c(), required=FALSE, id.name="auto"){

  # check the mode element
  mode.name <- names(mode)
  if(length(mode) < 1 | length(mode.name) < 1 | length(mode) > 2 | length(mode.name) > 2){
    stop(simpleError("'mode' must have one (or two, if its a range) named element!"))
  } else {}
  if(!all(mode.name %in% c("equals","notequals","and","or","min","max"))){
    stop(simpleError(paste0("The mode you provided is invalid: ", mode.name)))
  } else {}
  if(length(mode) == 2 & !all(mode.name %in% c("min","max"))){
    stop(simpleError("If 'mode' has two elements, they can only be \"min\" and \"max\"!"))
  } else {}

  if(identical(id.name, "auto")){
    sourceValsForID <- sapply(sources, function(this.source){
          return(check.ID(this.source))
      })
    attr.list <- list(id=auto.ids(paste0(paste(sourceValsForID, collapse=""), mode), prefix=ID.prefix("logic")))
  } else if(!is.null(id.name)){
    attr.list <- list(id=id.name)
  } else {
    stop(simpleError("'id.name' must have a value!"))
  }

  # firstly, check the sources. if some are named, contruct proper values
  # for RKWard, like string="foo" should actually be "foo.string"
  src.names <- names(sources)
  if(!is.null(src.names)){
    # check these names if they're valid modifiers here
    sane.src.names <- c()
    for (thisName in src.names){
      sane.src.names <- c(sane.src.names, modif.validity(source="all", modifier=thisName, bool=FALSE))
    }
    sources <- as.character(sapply(1:length(sane.src.names), function(src.no){
        this.modif <- sane.src.names[src.no]
        this.source <- sources[[src.no]]
        if(is.XiMpLe.node(this.source)){
          valid.modif <- modif.validity(source=this.source, modifier=this.modif, bool=FALSE)
        } else {
          valid.modif <- modif.validity(source="all", modifier=this.modif, bool=FALSE)
        }
        if(nchar(valid.modif) > 0){
          new.value <- paste(check.ID(sources[[src.no]], search.environment=TRUE), this.modif, sep=".")
        } else {
          new.value <- check.ID(sources[[src.no]], search.environment=TRUE)
        }
        return(new.value)
      }))
  } else {
    sources <- as.character(sapply(sources, check.ID))
  }

  attr.list[["sources"]] <- paste(sources, collapse=";")

  if(identical(mode.name,"equals")){
    attr.list[["mode"]] <- mode.name
    attr.list[["standard"]] <- as.character(mode)
  } else if(identical(mode.name,"notequals")){
    attr.list[["mode"]] <- mode.name
    attr.list[["standard"]] <- as.character(mode)
  } else if(identical(mode.name,"and")){
    attr.list[["mode"]] <- mode.name
  } else if(identical(mode.name,"or")){
    attr.list[["mode"]] <- mode.name
  } else {
    if("min" %in% mode.name){
      attr.list[["mode"]] <- "range"
      attr.list[["min"]] <- as.numeric(mode["min"])
    } else {}
    if("max" %in% mode.name){
      attr.list[["mode"]] <- "range"
      attr.list[["max"]] <- as.numeric(mode["max"])
    } else {}
  }

  if(isTRUE(required)){
      attr.list[["required_true"]] <- "true"
  } else {}

  node <- XMLNode("convert", attrs=attr.list)

  return(node)
}
