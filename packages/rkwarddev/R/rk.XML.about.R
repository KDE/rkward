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


#' Create XML node "about" for RKWard pluginmaps
#'
#' @param name A character string with the plugin name.
#' @param author A vector of objects of class \code{person} with these elements (mandatory):
#'    \describe{
#'      \item{given}{Author given name}
#'      \item{family}{Author family name}
#'      \item{email}{Author mail address (can be omitted if \code{role} does not include \code{"cre"})}
#'      \item{role}{This person's specific role, e.g. \code{"aut"} for actual author, \code{"cre"} for maintainer or \code{"ctb"} for contributor.}
#'    }
#'    See \code{\link[utils:person]{person}} for more details on this, especially for valid roles.
#' @param about A named list with these elements:
#'    \describe{
#'      \item{desc}{A short description (mandatory)}
#'      \item{version}{Plugin version (mandatory)}
#'      \item{date}{Release date (mandatory)}
#'      \item{url}{URL for the plugin (optional)}
#'      \item{license}{License the plugin is distributed under (mandatory)}
#'      \item{category}{A category for this plugin (optional)}
#'      \item{long.desc}{A long description (optional, defaults to \code{desc})}
#'    }
#' @param dependencies Deprecated, use \code{\link[rkwarddev:rk.XML.dependencies]{rk.XML.dependencies}} instead.
#' @param package Deprecated, use \code{\link[rkwarddev:rk.XML.dependencies]{rk.XML.dependencies}} instead.
#' @param pluginmap Deprecated, use \code{\link[rkwarddev:rk.XML.dependencies]{rk.XML.dependencies}} instead.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.dependencies]{rk.XML.dependencies}},
#'    \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' about.node <- rk.XML.about(
#'   name="Square the circle",
#'   author=c(
#'     person(given="E.A.", family="Dölle",
#'       email="doelle@@eternalwondermaths.example.org", role="aut"),
#'     person(given="A.", family="Assistant",
#'       email="alterego@@eternalwondermaths.example.org", role=c("cre","ctb"))
#'     ),
#'   about=list(
#'     desc="Squares the circle using Heisenberg compensation.",
#'     version="0.1-3",
#'     date=Sys.Date(),
#'     url="http://eternalwondermaths.example.org/23/stc.html",
#'     license="GPL",
#'     category="Geometry")
#' )
#' 
#' cat(pasteXML(about.node, shine=2))


rk.XML.about <- function(name, author, about=list(desc="SHORT_DESCRIPTION", version="0.01-0", date=Sys.Date(), url="http://EXAMPLE.com", license="GPL (>= 3)", long.desc=NULL), dependencies=NULL, package=NULL, pluginmap=NULL){
  # sanity checks
  stopifnot(all(length(name), length(author)) > 0)
  if(is.null(about)){
    about <- list()
  } else {}
  if(!"desc" %in% names(about)){
    about[["desc"]] <- "SHORT_DESCRIPTION"
  } else {}
  if(!"version" %in% names(about)){
    about[["version"]] <- "0.01-0"
  } else {}
  if(!"date" %in% names(about)){
    about[["date"]] <- Sys.Date()
  } else {}
  if(!"url" %in% names(about)){
    about[["url"]] <- "http://EXAMPLE.com"
  } else {}
  if(!"license" %in% names(about)){
    about[["license"]] <- "GPL (>= 3)"
  } else {}
  if(!"long.desc" %in% names(about) | is.null(about[["long.desc"]])){
    about[["long.desc"]] <- about[["desc"]]
  } else {}

  ## author
  # - given
  # - family
  # - email
  # - role
  xml.authors <- unlist(sapply(author, function(this.author){
      stopifnot(all(c("given", "family") %in% names(unlist(this.author))))
      author.given  <- format(this.author, include="given")
      author.family <- format(this.author, include="family")
      if("email" %in% names(unlist(this.author))){
        author.email  <- format(this.author, include="email", braces=list(email=""))
      } else {
        author.email  <- NULL
      }
      author.role   <- format(this.author, include="role", braces=list(role=""), collapse=list(role=", "))
      # at least maintainers need an email address
      if("cre" %in% unlist(this.author) & is.null(author.email)){
        stop(simpleError("the maintainer ", author.given, " ", author.family, " needs an email address!"))
      } else {}
     
      result <- XMLNode("author",
        attrs=list(
          given=author.given,
          family=author.family,
          email=author.email,
          role=author.role
        ))
      return(result)
    }))

  ###################
  ## the 'package' code is about to be removed in a future release
  ###################
    ## package
    # - name
    # - min="min_version",
    # - max="max_version",
    # - repository
    # create example, if empty
    if(is.null(package)){
      xml.package <- list()
    } else {
      warning("<package> inside <about> is deprecated, use rk.XML.dependencies() instead!")
      xml.package <- sapply(package, function(this.package){
          pck.options <- names(this.package)
          pck.attributes <- list(name=this.package[["name"]])
          for (this.option in c("min", "max","repository" )){
            if(this.option %in% pck.options){
              pck.attributes[[this.option]] <- this.package[[this.option]]
            } else {}
          }
          result <- XMLNode("package", attrs=pck.attributes)
          return(result)
        })
    }

  ###################
  ## the 'pluginmap' code is about to be removed in a future release
  ###################
    ## pluginmap
    # - name,
    # - url
    # create example, if empty
    if(is.null(pluginmap)){
      xml.pluginmap <- list()
    } else {
      warning("<pluginmap> inside <about> is deprecated, use rk.XML.dependencies() instead!")
      xml.pluginmap <- sapply(pluginmap, function(this.pluginmap){
          result <- XMLNode("pluginmap",
            attrs=list(
              name=this.pluginmap[["name"]],
              url=this.pluginmap[["url"]]
            ))
          return(result)
        })
    }

  ###################
  ## the 'dependencies' code is about to be removed in a future release
  ###################
    ## dependencies
    # - rkward.min="rkward_min_version",
    # - rkward.max="rkward_max_version",
    # - R.min="R_min_version",
    # - R.max="R_max_version"
    # + package
    # + pluginmap
    for (pmap in xml.pluginmap){
      xml.package[[length(xml.package)+1]] <- pmap
    }
    if(is.null(dependencies)){
      xml.dependencies <- list()
    } else {
      warning("<dependencies> inside <about> is deprecated, use rk.XML.dependencies() instead!")
      dep.options <- names(dependencies)
      dep.attributes <- list()
      if("rkward.min" %in% dep.options){
        dep.attributes[["rkward_min_version"]] <- dependencies[["rkward.min"]]
      } else {}
      if("rkward.max" %in% dep.options){
        dep.attributes[["rkward_max_version"]] <- dependencies[["rkward.max"]]
      } else {}
      if("R.min" %in% dep.options){
        dep.attributes[["R_min_version"]] <- dependencies[["R.min"]]
      } else {}
      if("R.max" %in% dep.options){
        dep.attributes[["R_max_version"]] <- dependencies[["R.max"]]
      } else {}
      xml.dependencies <- XMLNode("dependencies",
        attrs=dep.attributes,
        .children=child.list(xml.package, empty=FALSE))
    }

  ## about
  # - name
  # - desc="shortinfo",
  # - version
  # - date="releasedate",
  # - url
  # - license
  # - category
  # + authors
  # + dependencies
  xml.authors[[length(xml.authors)+1]] <- xml.dependencies
  if(is.null(xml.authors)){
    xml.authors <- list()
  } else {}
  xml.about <-  XMLNode("about",
    attrs=list(
      name=name,
      "shortinfo"=about[["desc"]],
      "longinfo"=about[["long.desc"]],
      version=about[["version"]],
      "releasedate"=about[["date"]],
      url=about[["url"]],
      license=about[["license"]],
      category=about[["category"]]
    ),
    .children=xml.authors)

  return(xml.about)
}
