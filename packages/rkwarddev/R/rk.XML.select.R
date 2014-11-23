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


#' Create XML node "select" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param options A named list with options to choose from. The names of the list elements will become
#'    labels of the options, \code{val} defines the value to submit if the option is selected, and
#'    \code{chk=TRUE} should be set in the one option which is selected by default. You might also provide an \code{i18n}
#'    for this particular option (see \code{i18n}). Objects generated with \code{\link[rkwarddev:rk.XML.option]{rk.XML.option}}
#'    are accepted as well.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"} and a label was provided, an ID will be generated automatically from the label.
#' @param help Character string or list of character values and XiMpLe nodes, will be used as the \code{text} value for a setting node in the .rkh file.
#'    If set to \code{FALSE}, \code{\link[rkwarddev:rk.rkh.scan]{rk.rkh.scan}} will ignore this node.
#'    Also needs \code{component} to be set accordingly!
#' @param component Character string, name of the component this node belongs to. Only needed if you
#'    want to use the scan features for automatic help file generation; needs \code{help} to be set
#'    accordingly, too!
#' @param i18n Either a character string or a named list with the optional element \code{context},
#'    to give some \code{i18n_context}
#'    information for this node. If set to \code{FALSE}, the attribute \code{label} will be renamed into 
#'    \code{noi18n_label}.
#' @return An object of class \code{XiMpLe.node}.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' test.select <- rk.XML.select("myselect",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' cat(pasteXML(test.select))

rk.XML.select <- function(label, options=list(label=c(val="", chk=FALSE, i18n=NULL)), id.name="auto",
  help=NULL, component=rk.get.comp(), i18n=NULL){
  if(identical(id.name, "auto")){
    id <- auto.ids(label, prefix=ID.prefix("select"))
  } else {
    id <- id.name
  }
  attr.list <- list(id=id, label=label)

  # convert list elements into a list of XiMpLe nodes (if they aren't already)
  sl.options <- rk.check.options(options, parent="select")

  # check the node names and allow only valid ones
  valid.child("select", children=sl.options)

  # check for additional i18n info; if FALSE, "label" will be renamed to "noi18n_label"
  attr.list <- check.i18n(i18n=i18n, attrs=attr.list)

  select <- XMLNode("select",
      attrs=attr.list,
      .children=child.list(sl.options, empty=FALSE)
    )

  # check for .rkh content
  rk.set.rkh.prompter(component=component, id=id, help=help)

  return(select)
}
