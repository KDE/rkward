\name{rk.plugin.skeleton}
\alias{rk.plugin.skeleton}
\title{Create skeleton for RKWard plugins}
\usage{
  rk.plugin.skeleton(name, about, path = tempdir(), dialog
  = list(), dial.require = c(), overwrite = FALSE, wizard =
  FALSE, tests = TRUE, lazyLoad = TRUE)
}
\arguments{
  \item{name}{Character sting, name of the plugin package.}

  \item{about}{A list with descriptive information on the
  plugin, its authors and dependencies. At the very least
  you must specify \code{name} and \code{author}. See
  \code{\link[XiMpLe:rk.XML.about]{rk.XML.about}} for
  details and a full list of elements!}

  \item{path}{Character sting, path to the main directory
  where the skeleton should be created.}

  \item{dialog}{A list of objects of class XiMpLe.node. If
  provided, will be included in the created plugin XML file
  as the dialog.}

  \item{dial.require}{A character vector with names of R
  packages that the dialog requires.}

  \item{overwrite}{Logical, whether existing files should
  be replaced. Defaults to \code{FALSE}.}

  \item{wizard}{Logical, whether a \code{<wizard>} section
  should be added to the \code{<dialog>} section. Defaults
  to \code{FALSE}.}

  \item{tests}{Logical, whether directories and files for
  plugin tests should be created. Defaults to \code{TRUE}.}
}
\description{
  Create skeleton for RKWard plugins
}
\examples{
\dontrun{
# a simple example with only basic information
about.info <- list(
name="Square the circle",
author=c(
person(given="E.A.", family="Dölle",
email="doelle@eternalwondermaths.example.org", role="aut"),
person(given="A.", family="Assistant",
email="alterego@eternalwondermaths.example.org", role=c("cre","ctb"))
))

rk.plugin.skeleton("Square the Circle", about=about.info)

# a more complex example, already including some dialog elements
about.info <- list(
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

test.dropdown <- rk.XML.dropdown("mydrop",
opts=list("First Option"=c(val="val1"),
"Second Option"=c(val="val2", chk=TRUE)))
test.checkboxes <- rk.XML.row(rk.XML.col(
list(test.dropdown,
rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
rk.XML.cbox(label="bar", val="bar2"))
))
test.vars <- rk.XML.vars("select some vars", "vars go here")
test.tabbook <- rk.XML.tabbook("My Tabbook", tab.labels=c("First Tab",
"Second Tab"), children=list(test.checkboxes, test.vars))
test.plugin <- rk.XML.plugin("My test", label="Check this out",
children=test.tabbook)

rk.plugin.skeleton("Square the Circle", about=about.info,
dialog=test.tabbook, overwrite=TRUE)
}
}

