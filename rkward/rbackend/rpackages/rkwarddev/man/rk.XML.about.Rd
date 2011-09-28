\name{rk.XML.about}
\alias{rk.XML.about}
\title{Create XML node "about" for RKWard pluginmaps}
\usage{
  rk.XML.about(name, author, about = list(desc =
  "SHORT_DESCRIPTION", version = "0.01-0", date =
  Sys.Date(), url = "http://EXAMPLE.com", license = "GPL
  (>= 3)"), dependencies = NULL, package = NULL, pluginmap
  = NULL)
}
\arguments{
  \item{name}{A character string with the plugin name.}

  \item{author}{A vector of objects of class \code{person}
  with these elements (mandatory): \describe{
  \item{given}{Author given name} \item{family}{Author
  family name} \item{email}{Author mail address}
  \item{role}{This person's specific role, e.g.
  \code{"aut"} for actual author, \code{"cre"} for
  maintainer or \code{"ctb"} for contributor.} } See
  \code{\link[utils:person]{person}} for more details on
  this, especially for valid roles.}

  \item{about}{A named list with these elements: \describe{
  \item{desc}{A short description (mandatory)}
  \item{version}{Plugin version (mandatory)}
  \item{date}{Release date (mandatory)} \item{url}{URL for
  the plugin (optional)} \item{license}{License the plugin
  is distributed under (mandatory)} \item{category}{A
  category for this plugin (optional)} }}

  \item{dependencies}{A named list with these elements:
  \describe{ \item{rkward.min}{Minimum RKWard version
  needed for this plugin (optional)}
  \item{rkward.max}{Maximum RKWard version needed for this
  plugin (optional)} \item{R.min}{Minimum R version needed
  for this plugin (optional)} \item{R.max}{Maximum R
  version needed for this plugin (optional)} }}

  \item{package}{A list of named character vectors, each
  with these elements: \describe{ \item{name}{Name of a
  package this plugin depends on (optional)}
  \item{min}{Minimum version of the package (optional)}
  \item{max}{Maximum version of the package (optional)}
  \item{repository}{Repository to download the package
  (optional)} }}

  \item{pluginmap}{A named list with these elements:
  \describe{ \item{name}{Identifier of a pluginmap this
  plugin depends on (optional)} \item{url}{URL to get the
  pluginmap (optional)} }}
}
\description{
  Create XML node "about" for RKWard pluginmaps
}
\examples{
about.node <- rk.XML.about(
name="Square the circle",
author=c(
person(given="E.A.", family="Dölle",
email="doelle@eternalwondermaths.example.org", role="aut"),
person(given="A.", family="Assistant",
email="alterego@eternalwondermaths.example.org", role=c("cre","ctb"))
),
about=list(
desc="Squares the circle using Heisenberg compensation.",
version="0.1-3",
date=Sys.Date(),
url="http://eternalwondermaths.example.org/23/stc.html",
license="GPL",
category="Geometry"),
dependencies=list(
rkward.min="0.5.3",
rkward.max="",
R.min="2.10",
R.max=""),
package=list(
c(name="heisenberg", min="0.11-2", max="",
repository="http://rforge.r-project.org"),
c(name="DreamsOfPi", min="0.2", max="", repository="")),
pluginmap=list(
c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
)

cat(pasteXMLNode(about.node, shine=2))
}

