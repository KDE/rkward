\name{rk.plugin.component}
\alias{rk.plugin.component}
\title{Generate RKWard plugin components}
\usage{
  rk.plugin.component(about, xml = list(), js = list(),
    rkh = list(), provides = c("logic", "dialog"),
    scan = c("var", "saveobj", "settings"),
    guess.getter = FALSE, hierarchy = "test",
    include = NULL, create = c("xml", "js", "rkh"),
    hints = TRUE, gen.info = TRUE, indent.by = "\t")
}
\arguments{
  \item{about}{Either a character string with the name of
  this plugin component, or an object of class
  \code{XiMpLe.node} with further descriptive information
  on it, like its authors and dependencies (see
  \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for
  details). This is only useful for information that
  differs from the \code{<about>} section of the
  \code{.pluginmap} file.}

  \item{xml}{A named list of options to be forwarded to
  \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}}, to
  generate the GUI XML file.  Not all options are supported
  because some don't make sense in this context. Valid
  options are: \code{"dialog"}, \code{"wizard"},
  \code{"logic"} and \code{"snippets"}.  If not set, their
  default values are used. See
  \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}} for
  details.}

  \item{js}{A named list of options to be forwarded to
  \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}, to generate
  the JavaScript file.  Not all options are supported
  because some don't make sense in this context. Valid
  options are: \code{"require"}, \code{"results.header"},
  \code{"variables"}, \code{"globals"},
  \code{"preprocess"}, \code{"calculate"},
  \code{"printout"}, \code{"doPrintout"} and
  \code{"load.silencer"}.  If not set, their default values
  are used. See
  \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}} for
  details.}

  \item{rkh}{A named list of options to be forwarded to
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}, to
  generate the help file.  Not all options are supported
  because some don't make sense in this context. Valid
  options are: \code{"summary"}, \code{"usage"},
  \code{"sections"}, \code{"settings"}, \code{"related"}
  and \code{"technical"}.  If not set, their default values
  are used. See
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} for
  details.}

  \item{provides}{Character vector with possible entries of
  \code{"logic"}, \code{"dialog"} or \code{"wizard"},
  defining what sections the GUI XML file should provide
  even if \code{dialog}, \code{wizard} and \code{logic} are
  \code{NULL}.  These sections must be edited manually and
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

  \item{hints}{Logical, if \code{TRUE} and you leave
  optional entries empty (like \code{rkh=list()}), dummy
  sections will be added.}

  \item{guess.getter}{Logical, if \code{TRUE} try to get a
  good default getter function for JavaScript variable
  values (if \code{scan} is active). This will use some
  functions which were added with RKWard 0.6.1, and
  therefore raise the dependencies for your
  plugin/component accordingly. Nonetheless, it's
  recommended.}

  \item{hierarchy}{A character vector with instructions
  where to place this component in the menu hierarchy, one
  list or string.  Valid single values are \code{"file"},
  \code{"edit"}, \code{"view"}, \code{"workspace"},
  \code{"run"}, \code{"data"}, \code{"analysis"},
  \code{"plots"}, \code{"distributions"}, \code{"windows"},
  \code{"settings"} and \code{"help"}, anything else will
  place it in a "test" menu. If \code{hierarchy} is a list,
  each entry represents the label of a menu level.}

  \item{include}{Character string or vector, relative
  path(s) to other file(s), which will then be included in
  the head of the GUI XML document.}

  \item{create}{A character vector with one or more of
  these possible entries: \describe{
  \item{\code{"xml"}}{Create the plugin \code{.xml} XML
  file skeleton.} \item{\code{"js"}}{Create the plugin
  \code{.js} JavaScript file skeleton.}
  \item{\code{"rkh"}}{Create the plugin \code{.rkh} help
  file skeleton.} }}

  \item{gen.info}{Logical, if \code{TRUE} comment notes
  will be written into the genrated documents, that they
  were generated by \code{rkwarddev} and changes should be
  done to the script.}

  \item{indent.by}{A character string defining the
  indentation string to use.}
}
\value{
  An object of class \code{rk.plug.comp}.
}
\description{
  Generate RKWard plugin components
}
\examples{
\dontrun{
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

rk.plugin.component("Square the Circle",
  xml=list(dialog=test.tabbook))
}
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

