\name{rk.plugin.skeleton}
\alias{rk.plugin.skeleton}
\title{Create skeleton for RKWard plugins}
\usage{
  rk.plugin.skeleton(name, about = NULL, path = tempdir(),
  dialog = NULL, wizard = NULL, logic = NULL, snippets =
  NULL, provides = c("logic", "dialog"), dial.require =
  c(), overwrite = FALSE, tests = TRUE, lazyLoad = TRUE,
  menu = "test", results.header = NULL, JS.prep = NULL,
  JS.calc = NULL, JS.prnt = NULL, create = c("pmap", "xml",
  "js", "rkh", "desc"), edit = FALSE, load = FALSE, show =
  FALSE)
}
\arguments{
  \item{name}{Character sting, name of the plugin package.}

  \item{about}{A list with descriptive information on the
  plugin, its authors and dependencies. At the very least
  you must specify \code{name} and \code{author}. See
  \code{\link[rkwarddev:rk.XML.about]{rk.XML.about}} for
  details and a full list of elements! If \code{NULL}, no
  \code{DESCRIPTION} file will be created either.}

  \item{path}{Character sting, path to the main directory
  where the skeleton should be created.}

  \item{dialog}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<dialog>} section. See
  \code{\link[rkwarddev:rk.XML.dialog]{rk.XML.dialog}} for
  details.}

  \item{wizard}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<wizard>} section. See
  \code{\link[rkwarddev:rk.XML.wizard]{rk.XML.wizard}} for
  details.}

  \item{logic}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<logic>} section. See
  \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}} for
  details.}

  \item{snippets}{An object of class \code{XiMpLe.node} to
  be pasted as the \code{<snippets>} section. See
  \code{\link[rkwarddev:rk.XML.snippets]{rk.XML.snippets}}
  for details.}

  \item{provides}{Character vector with possible entries of
  \code{"logic"}, \code{"dialog"} or \code{"wizard"},
  defining what sections the document should provide even
  if \code{dialog}, \code{wizard} and \code{logic} are
  \code{NULL}. These sections must be edited manually and
  some parts are therefore commented out.}

  \item{dial.require}{A character vector with names of R
  packages that the dialog requires.}

  \item{overwrite}{Logical, whether existing files should
  be replaced. Defaults to \code{FALSE}.}

  \item{tests}{Logical, whether directories and files for
  plugin tests should be created. Defaults to \code{TRUE}.}

  \item{lazyLoad}{Logical, whether the package should be
  prepared for lazy loading or not. Should be left
  \code{TRUE}, unless you have very good reasons not to.}

  \item{menu}{A character string with instructions where to
  place the plugin in the menu hierarchy, Valid values are
  \code{"file"}, \code{"edit"}, \code{"view"},
  \code{"workspace"}, \code{"run"}, \code{"data"},
  \code{"analysis"}, \code{"plots"},
  \code{"distributions"}, \code{"windows"},
  \code{"settings"} and \code{"help"}. Anything else will
  place it in a "test" menu.}

  \item{results.header}{A character string to headline the
  printed results.}

  \item{JS.prep}{A character string with JavaScript code to
  be included in the \code{preprocess()} function. This
  string will be pasted as-is, see
  \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}.}

  \item{JS.calc}{A character string with JavaScript code to
  be included in the \code{calculate()} function. This
  string will be pasted as-is, see
  \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}.}

  \item{JS.prnt}{A character string with JavaScript code to
  be included in the \code{printout()} function. This
  string will be pasted as-is, see
  \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}.}

  \item{create}{A character vector with one or more of
  these possible entries: \describe{
  \item{\code{"pmap"}}{Create the \code{.pluginmap} file.}
  \item{\code{"xml"}}{Create the plugin \code{.xml} XML
  file skeleton.} \item{\code{"js"}}{Create the plugin
  \code{.js} JavaScript file skeleton.}
  \item{\code{"rkh"}}{Create the plugin \code{.rkh} help
  file skeleton.} \item{\code{"desc"}}{Create the
  \code{DESCRIPTION} file.} } Default is to create all of
  these files. Existing files will only be overwritten if
  \code{overwrite=TRUE}.}

  \item{edit}{Logical, if \code{TRUE} RKWard will
  automatically open the created files for editing, by
  calling \code{rk.edit.files}. This applies to all files
  defined in \code{create}.}

  \item{load}{Logical, if \code{TRUE} and \code{"pmap"} in
  \code{create}, RKWard will automatically add the created
  .pluginmap file to its menu structure by calling
  \code{rk.load.pluginmaps}. You can then try the plugin
  immediately.}

  \item{show}{Logical, if \code{TRUE} and \code{"pmap"} in
  \code{create}, RKWard will automatically call the created
  plugin after it was loaded (i.e., this implies and also
  sets \code{load=TRUE}).}
}
\value{
  Character string with the path to the plugin root
  directory.
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
test.tabbook <- rk.XML.dialog(rk.XML.tabbook("My Tabbook", tab.labels=c("First Tab",
"Second Tab"), children=list(test.checkboxes, test.vars)))

rk.plugin.skeleton("Square the Circle", about=about.info,
dialog=test.tabbook, overwrite=TRUE)
}
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

