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

