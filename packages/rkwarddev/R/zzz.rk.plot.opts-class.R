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

# it seems "@include" only works for classes, not for functions (rk.XML.embed()) :-/
# so for the time being, this file is renamed to be parsed at the end

#' S4 Class rk.plot.opts
#' 
#' This simple class is used for JavaScript generation and is produced by
#' \code{\link{rk.plotOptions}}. You shouldn't need to temper with this
#' type of class manually.
#' 
#' @slot XML An object of class XiMpLe.node.
#' @slot preprocess An object of class rk.JS.var.
#' @slot printout An object of class rk.JS.var.
#' @slot calculate An object of class rk.JS.var.
#' @keywords Classes
#' @rdname rk.plot.opts-class
#' @include rk.XML.embed.R
#' @include 00_class_02_rk.JS.var.R
#' @include rk.JS.vars.R
#' @export

setClass("rk.plot.opts",
  representation=representation(
    XML="XiMpLe.node",
    preprocess="rk.JS.var",
    printout="rk.JS.var",
    calculate="rk.JS.var"
  ),
  prototype(
    XML=rk.XML.embed(component="rkward::plot_options", button=TRUE, label="Generic plot options", id.name="auto"),
    preprocess=rk.JS.vars(
      rk.XML.embed(component="rkward::plot_options", button=TRUE, label="Generic plot options", id.name="auto"),
      modifiers="code.preprocess", check.modifiers=FALSE),
    printout=rk.JS.vars(
      rk.XML.embed(component="rkward::plot_options", button=TRUE, label="Generic plot options", id.name="auto"),
      modifiers="code.printout", check.modifiers=FALSE),
    calculate=rk.JS.vars(
      rk.XML.embed(component="rkward::plot_options", button=TRUE, label="Generic plot options", id.name="auto"),
      modifiers="code.calculate", check.modifiers=FALSE)
  )
)

# setValidity("rk.plot.opts", function(object){
#   return(TRUE)
# })
