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


#' Create XML node "radio" for RKWard plugins
#' 
#' @note It is also possible to address a particular option by giving it an ID, probably useful
#' in logic sections. Have a look at \code{\link[rkwarddev:rk.XML.option]{rk.XML.option}} for details.
#'
#' @param label Character string, a text label for this plugin element.
#' @param options A named list with options to choose from. The names of the list elements will become
#'    labels of the options, \code{val} defines the value to submit if the option is checked, and
#'    \code{chk=TRUE} should be set in the one option which is checked by default. Objects generated with
#'    \code{\link[rkwarddev:rk.XML.option]{rk.XML.option}} are accepted as well.
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"} and a label was provided, an ID will be generated automatically from the label.
#' @param help Character string, will be used as the \code{text} value for a setting node in the .rkh file.
#'    Also needs \code{component} to be set accordingly!
#' @param component Character string, name of the component this node belongs to. Only needed if you
#'    want to use the scan features for automatic help file generation; needs \code{help} to be set
#'    accordingly, too!
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.option]{rk.XML.option}},
#'    \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.radio <- rk.XML.radio("Chose one",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' cat(pasteXML(test.radio))

rk.XML.radio <- function(label, options=list(label=c(val=NULL, chk=FALSE)), id.name="auto", help=NULL, component=rk.get.comp()){
  if(identical(id.name, "auto")){
    id <- auto.ids(label, prefix=ID.prefix("radio"))
  } else {
    id <- id.name
  }
  rd.attr.list <- list(id=id, label=label)

  # convert list elements into a list of XiMpLe nodes (if they aren't already)
  rd.options <- rk.check.options(options, parent="radio")

  radio <- XMLNode("radio",
      attrs=rd.attr.list,
      .children=child.list(rd.options, empty=FALSE)
    )

  # if present, store option IDs with parent ID 
  rk.register.options(options, parent.node=radio)

  # check for .rkh content
  rk.set.rkh.prompter(component=component, id=id, help=help)

  return(radio)
}
