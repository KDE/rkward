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


#' Create XML node "formula" for RKWard plugins
#'
#' If \code{fixed} or \code{dependent} are objects of class \code{XiMpLe.node},
#' their \code{id} will be extracted and used.
#'
#' @param fixed The \code{id} of the varslot holding the selected fixed factors.
#' @param dependent The \code{id} of the varslot holding the selected dependent variable.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the \code{fixed} and \code{dependent} value.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.varselector]{rk.XML.varselector}},
#'    \code{\link[rkwarddev:rk.XML.varslot]{rk.XML.varslot}},
#'    \code{\link[rkwarddev:rk.XML.vars]{rk.XML.vars}} (a wrapper, including formula),
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.varselector <- rk.XML.varselector("Select some vars")
#' test.varslot1 <- rk.XML.varslot("Fixed factors", source=test.varselector)
#' test.varslot2 <- rk.XML.varslot("Dependent variables", source=test.varselector)
#' test.formula <- rk.XML.formula(fixed=test.varslot1, dependent=test.varslot2)
#' cat(pasteXML(test.formula))

rk.XML.formula <- function(fixed, dependent, id.name="auto"){
  # check if these are actually varslots
  sapply(list(fixed, dependent), function(this.attr){
      if(inherits(this.attr, "XiMpLe.node")){
        this.attr.name <- slot(this.attr ,"name")
        if(!identical(this.attr.name, "varslot")){
          stop(simpleError(paste0("'fixed' and 'dependent' must be <varslot> nodes! You provided: <", this.attr.name, ">")))
        } else {}
      } else {}
    })

  fixed.id <- check.ID(fixed)
  depnd.id <- check.ID(dependent)

  if(identical(id.name, "auto")){
    attr.list <- list(id=auto.ids(paste0(fixed.id, depnd.id), prefix=ID.prefix("formula")))
  } else if(!is.null(id.name)){
    attr.list <- list(id=id.name)
  } else {
    stop(simpleError("'id.name' must have a value!"))
  }

  attr.list[["fixed_factors"]] <- fixed.id
  attr.list[["dependent"]] <- depnd.id

  node <- XMLNode("formula", attrs=attr.list)

  return(node)
}
