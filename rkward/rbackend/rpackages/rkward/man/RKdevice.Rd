\name{RK}
\alias{RK}
\title{Plot graphics to RKWard native device}
\usage{
  RK(width = getOption("rk.screendevice.width"),
    height = getOption("rk.screendevice.height"),
    pointsize = 12, family = NULL, bg = "white",
    title = "", antialias = TRUE)
}
\arguments{
  \item{width}{Width of the device in inches. The default
  is to use the heigth configured in Settings -> Configure
  RKWard -> Onscreen Graphics.}

  \item{height}{Height of the device in inchesgraphics in
  pixels. The default is to use the heigth configured in
  Settings -> Configure RKWard -> Onscreen Graphics.}

  \item{pointsize}{Default pointsize}

  \item{family}{Default font family. This can be a
  character vector of length 1 or 2. The second value is
  used for plotting symbols. Effectively the default is
  c("Helvetica", "Symbol"). A wide variety of sepcification
  is supported, including the device independent fonts
  names "sans", "serif", and "mono"}

  \item{bg}{Background color.}

  \item{title}{Window title.}

  \item{antialias}{Antialiasing. Can be turned off for
  somewhat faster drawing.}
}
\description{
  \code{RK()} creates an R on-screen device that will be
  rendered in the RKWard frontend. The default settings for
  \code{width}, and \code{height} can be modified from
  Settings -> Configure RKWard -> Onscreen Graphics.
}
\keyword{devices}

