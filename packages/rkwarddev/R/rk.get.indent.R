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

#' @rdname rk.set.indent
#' @param escape Logical, if set to \code{TRUE} each occurring "\\t" will be escaped by an additional "\\".
#' @export
rk.get.indent <- function(escape=FALSE){
  indent.by <- get.rk.env("indent.by", value="\t")
  if(isTRUE(escape)){
    indent.by <- gsub("\t", "\\\\t", indent.by)
  } else {}
  return(indent.by)
}
