\name{rk.XML.about}
\alias{rk.XML.about}
\title{Create XML node "about" for RKWard pluginmaps}
\usage{
  rk.XML.about(about, author, dependencies = NULL, package
  = NULL, pluginmap = NULL)
}
\arguments{
  \item{about}{A named list with these elements: \describe{
  \item{name} {Plugin name} \item{desc} {A short
  description} \item{version} {Plugin version} \item{date}
  {Release date} \item{url} {URL for the plugin}
  \item{license} {License the plugin is distributed under}
  \item{category} {An optional category} }}

  \item{author}{A list of named character vectors with
  these elements: \describe{ \item{name} {Full author name}
  \item{email} {Author mail address} \item{url} {Author
  homepage} }}

  \item{dependencies}{A named list with these elements:
  \describe{ \item{rkward.min} {Minimum RKWard version
  needed for this plugin} \item{rkward.max} {Maximum RKWard
  version needed for this plugin} \item{R.min} {Minimum R
  version needed for this plugin} \item{R.max} {Maximum R
  version needed for this plugin} }}

  \item{package}{A list of named character vectors, each
  with these elements: \describe{ \item{name} {Name of a
  package this plugin depends on} \item{min} {Minimum
  version of the package} \item{max} {Maximum version of
  the package} \item{repository} {Repository to download
  the package} }}

  \item{pluginmap}{A named list with these elements:
  \describe{ \item{name} {Identifier of a pluginmap this
  plugin depends on} \item{url} {URL to get the pluginmap}
  }}
}
\description{
  Create XML node "about" for RKWard pluginmaps
}
\examples{
about.node <- rk.XML.about(
about=list(
name="Square the circle",
desc="Squares the circle using Heisenberg compensation.",
version="0.1-3",
date=Sys.Date(),
url="http://eternalwondermaths.example.org/23/stc.html",
license="GPL",
category="Geometry"),
author=list(
c(name="E.A. Dölle", email="doelle@eternalwondermaths.example.org",
url="http://eternalwondermaths.example.org"),
c(name="A. Assistant", email="alterego@eternalwondermaths.example.org",
url="http://eternalwondermaths.example.org/staff/")
),
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

cat(pasteXMLNode(about.node))
}

