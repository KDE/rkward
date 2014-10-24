#! /usr/bin/python

BUGADDR = "http://p.sf.net/rkward/bugs"

import codecs
import sys
import os.path
import os
from xml.dom import minidom
import copy

def usage ():
  print ("Usage: " + sys.argv[0] + " [--default_po=PO_ID] [--outdir=DIR] files")
  exit (1)

# list of tag-names the content of which to extract in full (including, possibly, HTML-tags, within)
text_containers = ['section', 'text', 'related', 'title', 'summary', 'usage', 'technical', 'setting']

# initialize globals, and parse args
infile = {"infile": "", "file_prefix": "", "caption": ""}
default_po = ""
outfile = ""
outdir = ""
initialized_pot_files = []
current_po_id = ""
toplevel_sources = []
for arg in (list (sys.argv[1:])):
  if (arg.startswith ("--default_po=")):
    default_po = arg.split ("=", 1)[1]
  elif (arg.startswith ("--outdir=")):
    outdir = arg.split ("=", 1)[1]
  elif (arg.startswith ("--")):
    usage ()
  else:
    toplevel_sources.append (arg)
if (len (toplevel_sources) < 1):
  usage ()

tag_stack = []
def backtrace ():
  ret = infile["infile"]
  if (infile["caption"] != ""):
    ret += " (" + infile["caption"] + ")"
  ret += ":"
  for tag in tag_stack:
    ret += " -> " + tag
  return (ret)

def quote (text):
  return "\"" + text.replace ("\\", "\\\\").replace ("\"", "\\\"") + "\""

# Normalizes larger text fragments. TODO: Do we want to protect <pre>-blocks?
def normalize (text):
  lines = text.split ("\n")
  nlines = []
  for line in lines:
    nlines.append (' '.join (line.strip ().split ()))	# remove whitespace at start, end, and simplify whitespace within each line
  return ' '.join (nlines)
  
# get everything inside the element as text. Might include further xml tags.
def getFullText (element):
  rc = []
  for cn in element.childNodes:
    if cn.nodeType != cn.COMMENT_NODE:
      rc.append(cn.toxml ("utf-8"))
  return ''.join (rc).strip ()

# get the content of all text nodes inside this node (does not include xml tags)
def getText (node):
  rc = []
  for cn in node.childNodes:
    if cn.nodeType == cn.TEXT_NODE:
      rc.append(cn.data)
  return ''.join (rc).strip ()

# Look for an i18n comment in the given node, and write it out to the outfile
def getI18nComment (node):
  for cn in node.childNodes:
    if cn.nodeType == cn.COMMENT_NODE:
      comment = cn.data.strip ()
      if (comment.startswith ("I18N:") or comment.startswith ("TRANSLATORS:")):
        return "/*i18n: " + comment + "\nOrigin was " + backtrace () + " */\n"
  return "/*i18n: Origin was " + backtrace () + " */\n"

# Main workhorse: Look at given node and recurse into children
def handleNode (node):
  if (node.nodeType == node.ELEMENT_NODE):
    tag_stack.append (node.tagName)
    if (node.hasAttribute ("label")):
      outfile.write (getI18nComment (node))
      if (node.hasAttribute ("i18n_context")):
        outfile.write ("i18nc (" + quote (node.getAttribute ("i18n_context")) + ", " + quote (node.getAttribute ("label")) + ");\n")
      outfile.write ("i18n (" + quote (node.getAttribute ("label")) + ");\n")
    if (node.hasAttribute ("file")):
      if (node.tagName != "code"):
        # TODO: handle .js files
        handleSubFile (node.getAttribute ("file"))
    if (node.tagName in text_containers):
      textchunks = getFullText (node).split ("\n\n")
      for chunk in textchunks:
        outfile.write (getI18nComment (node))
        outfile.write ("i18n (" + quote (normalize (chunk)) + ");\n")
    elif (getText (node) != ""):
      sys.stderr.write ("Found text content where none expected: " + backtrace () + "\n")
  if (not ((node.nodeType == node.ELEMENT_NODE) and (node.tagName in text_containers))):
    # Don't go looking into the contents of text containers any further (may contain HTML markup)
    for child in node.childNodes:
      handleNode (child)
  if (node.nodeType == node.ELEMENT_NODE):
    tag_stack.pop ()

# Try to determine a caption for the file (will be used as context comment)
def getFileCaption (docelem):
  elems = docelem.getElementsByTagName ("title")
  if (elems.length):
    return normalize (getFullText (elems.item (0)))
  elems = docelem.getElementsByTagName ("dialog")
  if (elems.length):
    return elems.item (0).getAttribute ("label")
  elems = docelem.getElementsByTagName ("wizard")
  if (elems.length):
    return elems.item (0).getAttribute ("label")
  return ""
    
# When we encounter a "file"-attribute, we generally dive right into parsing that file, i.e. we do depth first
# Advantage is that strings in all files belonging to one plugin will be in direct succession in the .pot file
# The exception is if the referenced file declares an own (different) po_id. In this case it will be handled, later.
def handleSubFile (filename):
  global toplevel_sources
  global infile
  cdir = os.path.dirname (infile["infile"])
  filename = os.path.join (cdir, infile["file_prefix"], filename)
  if (not os.path.isfile (filename)):
    sys.stderr.write (backtrace ()  + " WARNING: File " + filename + " does not exist\n")
    return
  xmldoc = minidom.parse (filename)
  if (xmldoc.documentElement.hasAttribute ("po_id") and (xmldoc.documentElement.getAttribute ("po_id") != current_po_id)):
    toplevel_sources.append (filename)
    sys.stderr.write ("Added " + filename + " to toplevel\n")
  else:
    sys.stderr.write ("Recursing into " + filename + "\n")
    oldinfile = copy.deepcopy (infile)
    infile["infile"] = filename
    infile["file_prefix"] = xmldoc.documentElement.getAttribute ("base_prefix")
    infile["caption"] = getFileCaption (xmldoc.documentElement)
    if ((infile["caption"] == "") and (oldinfile["caption"] != "")):
      infile["caption"] = "Loaded from " + oldinfile["caption"]
    handleNode (xmldoc.documentElement)
    infile = oldinfile

def initialize_pot_file (po_id):
  global outfile
  global current_po_id
  current_po_id = po_id
  if (outfile != ""):
    outfile.close ()
  if (current_po_id in initialized_pot_files):
    mode = 'w+'
  else:
    initialized_pot_files.append (current_po_id)
    mode = 'w'
  outfile = codecs.open (os.path.join (outdir, po_id + '.pot.cpp'), mode, 'utf-8')

#######
# Loop over toplevel_sources (specified on command line, or those that want to be split into separate po) and extract messages
# NOTE: toplevel_sources may grow, dynamically, but only at the end.
i = 0
while i < len (toplevel_sources):
  xmldoc = minidom.parse (toplevel_sources[i])
  po_id = xmldoc.documentElement.getAttribute ("po_id")
  if (po_id == ""):
    po_id = default_po
  if (po_id == ""):
    sys.stderr.write ("No po_id attribute on file " + toplevel_sources[i] + " and no default specified\n")
    continue
  initialize_pot_file (po_id)
  handleSubFile (toplevel_sources[i])  # Some duplication of parsing, instead of duplication of code
  i += 1

#######
# Run xgettext on all generated .pot.cpp files
for potcpp in initialized_pot_files:
  potcppfile = os.path.join (outdir, potcpp + ".pot.cpp")
  os.system ("xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 " + 
             "-kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 " +
             "--msgid-bugs-address=" + BUGADDR + " -o " + os.path.join (outdir, "rkward__" + potcpp + ".pot ") + potcppfile)
  os.remove (potcppfile)
