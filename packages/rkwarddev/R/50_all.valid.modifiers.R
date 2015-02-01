## list with valid modifiers
# used by -- at least -- modif.validity()
all.valid.modifiers <- list(
  all=c("", "visible", "visible.not", "visible.numeric", "enabled", "enabled.not", "enabled.numeric",
  "required", "true", "false", "not", "numeric", "preprocess", "calculate", "printout", "preview"),
  browser=c("selection"),
  checkbox=c("state", "state.not", "state.numeric"),
  dropdown=c("string", "string.quoted", "number"),
# removed embed, can be all sorts of stuff, see e.g. generic plot options
#  embed=c("code"),
# for the same reason external is not listed here
  frame=c("checked", "checked.not", "checked.numeric"),
  input=c("text"),
  formula=c("model", "table", "labels", "fixed_factors", "dependent"),
  matrix=c("rows", "columns", "tsv", "cbind"), # TODO: missing a solution for 1,2,3,... here
  option=c("enabled"),
  optionset=c("row_count", "current_row", "optioncolumn_ids"),
  preview=c("state", "state.not", "state.numeric"),
  radio=c("string", "string.quoted", "number"),
  saveobject=c("selection", "parent", "objectname", "active"),
  select=c("string", "string.quoted", "number"),
  spinbox=c("int", "real"),
  text=c("text"),
  valueselector=c("available", "selected", "root"),
  valueslot=c("available", "selected", "source", "shortname", "label"),
  varselector=c("selected", "root"),
  varslot=c("available", "selected", "source", "shortname", "label")
) ## end list with valid modifiers
