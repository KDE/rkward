# Copyright 2015 Meik Michalke <meik.michalke@hhu.de>
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

#' Evaluate code in a local environment
#' 
#' Can be used like \code{\link[base:local]{local}}, but evaluation is being done in a speacial
#' local environment of the rkwarddev package. This can be neccessary if you want to call functions
#' nested insinde \code{\link[rkwarddev:js]{js}}, because it might not find all objects if they were
#' only defined in a standard local environment.
#' 
#' @param ... The code to be evaluated.
#' @return The result of evaluating the object(s).
#' @export

rk.local <- function(...){
  rm(list=ls(), envir=.rk.local.env)
  local(..., envir=.rk.local.env)
}
