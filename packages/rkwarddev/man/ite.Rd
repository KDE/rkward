\name{ite}
\alias{ite}
\title{Generate JavaScript if/then/else constructs}
\usage{
  ite(ifjs, thenjs, elsejs = NULL)
}
\arguments{
  \item{ifjs}{A character string, to be placed in the
  brackets if an \code{if()} statement.}

  \item{thenjs}{A character string, the code to be executed
  in case the \code{if()} statement is true.}

  \item{elsejs}{A character string, the code to be executed
  in case the \code{if()} statement is not true.}
}
\value{
  An object of class \code{rk.JS.ite}
}
\description{
  Generate JavaScript if/then/else constructs
}
\examples{
# first create an example checkbox XML node
cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
# now some JavaScript generation
ite(cbox1, echo("bar <- \\"", cbox1, "\\""), echo("bar <- NULL"))
}
\seealso{
  \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
  \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
  \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
  \code{\link[rkwarddev:echo]{echo}},
  \code{\link[rkwarddev:id]{id}},
  \code{\link[rkwarddev:qp]{qp}}, and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

