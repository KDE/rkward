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


#' Create XML node "dependency_check" for RKWard pluginmaps
#'
#' @note The \code{<dependency_check>} node was introduced with RKWard 0.6.1, please set the dependencies
#'    of your component/plugin accordingly.
#'
#' @param id.name Character string, a unique ID for this plugin element.
#' @param dependencies A named list with these elements:
#'    \describe{
#'      \item{rkward.min}{Minimum RKWard version needed for this plugin (optional)}
#'      \item{rkward.max}{Maximum RKWard version needed for this plugin (optional)}
#'      \item{R.min}{Minimum R version needed for this plugin (optional)}
#'      \item{R.max}{Maximum R version needed for this plugin (optional)}
#'    }
#' @param package A list of named character vectors, each with these elements:
#'    \describe{
#'      \item{name}{Name of a package this plugin depends on (optional)}
#'      \item{min}{Minimum version of the package (optional)}
#'      \item{max}{Maximum version of the package (optional)}
#'      \item{repository}{Repository to download the package (optional)}
#'    }
#' @param pluginmap A named list with these elements:
#'    \describe{
#'      \item{name}{Identifier of a pluginmap this plugin depends on (optional)}
#'      \item{url}{URL to get the pluginmap (optional)}
#'    }
#' @param hints Logical, if \code{TRUE}, \code{NULL} values will be replaced with example text.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.dependencies]{rk.XML.dependencies}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' dependency_check.node <- rk.XML.dependency_check(
#'   id.name="dep_check",
#'   dependencies=list(
#'     rkward.min="0.5.3",
#'     rkward.max="",
#'     R.min="2.10",
#'     R.max=""),
#'   package=list(
#'     c(name="heisenberg", min="0.11-2", max="",
#'       repository="http://rforge.r-project.org"),
#'     c(name="DreamsOfPi", min="0.2", max="", repository="")),
#'   pluginmap=list(
#'     c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
#' )

rk.XML.dependency_check <- function(id.name, dependencies=NULL, package=NULL, pluginmap=NULL, hints=FALSE){
  # this is basically rk.XML.dependencies() with a different node name and additional ID
  xml.dependency_check <- rk.XML.dependencies(
    dependencies=dependencies,
    package=package,
    pluginmap=pluginmap,
    hints=hints)

  XMLName(xml.dependency_check) <- "dependency_check"
  XMLAttrs(xml.dependency_check)[["id"]] <- id.name

  return(xml.dependency_check)
}
