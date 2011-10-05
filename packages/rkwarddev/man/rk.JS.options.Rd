\name{rk.JS.options}
\alias{rk.JS.options}
\title{Combine several options in one JavaScript variable}
\usage{
  rk.JS.options(opt.name, ...)
}
\arguments{
  \item{opt.name}{Character string, name of the JavaScript
  variable to use in the script code.}

  \item{...}{A list of objects of class \code{rk.JS.ite}
  (see \code{\link[rkwarddev:ite]{ite}}). Use the
  \code{thenjs} element only to define the value to add to
  the option, without commas or similar (e.g.,
  \code{"paired=TRUE"} or \code{id("conf.level=",
  conflevel)}.}
}
\value{
  An object of class \code{rk.JS.opt}, use
  \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}} on that.
}
\description{
  Combine several options in one JavaScript variable
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

