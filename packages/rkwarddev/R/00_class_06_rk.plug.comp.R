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


#' @include rk.XML.plugin.R
#' @include rk.JS.doc.R
#' @include rk.rkh.doc.R
#' @export

# this class holds plugin components, i.e. single dialogs, to add to a plugin skeleton
# produced by rk.plugin.component()

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
