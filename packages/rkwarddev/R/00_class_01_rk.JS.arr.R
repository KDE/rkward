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

#' S4 Class rk.JS.arr
#' 
#' This simple class is used for JavaScript generation and produced by
#' \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}}. You shouldn't
#' need to temper with this type of class manually.
#' 
#' @slot arr.name Character string, name of the array variable.
#' @slot opt.name Character string, name of the option variable.
#' @slot IDs Character vector of IDs.
#' @slot variables Character vector of variables.
#' @slot funct Character string, name of an R function call.
#' @slot quote Logical, should values be quoted?
#' @slot option Character string, name of the option to set.
#' @keywords Classes
#' @rdname rk.JS.arr-class
#' @export

setClass("rk.JS.arr",
  representation=representation(
    arr.name="character",
    opt.name="character",
    IDs="vector",
    variables="vector",
    funct="character",
    quote="logical",
    option="character"
  ),
  prototype(
    arr.name=character(),
    opt.name=character(),
    IDs=c(),
    variables=c(),
    funct="c",
    quote=FALSE,
    option=character()
  )
)
