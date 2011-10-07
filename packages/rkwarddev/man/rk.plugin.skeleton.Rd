\name{rk.plugin.skeleton}
\alias{rk.plugin.skeleton}
\title{Generate skeletons for RKWard plugins}
\usage{
  rk.plugin.skeleton(name, about = NULL, path = tempdir(),
  provides = c("logic", "dialog"), scan = c("var",
  "saveobj", "settings"), xml = list(), js = list(),
  pluginmap = list(), rkh = list(), overwrite = FALSE,
  tests = TRUE, lazyLoad = TRUE, create = c("pmap", "xml",
  "js", "rkh", "desc"), edit = FALSE, load = FALSE, show =
  FALSE, indent.by = "\t")
}
\arguments{
  \item{name}{Character sting, name of the plugin package.}

  \item{about}{An object of class \code{XiMpLe.node} with
  descriptive information on the plugin, its authors and
  dependencies, see
  \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for
  details. Skipped if \code{NULL}. If \code{NULL}, no
  \code{DESCRIPTION} file will be created either.}

  \item{path}{Character sting, path to the main directory
  where the skeleton should be created.}

  \item{provides}{Character vector with possible entries of
  \code{"logic"}, \code{"dialog"} or \code{"wizard"},
  defining what sections the GUI XML file should provide
  even if \code{dialog}, \code{wizard} and \code{logic} are
  \code{NULL}. These sections must be edited manually and
  some parts are therefore commented out.}

  \item{scan}{A character vector to trigger various
  automatic scans of the generated GUI XML file. Valid
  enties are: \describe{ \item{\code{"var"}}{Calls
  \code{\link{rk.JS.scan}} to define all needed variables
  in the \code{calculate()} function of the JavaScript
  file. These variables will be added to variables defined
  by the \code{js} option, if any (see below).}
  \item{\code{"saveobj"}}{Calls \code{\link{rk.JS.saveobj}}
  to generate code to save R results in the
  \code{printout()} function of the JavaScript file. This
  code will be added to the code defined by the \code{js}
  option, if any (see below).}
  \item{\code{"settings"}}{Calls \code{\link{rk.rkh.scan}}
  to generate \code{<setting>} sections for each relevant
  GUI element in the \code{<settings>} section of the help
  file. This option will be overruled if you provide that
  section manually by the \code{rkh} option (see below).}
  }}

  \item{xml}{A named list of options to be forwarded to
  \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}}, to
  generate the GUI XML file. Not all options are supported
  because some don't make sense in this context. Valid
  options are: \code{"dialog"}, \code{"wizard"},
  \code{"logic"} and \code{"snippets"}. If not set, their
  default values are used. See
  \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}} for
  details.}

  \item{js}{A named list of options to be forwarded to
  \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}, to generate
  the JavaScript file. Not all options are supported
  because some don't make sense in this context. Valid
  options are: \code{"require"}, \code{"results.header"},
  \code{"variables"}, \code{"preprocess"},
  \code{"calculate"} and \code{"printout"}. If not set,
  their default values are used. See
  \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}} for
  details.}

  \item{pluginmap}{A named list of options to be forwarded
  to
  \code{\link[rkwarddev:rk.XML.pluginmap]{rk.XML.pluginmap}},
  to generate the pluginmap file. Not all options are
  supported because some don't make sense in this context.
  Valid options are: \code{"hierarchy"} and
  \code{"require"}. If not set, their default values are
  used. See
  \code{\link[rkwarddev:rk.XML.pluginmap]{rk.XML.pluginmap}}
  for details.}

  \item{rkh}{A named list of options to be forwarded to
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}, to
  generate the help file. Not all options are supported
  because some don't make sense in this context. Valid
  options are: \code{"summary"}, \code{"usage"},
  \code{"sections"}, \code{"settings"}, \code{"related"}
  and \code{"technical"}. If not set, their default values
  are used. See
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} for
  details.}

  \item{overwrite}{Logical, whether existing files should
  be replaced. Defaults to \code{FALSE}.}

  \item{tests}{Logical, whether directories and files for
  plugin tests should be created. Defaults to \code{TRUE}.}

  \item{lazyLoad}{Logical, whether the package should be
  prepared for lazy loading or not. Should be left
  \code{TRUE}, unless you have very good reasons not to.}

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

  \item{indent.by}{A character string defining the
  indentation string to use.}
}
\value{
  Character string with the path to the plugin root
  directory.
}
\description{
  Generate skeletons for RKWard plugins
}
\examples{
\dontrun{
# a simple example with only basic information
about.info <- rk.XML.about(
name="Square the circle",
author=c(
person(given="E.A.", family="Dölle",
email="doelle@eternalwondermaths.example.org", role="aut"),
person(given="A.", family="Assistant",
email="alterego@eternalwondermaths.example.org", role=c("cre","ctb"))
))

rk.plugin.skeleton("Square the Circle", about=about.info)

# a more complex example, already including some dialog elements
about.info <- rk.XML.about(
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
xml=list(dialog=test.tabbook), overwrite=TRUE)
}
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

