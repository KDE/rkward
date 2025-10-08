# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Meik Michalke <rkward-devel@kde.org>
# SPDX-FileContributor: Thomas Friedrichsmeier <rkward-devel@kde.org>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

## Instructions / How to update the files.kde.org/rkward/R repo
# 1. Be sure to edit the main.root.packages var according to your setup
# 2. You will need a *manual* clone of all packages that you want to update in file.path(main.root.packages, "sources")
#    You will need to manually checkout the appropriate branch/tag (and of course you should do at least some very
#    basic checking) NOTE: roxy.package() could automate this, but for now it's a deliberate decision to require humand
#    attention on package updates.
# 3. Enable the packages to update in the for loop, below
# 4. Source/run the whole script. You may/will have to install various additional packages at several points
# 5. Upload to sftp://files.kde.org, internal path rkward/web/R

# This script is based on roxyPackage by Meik Michalke, which can do a lot more package management tasks that just this.

## setup environment
# .libPaths("~/R")
require(roxyPackage)
require(tools)

local({
  main.root.packages <- file.path("/home","thomas","develop","rpackages")
  repo.root <- file.path(main.root.packages,"repo")

  # Sync existing repository. Must work with a specific mirror (rather than files.kde.org), for wget to work well
  # Mirror needs to support ftp, but specify without protocol, here
  repo_mirror <- "ftp.gwdg.de/pub/linux/kde/extrafiles/rkward/R/"
  dir.create(repo.root)
  system(paste0("cd ", repo.root, "; wget --mirror ftp://", repo_mirror, " -nH --cut-dirs=", length(strsplit(repo_mirror, "/")[[1]])-1))
  repo.copy <- file.path(tempdir(),"repo_rkward_copy")
  unlink(repo.copy, recursive=TRUE)
  dir.create(repo.copy)
  file.copy(list.files(repo.root, full.names=TRUE), repo.copy, recursive=TRUE, copy.date=TRUE)

    actions <- c(
        "html",           # update HTML index files
        "package"#,         # build & install plugin package
    )

  for (this.plugin in c(
     # https://github.com/rkward-community
     "rk.ANOVA",
     "rk.ClusterAnalysis",
     "rk.CohenKappa",
     "rk.downloadAppImage",
     "rk.FactorAnalysis",
     "rk.gitInstall",
     "rk.MultidimensionalScaling",
     "rk.Teaching",
     "rkwarddev",
#     "rk.MPT", # documented as broken

     # https://github.com/AlfCano
     "rk.cSplit",
     "rk.dplyr",
     "rk.forcats",
     "rk.gsub.sub",
     "rk.gtsummary",
     "rk.pivot.reshape",
     "rk.survey.design",
     "rk.transpose.df",
     NULL
  )){
    pck.name <- this.plugin
    main.root <- file.path(main.root.packages,"sources")
    r.dir <- file.path(main.root, pck.name)

    roxy.package(actions=actions,
      pck.description=NULL,
      pck.source.dir=r.dir,
      pck.version=NULL,
      R.libs.append=rk.home("lib"),
      R.libs=.libPaths()[1],
      repo.root=repo.root,
      cleanup=TRUE,
      URL=c(
        default="https://files.kde.org/rkward/R"
      ),
      html.options=list(
        index="Available RKWard Plugin Packages",
        title="RKWard Plugin Package"
      ),
      Rbuildignore=c(".git")
    )
  }

  old_files <- list.files(repo.copy, recursive=TRUE)
  new_files <- list.files(repo.root, recursive=TRUE)
  new_files <- new_files[sapply(new_files, function(file) {
    if (!file.exists(file.path(repo.copy, file))) return(TRUE)  # all new
    if (md5sum(file.path(repo.copy, file)) != md5sum(file.path(repo.root, file))) return(TRUE)  # changed
    return(FALSE)
  })]
  if (length(new_files)) {
    cat("\nThe following files have been updated. Please remember to sync them to the repository!\n")
    cat(paste(new_files, collapse="\n"))
    cat("\n")
  }
})
