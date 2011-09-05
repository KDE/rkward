\name{rk.XML.vars}
\alias{rk.XML.vars}
\title{Create a variable selector for RKWard plugins}
\usage{
  rk.XML.vars(label, slot.text = NULL, classes = NULL,
  horiz = TRUE, id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for the whole
  frame.}

  \item{slot.text}{Character string, a text label for the
  variable selection slot.}

  \item{classes}{An optional character vector, defining
  class names to which the selection must be limited.}

  \item{horiz}{Logical. If \code{TRUE}, the varslot will be
  placed next to the selector, if \code{FALSE} below it.}

  \item{id.name}{Character vector, unique IDs for the frame
  (first entry), the varselector (second entry) and varslot
  (third entry). If \code{"auto"}, IDs will be generated
  automatically from \code{label} and \code{slot.text}.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  This function will create a <frame> node including a
  <varselector> and a <varslot> node.
}

