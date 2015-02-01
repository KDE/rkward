## list with valid child nodes
# important for certain parent nodes, as long as
# XiMpLe doesn't interpret doctypes
# used by -- at least -- valid.child()
all.valid.children <- list(
  # 'as' is not a node, but an attribute of <copy>
  as=c("browser", "checkbox", "column", "copy",
    "dropdown", "formula", "frame", "input", "page", "radio", "row", "saveobject",
    "spinbox", "stretch", "tabbook", "text", "valueselector", "valueslot", "varselector", "varslot", "!--"),
  component=c("dependencies", "!--"),
  components=c("component", "!--"),
  context=c("menu", "!--"),
  dialog=c("browser", "checkbox", "column", "copy",
    "dropdown", "embed", "formula", "frame", "include", "input", "insert", "matrix",
    "optionset", "preview", "radio", "row", "saveobject", "spinbox", "stretch", "tabbook",
    "text", "valueselector", "valueslot", "varselector", "varslot", "!--"),
  dropdown=c("option", "!--"),
  hierarchy=c("menu", "!--"),
  logic=c("connect", "convert", "dependency_check", "external", "i18n", "include", "insert",
    "script", "set", "switch", "!--"),
  menu=c("entry", "menu", "!--"),
  optionset=c("content", "logic", "optioncolumn", "!--"),
  page=c("browser", "checkbox", "column", "copy",
    "dropdown", "formula", "frame", "input", "matrix", "optionset", "page", "radio",
    "row", "saveobject", "spinbox", "stretch", "tabbook", "text", "valueselector",
    "valueslot", "varselector", "varslot", "!--"),
  radio=c("option", "!--"),
  select=c("option", "!--"),
  settings=c("setting", "caption", "!--"),
  snippets=c("include", "snippet", "!--"),
  valueselector=c("option", "!--"),
  wizard=c("browser", "checkbox", "column", "copy",
    "dropdown", "embed", "formula", "frame", "include", "input", "insert", "matrix",
    "optionset", "page", "preview", "radio", "row", "saveobject", "spinbox", "stretch",
    "tabbook", "text", "valueselector", "valueslot", "varselector", "varslot", "!--")
) ## end list with valid child nodes
