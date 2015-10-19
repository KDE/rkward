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


#' Replace XiMpLe.node objects with their ID value
#' 
#' This function is intended to be used for generating JavaScript code for
#' RKWard plugins. Its sole purpose is to replace objects of class \code{XiMpLe.node}
#' which hold an XML node of some plugin GUI definition, and objects of classes \code{rk.JS.arr},
#' \code{rk.JS.opt} or \code{rk.JS.var} with their ID (or JS variable name), and combine these
#' replacements with character strings.
#' 
#' @param ... One or several character strings and/or \code{XiMpLe.node} objects with plugin nodes,
#'   and/or objects of classes \code{rk.JS.arr}, \code{rk.JS.opt} or \code{rk.JS.var}, simply separated by comma.
#' @param quote Logical, it the character strings should be deparsed, so they come out "as-is" when
#'    written to files, e.g. by \code{cat}.
#' @param collapse Character string, defining if and how the individual elements should be glued together.
#' @param js Logical, if \code{TRUE} returns JavaScript varaible names for \code{XiMpLe.node} objects.
#'    Otherwise their actual ID is returned.
#' @param .objects Alternative way of specifying objects, if you already have them as a list.
#' @return A character string.
#' @export
#' @seealso \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'    \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'    \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'    \code{\link[rkwarddev:echo]{echo}},
#'    \code{\link[rkwarddev:qp]{qp}} (a shortcut for \code{id} with different defaults),
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # an example checkbox XML node
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' id("The variable name is: ", cbox1, "!")

id <- function(..., quote=FALSE, collapse="", js=TRUE, .objects=list(...)){
  ID.content <- sapply(.objects, function(this.part){
      # if this is a plot options object, by default only paste the printout slot
      # and discard the rest
      this.part <- stripCont(this.part, get="printout")

      if(is.XiMpLe.node(this.part)){
        if(identical(XMLName(this.part), "optioncolumn")){
          # optionsets are more difficult to identify automatically
          if(isTRUE(js)){
            node.id <- camelCode(get.IDs(check.optionset.tags(this.part), relevant.tags="optioncolumn")[,"abbrev"])
          } else {
            node.id <- get.IDs(check.optionset.tags(this.part), relevant.tags="optioncolumn")[,"id"]
          }
        } else {
          node.id <- XMLAttrs(this.part)[["id"]]
          if(isTRUE(js)){
            node.id <- camelCode(node.id)
          } else {}
        }
        return(node.id)
      } else if(inherits(this.part, "rk.JS.arr")){
        node.id <- slot(this.part, "opt.name")
        return(node.id)
      } else if(inherits(this.part, "rk.JS.opt")){
        node.id <- slot(this.part, "var.name")
        return(node.id)
      } else if(inherits(this.part, "rk.JS.var")){
        # can hold multiple IDs, but we'll only return the first valid one
        node.id <- paste.JS.var(this.part, names.only=TRUE)
        if(length(node.id) > 1){
          node.id <- node.id[1]
          warning(paste0("Object contained more than one ID, only the first one was used: ", node.id), call.=FALSE)
        } else {}
        return(node.id)
      } else if(inherits(this.part, "rk.JS.echo")){
        node.id <- slot(this.part, "value")
        return(node.id)
      } else if(inherits(this.part, "rk.JS.i18n")){
        # strip all semicolons from i18n calls
        node.id <- slot(this.part, "value")
        return(node.id)
      } else {
        if(isTRUE(quote)){
          text.part <- deparse(this.part)
        } else {
          text.part <- this.part
        }
        return(text.part)
      }
    })
  result <- paste(ID.content, collapse=collapse)
  return(result)
}
