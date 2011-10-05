\name{id}
\alias{id}
\title{Replace XiMpLe.node objects with their ID value}
\usage{
  id(..., quote = FALSE, collapse = "")
}
\arguments{
  \item{...}{One or several character strings and/or
  \code{XiMpLe.node} objects with plugin nodes, and/or
  objects of class \code{rk.JS.opt}, simply separated by
  comma.}

  \item{quote}{Logical, it the character strings sould be
  deparsed, so they come out "as-is" when written to files,
  e.g. by \code{cat}.}

  \item{collapse}{Character string, defining if and how the
  individual elements should be glued together.}
}
\value{
  A character string.
}
\description{
  This function is intended to be used for generating
  JavaScript code for RKWard plugins. Its sole purpose is
  to replace objects of class \code{XiMpLe.node} which hold
  an XML node of some plugin GUI definition, and objects of
  class \code{rk.JS.opt} with their \code{id/options}
  value, and combine these replacements with character
  strings.
}
\examples{
# an example checkbox XML node
cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
id("The ID is: ", cbox1, "!", quote=TRUE, collapse=" + ")
}
\seealso{
  \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
  \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
  \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
  \code{\link[rkwarddev:echo]{echo}}, and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

