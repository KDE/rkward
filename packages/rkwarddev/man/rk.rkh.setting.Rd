\name{rk.rkh.setting}
\alias{rk.rkh.setting}
\title{Create XML "setting" node for RKWard help pages}
\usage{
  rk.rkh.setting(id, text = NULL, title = NULL)
}
\arguments{
  \item{id}{Either a character string (the \code{id} of the
  XML element to explain), or an object of class
  \code{XiMpLe.node} (whose \code{id} will be extracted and
  used).}

  \item{text}{Character string, the text to be displayed.}

  \item{title}{Character string, title to be displayed. If
  \code{NULL}, the \code{label} of the element will be
  shown.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  This function will create a setting node for settings
  sections in RKWard help files.
}
\examples{
# define a formula section with varselector and varslots
test.checkbox <- rk.XML.cbox(label="foo", value="foo1", chk=TRUE)
# explain the option
test.setting <- rk.rkh.setting(test.checkbox, text="Check this to do Foo.")
cat(pasteXML(test.setting))
}
\seealso{
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}},
  \code{\link[rkwarddev:rk.rkh.settings]{rk.rkh.settings}}
  and the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

