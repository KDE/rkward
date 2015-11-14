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

#' Globally define handling of empty else conditions in JS
#' 
#' Some JS functions allow to decide whether empty \code{else} statements should be omitted or printed
#' nonetheless (which some consider more reader friendly). The default can be globally defined with
#' \code{rk.set.empty.e}, so you don't have to specify it in each function call.
#' 
#' \code{rk.get.empty.e} returns the set value, which defaults to \code{FALSE} by default.
#' 
#' @param empty Logical, whether .
#' @return \code{rk.set.empty.e} returns invisible(NULL), \code{rk.get.empty.e} either \code{TRUE} or \code{FALSE}.
#' @rdname rk.set.empty.e
#' @export

rk.set.empty.e <- function(empty=FALSE){
  set.rk.env("empty.e", value=empty)
  return(invisible(NULL))
}
