\name{rk.XML.optionset}
\alias{rk.XML.optionset}
\title{Create XML node "optionset" for RKWard plugins}
\usage{
  rk.XML.optionset(content, optioncolumn, min_rows = 0,
    min_rows_if_any = 0, max_rows = 0, keycolumn = NULL,
    logic = NULL, optiondisplay = TRUE, id.name = "auto")
}
\arguments{
  \item{content}{A list of XiMpLe.nodes to be placed inside
  the \code{<content>} node of this \code{<optionset>}.}

  \item{optioncolumn}{A list of \code{<optioncolumn>}
  XiMpLe.nodes.}

  \item{min_rows}{Numeric (integer), if specified, the set
  will be marked invalid, unless it has at least this
  number of rows. Ignored if set to 0.}

  \item{min_rows_if_any}{Numeric (integer), like min_rows,
  but will only be tested, if there is at least one row.
  Ignored if set to 0.}

  \item{max_rows}{Numeric (integer), if specified, the set
  will be marked invalid, unless it has at most this number
  of rows. Ignored if set to 0.}

  \item{keycolumn}{Character}

  \item{logic}{A valid \code{<logic>} node.}

  \item{optiondisplay}{Logical value, can be used to
  automatically add an \code{<optiondisplay>} node on top
  of the \code{<content>} section. Depending on whether
  it's \code{TRUE} or \code{FALSE}, its \code{index}
  argument will be set to \code{"true"} or \code{"false"},
  respectively. Set to \code{NULL} to deactivate.}

  \item{id.name}{Character string, a unique ID for this
  plugin element.  If \code{"auto"}, an ID will be
  generated automatically from the <content> nodes.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Note that if you want to refer to the optioncolumns in
  your JavaScript code, the \code{id} you need is a
  combination of \code{<optionset id>.<optioncolumn
  id>.<modifier>}. that is, you must always prefix it with
  the sets' \code{id}. For JavaScript code generating with
  \code{rkwarddev}, you can use the ID that functions like
  \code{\link[rkwarddev:id]{id}} return, because the
  JavaScript variable name will only contain a constant
  prefix ("ocol") an the column ID.
}
\note{
  The \code{<optionset>} node was introduced with RKWard
  0.6.1, please set the dependencies of your
  component/plugin accordingly.
}
\examples{
firstname <- rk.XML.input("Given name(s)")
lastname <- rk.XML.input("Family name")
genderselect <- rk.XML.radio("Gender", options=list(
  Male = c(val="m"),
  Female = c(val="f")))
(myOptionset <- rk.XML.optionset(
  content = list(
    rk.XML.row(
      firstname,
      lastname,
      genderselect)),
  optioncolumn = list(
    rk.XML.optioncolumn(firstname, modifier="text"),
    rk.XML.optioncolumn(lastname, modifier="text"),
    rk.XML.optioncolumn(genderselect)
  )
))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.optioncolumn]{rk.XML.optioncolumn}},
  \code{\link[rkwarddev:rk.XML.optiondisplay]{rk.XML.optiondisplay}},
  and the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

