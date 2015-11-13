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

#' Create R comment with JavaScript in RKWard plugin code
#' 
#' @param ... Character strings to form a comment.
#' @param indent.by A character string defining the indentation string to use. Note that
#'    backslashes need to be escaped (e.g. \code{"\\t"} to produce \code{"\t"}).
#' @param level Integer, which indentation level to use in the resulting R code, minimum is 1.
#' @param newline Character string, can be set to e.g. \code{"\n"} to force a newline after the call.
#' @return A character string.
#' @export
#' @examples
#' cat(R.comment("This will become an R comment"))

R.comment <- function(..., indent.by=rk.get.indent(escape=TRUE), level=2, newline=""){
  message <- paste0(list(...), collapse=" ")
  if(level > 1){
    indentation <- paste0(", \"", rep(indent.by, level-1), "\"")
  } else {
    indentation <- ""
  }
  result <- paste0("comment(\"", message, "\"", indentation, ");", newline)

  return(result)
}
