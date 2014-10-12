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

#' Check for rkwarddev package version requirements
#' 
#' @param min The minimum version number of rkwarddev that is required to run this script.
#' @param lib.loc The \code{lib.loc} argument passed over to \code{\link[utils:packageVersion]{packageVersion}}.
#' @return The function has no return value, but wil stop with an error if the specified version requirement is not met.
#' @export
#' @examples
#' rkwarddev.required(min="0.06-5")

rkwarddev.required <- function(min="0.06-5", lib.loc=NULL){

  if(packageVersion("rkwarddev") < min){
    stop(simpleError("please upgrade the rkwarddev package, it is too old to run this script."))
  } else {}

  return(invisible(NULL))
}
