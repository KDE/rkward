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


#' Create XML node "tabbook" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param tabs An optional named list with objects of class \code{XiMpLe.node} (or a list of these objects).
#'    You must provide one named element for each tab. Use \code{NULL} for tabs without predefined children.
#' @param id.name Character vector, unique IDs for the tabbook (first entry) and all tabs.
#'    If \code{"auto"}, IDs will be generated automatically from the labels.
#'    If \code{NULL}, no IDs will be given.
#' @param i18n Either a character string or a named list with the optional element \code{context},
#'    to give some \code{i18n_context}
#'    information for this node. If set to \code{FALSE}, the attribute \code{label} will be renamed into 
#'    \code{noi18n_label}.
#' @note If a node in \code{tabs} is \code{<insert>}, it is returned as-is, without being nested in \code{<tab>}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.checkboxes <- rk.XML.row(rk.XML.col(
#'   rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
#'   rk.XML.cbox(label="bar", val="bar2")))
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' # combine the above into a tabbook
#' test.tabbook <- rk.XML.tabbook("My Tabbook",
#'   tabs=list("First Tab"=test.checkboxes, "Second Tab"=test.dropdown))
#' cat(pasteXML(test.tabbook))

rk.XML.tabbook <- function(label=NULL, tabs=list(), id.name="auto", i18n=NULL){
  tab.labels <- names(tabs)
  num.tabs <- length(tabs)

  # check if number of children fits
  if("" %in% tab.labels & num.tabs > 0){
    stop(simpleError("All tabs must have a label (named list)!"))
  } else {}

  if(identical(id.name, "auto")){
    tab.ids <- auto.ids(tab.labels, prefix=ID.prefix("tab", length=3))
  } else {}
  tabs <- sapply(1:num.tabs, function(this.num){
      if(is.XiMpLe.node(tabs[[this.num]]) && XMLName(tabs[[this.num]]) %in% c("insert", "tab")){
        return(tabs[[this.num]])
      } else {
        this.tab <- tab.labels[[this.num]]
        attr.list <- list(label=this.tab)
        if(identical(id.name, "auto")){
          attr.list[["id"]] <- tab.ids[[this.num]]
        } else if(!is.null(id.name)){
          attr.list[["id"]] <- id.name[[this.num + 1]]
        } else {}
        if(!is.null(tabs[[this.num]])){
          child <- tabs[[this.num]]
        } else {
          child <- list()
        }
        return(XMLNode("tab",
          attrs=attr.list,
          .children=child.list(child, empty=FALSE))
        )
      }
    })


  if(identical(id.name, "auto")){
    if(!is.null(label)){
      tb.id <- auto.ids(label, prefix=ID.prefix("tabbook", length=4))
    } else {
      # try autogenerating some id
      tb.id <- auto.ids(tab.labels, prefix=ID.prefix("tabbook", length=4))
    }
  } else if(!is.null(id.name)){
    tb.id <- id.name[[1]]
  } else {
    tb.id <- NULL
  }

  tbk.attr.list <- list(id=tb.id)
  if(!is.null(label)){
    tbk.attr.list[["label"]] <- label
  } else {}

  # check for additional i18n info; if FALSE, "label" will be renamed to "noi18n_label"
  tbk.attr.list <- check.i18n(i18n=i18n, attrs=tbk.attr.list)

  tabbook <- XMLNode("tabbook",
      attrs=tbk.attr.list,
      .children=child.list(tabs, empty=FALSE)
    )

  return(tabbook)
}
