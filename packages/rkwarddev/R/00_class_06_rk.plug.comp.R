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


#' S4 Class rk.plug.comp
#' 
#' This simple class is used for JavaScript generation. It holds plugin components,
#' i.e. single dialogs, to add to a plugin skeleton, and is produced by
#' \code{\link{rk.plugin.component}}. You shouldn't need to temper with this
#' type of class manually.
#' 
#' @slot name Character string, name of the plugin.
#' @slot create Charactervector defining the component parts/files to be created.
#' @slot xml An object of class XiMpLe.doc containig the plugin XML code.
#'    See \code{\link{rk.XML.plugin}}.
#' @slot js A character string containing the plugin JavaScript code.
#'    See \code{\link{rk.JS.doc}}.
#' @slot rkh An object of class XiMpLe.doc containig the plugin help page.
#'    See \code{\link{rk.rkh.doc}}.
#' @slot hierarchy A list defining where to place the component in the menu structure.
#' @keywords Classes
#' @rdname rk.plug.comp-class
#' @include rk.XML.plugin.R
#' @include rk.JS.doc.R
#' @include rk.rkh.doc.R
#' @export

setClass("rk.plug.comp",
  representation=representation(
    name="character",
    create="vector",
    xml="XiMpLe.doc",
    js="character",
    rkh="XiMpLe.doc",
    hierarchy="list"
  ),
  prototype(
    name=character(),
    create=c(),
    xml=new("XiMpLe.doc"),
    js=character(),
    rkh=new("XiMpLe.doc"),
    hierarchy=list()
  )
)

# setValidity("rk.plug.comp", function(object){
#   return(TRUE)
# })
