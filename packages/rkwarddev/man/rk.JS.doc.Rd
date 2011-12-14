\name{rk.JS.doc}
\alias{rk.JS.doc}
\title{Create JavaScript outline from RKWard plugin XML}
\usage{
  rk.JS.doc(require = c(), variables = NULL,
    globals = NULL, results.header = NULL,
    preprocess = NULL, calculate = NULL, printout = NULL,
    doPrintout = NULL, load.silencer = NULL,
    gen.info = TRUE, indent.by = "\t")
}
\arguments{
  \item{require}{A character vector with names of R
  packages that the dialog depends on.}

  \item{variables}{Either a character string to be included
  to read in all needed variables from the dialog (see
  \code{\link{rk.JS.scan}}), or an object of class
  \code{rk.JS.var} which will be coerced into character.
  These variables will be defined in the \code{calculate()}
  and/or \code{doPrintout()} functions.}

  \item{globals}{Like \code{variables}, but these variables
  will be defined globally. If \code{variables} is set as
  well, the function tries to remove duplicate
  definitions.}

  \item{results.header}{A character string to headline the
  printed results. Include escapes quotes (\\") if needed.
  Set to \code{FALSE} or \code{""} if you need more control
  and want to define the header section in
  \code{printout}.}

  \item{preprocess}{A character string to be included in
  the \code{preprocess()} function. This string will be
  pasted as-is, after \code{require} has been evaluated.}

  \item{calculate}{A character string to be included in the
  \code{calculate()} function. This string will be pasted
  as-is, after \code{variables} has been evaluated.}

  \item{printout}{A character string to be included in the
  \code{printout()} function. This string will be pasted
  as-is, after \code{results.header} has been evaluated.
  Ignored if \code{doPrintout} is set.}

  \item{doPrintout}{A character string to be included in
  the \code{doPrintout()} function. This string will be
  pasted as-is. You don't need to define a \code{preview()}
  function, as this will be added automatically.  Use
  \code{ite("full", ...)} style JavaScript code to include
  headers etc.}

  \item{load.silencer}{Either a character string (ID of
  probably a checkbox), or an object of class
  \code{XiMpLe.node}.  This defines a switch you can add to
  your plugin, to set the \code{require()} call inside
  \code{suppressMessages()}, hence suppressing all load
  messages (except for warnings and errors) of required
  packages in the output.}

  \item{gen.info}{Logical, if \code{TRUE} a comment note
  will be written into the document, that it was generated
  by \code{rkwarddev} and changes should be done to the
  script.}

  \item{indent.by}{A character string defining how
  indentation should be done.}
}
\value{
  A character string.
}
\description{
  Create JavaScript outline from RKWard plugin XML
}
\note{
  The JavaScript
}
\seealso{
  \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
  \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
  \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
  \code{\link[rkwarddev:ite]{ite}},
  \code{\link[rkwarddev:echo]{echo}},
  \code{\link[rkwarddev:id]{id}}, and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

