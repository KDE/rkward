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


#' @export

# this simple class is for JavaScript generation,
# produced by rk.JS.array()

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
