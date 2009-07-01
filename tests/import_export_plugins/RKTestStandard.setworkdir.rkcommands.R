local({
## Prepare
## Compute
setwd("..")
## Print result
})
.rk.rerun.plugin.link(plugin="rkward::setworkdir", settings="dir.selection=..", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
setwd("import_export_plugins")
## Print result
})
.rk.rerun.plugin.link(plugin="rkward::setworkdir", settings="dir.selection=import_export_plugins", label="Run again")
.rk.make.hr()
