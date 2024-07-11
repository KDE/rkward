## README
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# this R script can be used to let roxyPackage maintain the rkward package sources
# to use it
#  1. install the package "roxyPackage"
#  2. make a local copy of this file
#  3. checkout the lates rkward package sources via git
#  4. cusomize the first part of the script (see below, you MUST set these paths correctly!)
#  5. run the script

## setup environment
require(roxyPackage)

local({
  #######################
  ## these are the values you probably need to update
  #######################
  package.version <- "0.8.0"

  # this should point to the "rkward" root directory of the R package
  package.sources <- file.path("/PathTo","git","rkward","rkward","rbackend","rpackages","rkward")
  # packages are copied to a "local repository", that is simply a folder
  # to create some files and directories. it can actually be used as a
  # R repository
  local.repository <- tempdir() # file.path("/PathTo", "repo_rkward")
  # path to R libs -- where should the package be installed to?
  install.path <- .libPaths()[1]
  # use sandbox mode -- see ?sandbox
  sandbox(TRUE)

  # what should be done? (un)comment actions as needed
  roxyPackage.actions <- c(
  ## documentation:
    # "roxy" is needed also to update NAMESPACE; but beware that
    # it will overwrite the docs, so be careful with svn commits
    # until the docs are fully tagged!
        "roxy",           # roxygenize the docs
  #       "cite",           # update CITATION file
  #       "doc",            # update pdf documentation
  #       "cl2news",        # convert ChangeLog into NEWS.Rd
  #       "cleanRd",        # linebreaks for >90 chars in *.Rd
  #       "log",            # update ChangeLog
  #       "buildVignettes", # re-build vignettes with index
  ## local repository:
  #       "news2rss",       # convert NEWS.Rd into RSS feed
  #       "html",           # update index.html
  #       "win",            # update the windows binary package
  #       "macosx",         # update the mac OS X binary package
  ## build:
        "package"         # build & install the package
  #     "buildVignettes", "package", "check" # check package
  ## initialize files
  #        "readme"         # add initial README.md file
  #        "license"        # update LICENSE file
  #        "vignette"       # add Rmd vignette stub
  )

  #######################
  ## from here on all should be left untouched
  #######################
  # package description
  package.description <- package_description(
    Package="rkward",
    Type="Package",
    Title="Provides functions related to the RKWard GUI",
    Author="Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net> and the RKWard Team <rkward-devel@kde.org>",
    AuthorsR="c(person(given=\"Thomas\", family=\"Friedrichsmeier\", email=\"thomas.friedrichsmeier@kdemail.net\",
        role=c(\"aut\")),
      person(given=\"the RKWard\", family=\"team\", email=\"rkward-devel@kde.org\",
        role=c(\"cre\",\"aut\")))",
    Maintainer="RKWard-devel mailing list <rkward-devel@kde.org>",
    ## TODO: check dependencies
    Depends="R (>= 2.9.0),methods",
    Suggests="Cairo,googleVis,htmlwidgets,lattice,R2HTML",
    #VignetteBuilder="knitr",
    Description="This package contains functions which are useful in combination with the RKWard GUI. Many of these
      functions are only needed for the internal communication between RKWard and R, but some are also useful in user scripts.",
    License="GPL (>= 2)",
    Encoding="UTF-8",
    LazyLoad="yes",
    URL="https://rkward.kde.org",
    BugReports="https://rkward.kde.org/Bugs.html"
  )

  ## here we go
  roxy.package(actions=roxyPackage.actions,
    pck.source.dir=package.sources,
    pck.version=package.version,
    pck.description=package.description,
    R.libs=install.path,
    repo.root=local.repository,
    cleanup=TRUE,
    URL="https://rkward.kde.org",
    #readme.options=list(
    #  githubUser=""
    #),
    html.options=list(
      flattr.id="myxw65",
      repo.flattr.id="myxw65"#,
      #imprint="https://rkward.kde.org/...",
      #privacy.policy="https://rkward.kde.org/..."
    )
  )
  # add additional note to package file
  package_note <- paste0(
    "# - This file is part of the RKWard project (https://rkward.kde.org).\n",
    "# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>\n",
    "# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>\n",
    "# SPDX-License-Identifier: GPL-2.0-or-later"
  )
  sandbox_path <- sandbox.status()
  if(identical(sandbox_path, "")){
    package_file <- file.path(package.sources, "R", "rkward-package.R")
  } else {
    package_file <- file.path(sandbox_path, "src", "rkward", "R", "rkward-package.R")
  }
  package_file_orig <- readLines(package_file)
  cat(package_note, package_file_orig, file=package_file, sep = "\n")
})
