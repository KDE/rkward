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


#' Create a value selector for RKWard plugins
#'
#' This function will create a <frame> node including a <valueselector> and a <valueslot> node. It is
#' actually a wrapper for \code{\link[rkwarddev:rk.XML.valueslot]{rk.XML.valueslot}} and
#' \code{\link[rkwarddev:rk.XML.valueselector]{rk.XML.valueselector}}, since you usually won't define one
#' without the other.
#'
#' @param label Character string, a text label for the value browser.
#' @param slot.text Character string, a text label for the value selection slot.
#' @param options A named list with string values to choose from. The names of the list elements will become
#'    labels of the options, \code{val} defines the value to submit if the value is selected, and
#'    \code{chk=TRUE} should be set in the one option which is checked by default. You might also provide an \code{i18n}
#'    for this particular option (see \code{i18n}). Objects generated with \code{\link[rkwarddev:rk.XML.option]{rk.XML.option}}
#'    are accepted as well.
#' @param required Logical, whether the selection of values is mandatory or not.
#' @param multi Logical, whether the valueslot holds only one or several objects.
#' @param duplicates Logical, if \code{multi=TRUE} defines whether the same entry may be added multiple times. Sets \code{multi=TRUE}.
#' @param min If \code{multi=TRUE} defines how many objects must be selected.
#' @param any If \code{multi=TRUE} defines how many objects must be selected at least if any
#'    are selected at all.
#' @param max If \code{multi=TRUE} defines how many objects can be selected in total
#'    (0 means any number).
#' @param horiz Logical. If \code{TRUE}, the valueslot will be placed next to the selector,
#'    if \code{FALSE} below it.
#' @param add.nodes A list of objects of class \code{XiMpLe.node} to be placed after the valueslot.
#' @param frame.label Character string, a text label for the whole frame.
#' @param id.name Character vector, unique IDs for the frame (first entry), the valueselector (second entry)
#'    and valueslot (third entry). If \code{formula.dependent} is not \code{NULL}, a fourth and fifth entry is needed as well,
#'    for the dependent valueslot and the formula node, respectively.
#'    If \code{"auto"}, IDs will be generated automatically from \code{label} and \code{slot.text}.
#' @param help Character string or list of character values and XiMpLe nodes, will be used as the \code{text} value for a setting node in the .rkh file.
#'    If set to \code{FALSE}, \code{\link[rkwarddev:rk.rkh.scan]{rk.rkh.scan}} will ignore this node.
#'    Also needs \code{component} to be set accordingly!
#' @param component Character string, name of the component this node belongs to. Only needed if you
#'    want to use the scan features for automatic help file generation; needs \code{help} to be set
#'    accordingly, too!
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.valueslot]{rk.XML.valueslot}},
#'    \code{\link[rkwarddev:rk.XML.valueselector]{rk.XML.valueselector}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.values <- rk.XML.values("Select some values", "Vars go here")
#' cat(pasteXML(test.values))

rk.XML.values <- function(label, slot.text, options=list(label=c(val=NULL, chk=FALSE, i18n=NULL)),
    required=FALSE, multi=FALSE, duplicates=FALSE, min=1, any=1, max=0,
    horiz=TRUE, add.nodes=NULL, frame.label=NULL, id.name="auto", help=NULL, component=rk.get.comp()){

    if(identical(id.name, "auto")){
        ## if this ID generation get's changed, change it in rk.XML.valueslot(), too!
        value.sel.attr <- list(id=auto.ids(label, prefix=ID.prefix("valueselector", length=3)))
        value.slot.id <- auto.ids(slot.text, prefix=ID.prefix("valueslot", length=4))
    } else if(!is.null(id.name)){
        value.sel.attr <- list(id=id.name[[2]])
        value.slot.id <- id.name[[3]]
    } else {}

    v.selector <- rk.XML.valueselector(
        label=label,
        options=options,
        id.name=value.sel.attr[["id"]])

    v.slot <- rk.XML.valueslot(
        label=slot.text,
        source=v.selector,
        required=required,
        multi=multi,
        duplicates=duplicates,
        min=min,
        any=any,
        max=max,
        id.name=value.slot.id,
        help=help,
        component=component)

    slot.content <- list(v.slot)

    # do we need to add extra nodes to the valueslot?
    if(!is.null(add.nodes)){
        for (this.node in child.list(add.nodes)) {
            slot.content[[length(slot.content)+1]] <- this.node
        }
    } else {}

    if(isTRUE(horiz)){
        values.frame <- rk.XML.frame(
            rk.XML.row(list(rk.XML.col(v.selector), rk.XML.col(slot.content))),
            label=frame.label,
            id.name=id.name[[1]]
        )
    } else {
        values.frame <- rk.XML.frame(
            v.selector,
            label=frame.label,
            id.name=id.name[[1]]
        )
        for (this.node in slot.content) {
            XMLChildren(values.frame) <- append(XMLChildren(values.frame), this.node)
        }
    }

    return(values.frame)
}
