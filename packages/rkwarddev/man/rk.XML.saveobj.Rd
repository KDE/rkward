\name{rk.XML.saveobj}
\alias{rk.XML.saveobj}
\title{Create XML node "saveobject" for RKWard plugins}
\usage{
  rk.XML.saveobj(label, chk = FALSE, checkable = TRUE,
  initial = "auto", required = FALSE, id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for this
  plugin element.}

  \item{chk}{Logical, if \code{TRUE} and
  \code{checkable=TRUE} the option is checkable and active
  by default.}

  \item{checkable}{Logical, if \code{TRUE} the option can
  be switched on and off.}

  \item{intitial}{Character string, the default name for
  the object should be saved to. If \code{"auto"} and a
  label was provided, an name will be generated
  automatically from the label.}

  \item{required}{Logical, whether an entry is mandatory or
  not.}

  \item{id.name}{Character string, a unique ID for this
  plugin element. If \code{"auto"} and a label was
  provided, an ID will be generated automatically from the
  label.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "saveobject" for RKWard plugins
}
\examples{
test.saveobj <- rk.XML.saveobj("Save the results")
cat(pasteXMLNode(test.saveobj))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

