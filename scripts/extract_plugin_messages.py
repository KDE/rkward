#! /usr/bin/python

import codecs
import sys
import os.path
import os
from xml.dom import minidom

# list of tag-names whose content to extract in full (including, possibly HTML-tags, within
text_containers = ['section', 'text', 'related', 'title', 'summary', 'usage', 'technical', 'setting']

infile = ""
outfile = ""
initialized_pot_files = []
current_po_id = ""
toplevel_sources = list (sys.argv[1:])

tag_stack = []
def backtrace ():
  ret = infile + ":"
  for tag in tag_stack:
    ret += " -> " + tag
  return (ret)

def getText (node):
  rc = []
  for cn in node.childNodes:
      if cn.nodeType == cn.TEXT_NODE:
        rc.append(cn.data)
  return ''.join (rc).strip ()

def getI18nComment (node):
  for cn in node.childNodes:
    if cn.nodeType == cn.COMMENT_NODE:
      comment = cn.data.strip ()
      if (comment.startswith ("I18N:") or comment.startswith ("TRANSLATORS:")):
        return "/*" + comment + "\n" + backtrace () + "*/\n"
  return "/*" + backtrace () + "*/\n"
 
def handleNode (node):
  if (node.nodeType == node.ELEMENT_NODE):
    tag_stack.append (node.tagName)
    if (node.hasAttribute ("label")):
      outfile.write (getI18nComment (node))
      if (node.hasAttribute ("i18n_context")):
        outfile.write ("i18nc (" + node.getAttribute ("i18n_context") + ", " + node.getAttribute ("label") + ")\n")
      outfile.write ("i18n (" + node.getAttribute ("label") + ")\n")
    if (node.hasAttribute ("file")):
      if (node.tagName != "code"):
        handleSubFile (node.getAttribute ("file"))
    if (node.tagName in text_containers):
      outfile.write (getI18nComment (node))
      outfile.write (getText (node))
    elif (getText (node) != ""):
      sys.stderr.write ("Found text content where none expected: " + backtrace () + "\n")
  for child in node.childNodes:
    handleNode (child)
  if (node.nodeType == node.ELEMENT_NODE):
    tag_stack.pop ()

def handleSubFile (filename):
  global toplevel_sources
  global infile
  cdir = os.path.dirname (infile)
  filename = os.path.join (cdir, filename)
  if (not os.path.isfile (filename)):
    sys.stderr.write (backtrace ()  + " WARNING: File " + filename + " does not exist\n")
    return
  xmldoc = minidom.parse (filename)
  if (xmldoc.documentElement.hasAttribute ("po_id") and (xmldoc.documentElement.getAttribute ("po_id") != current_po_id)):
    toplevel_sources.append (filename)
    sys.stderr.write ("Added " + filename + " to toplevel\n")
  else:
    sys.stderr.write ("Recursing into " + filename + "\n")
    oldinfile = infile
    infile = filename
    handleNode (xmldoc.documentElement)
    infile = oldinfile

def initialize_pot_file (po_id):
  global outfile
  current_po_id = po_id
  if (outfile != ""):
    outfile.close ()
  if (current_po_id in initialized_pot_files):
    mode = 'w+'
  else:
    initialized_pot_files.append (current_po_id)
    mode = 'w'
  outfile = codecs.open (po_id + '.pot.cpp', mode, 'utf-8')

# NOTE: toplevel_sources may grow, dynamically, but only at the end.
i = 0
print toplevel_sources
while i < len (toplevel_sources):
  infile = toplevel_sources[i]
  xmldoc = minidom.parse (infile)
  if (not xmldoc.documentElement.hasAttribute ("po_id")):
    sys.stderr.write ("No po_id attribute on file " + infile)
    continue
  initialize_pot_file (xmldoc.documentElement.getAttribute ("po_id"))
  handleNode (xmldoc.documentElement)
  i += 1

