\name{rk.JS.options}
\alias{rk.JS.options}
\title{Combine several options in one JavaScript variable}
\usage{
  rk.JS.options(var, ...)
}
\arguments{
  \item{var}{Character string, name of the JavaScript
  variable to use in the script code.}

  \item{...}{A list of objects of class \code{rk.JS.ite}
  (see \code{\link[rkwarddev:ite]{ite}}). Use the
  \code{thenjs} element only to define the value to add to
  the option (e.g., \code{", paired=TRUE"} or \code{id(",
  conf.level=\"", conflevel, "\"", quote=TRUE, collapse= "
  + ")}.}
}
\value{
  An object of class \code{rk.JS.opt}, use
  \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}} on that.
}
\description{
  Combine several options in one JavaScript variable
}
\examples{
# create two checkboxes for independent options
checkA <- rk.XML.cbox(label="Run Test A", value="A")
checkB <- rk.XML.cbox(label="Run it fast", value="true")
# combine them into one JavaScript options variable
rk.JS.options("optionsTestA",
ite(checkA, id("test=\\"", checkA, "\\"", quote=TRUE, collapse= " + ")),
ite(checkB, "fast=TRUE")
)
}
\seealso{
  \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}}, and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

