\name{rk.XML.switch}
\alias{rk.XML.switch}
\title{Create XML node "switch" for RKWard plugins}
\usage{
  rk.XML.switch(condition, modifier = NULL, true = NULL,
    false = NULL, case = NULL, default = NULL,
    id.name = "auto")
}
\arguments{
  \item{condition}{Either a character string (the \code{id}
  of the property whose state should be queried), or an
  object of class \code{XiMpLe.node} (whose \code{id} will
  be extracted and used).}

  \item{modifier}{Character string, an optional modifier to
  be appended tp \code{condition}.}

  \item{true}{A named list, setting the return value if
  \code{condition} is TRUE.  Must have either
  \code{fixed_value} or \code{dynamic_value}.}

  \item{false}{A named list, setting the return value if
  \code{condition} is FALSE.  Must have either
  \code{fixed_value} or \code{dynamic_value}.}

  \item{case}{A list of named lists, setting the return
  values if \code{condition == case}.  Each vector
  represents one case; it must include a character string
  called \code{standard}, which is the value to match
  against, as well as either \code{fixed_value} or
  \code{dynamic_value}.}

  \item{default}{A named list, setting the default return
  value if \code{condition} doesn't match any case. Must
  have either \code{fixed_value} or \code{dynamic_value}.}

  \item{id.name}{Character string, a unique ID for this
  property.  If \code{"auto"}, IDs will be generated
  automatically from the condition ID.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  This node can only be used in \code{<logic>} sections. If
  the provided property is logical, you must also set
  values for \code{true} and \code{false}. If not, you must
  provide at least one \code{case}.
}
\details{
  The values to be returned can be either
  \code{fixed_value} or \code{dynamic_value}. A
  \code{fixed_value} must be a character string which will
  be returned if the condition is met. Whereas a
  \code{dynamic_value} is the \code{id} of another
  property, an can be provided as either a character string
  or an object of class \code{XiMpLe.node}.
}
\examples{
myCheckbox <- rk.XML.cbox("foo")
rk.XML.switch(myCheckbox,
  true=c(fixed_value="foo"),
  false=list(fixed_value="bar")
)
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

