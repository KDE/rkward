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

#' Globally define the indentation string
#' 
#' Many functions allow to manually set the indentation string that should be used
#' for code formatting. The default string used can be globally defined with \code{rk.set.indent},
#' so you don't have to specify it in each function call.
#' 
#' \code{rk.get.indent} returns the set value, which defaults to a tab character by default.
#' 
#' @param by Character string, indentation string to be defined globally.
#' @return \code{rk.set.indent} returns invisible(NULL), \code{rk.get.indent} a character string.
#' @rdname rk.set.indent
#' @export

rk.set.indent <- function(by="\t"){
  indent.by <- rk.get.indent()
  indent.by <- by
  set.rk.env("indent.by", value=indent.by)
  return(invisible(NULL))
}
