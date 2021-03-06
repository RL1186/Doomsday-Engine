"""Generates a console help file out of Amethyst sources."""

import os
import sys
import string
import subprocess


def amethyst(input):
    """Runs amethyst with the given input and returns the output."""
    p = subprocess.Popen(['amethyst', '-dHELP'], 
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (output, errs) = p.communicate(input)
    #print >>sys.stderr, errs
    return output 
       

def makeHelp(outName, components):
    """Generates the output file 'outName' out of the Amethyst
    sources in the subdirectories listed in 'components'.
    - outName: File path of the output file.
    - components: List of subdirectories to include in the output."""
    
    modes = ['command', 'variable']

    outFile = file(outName, 'w')
    print >> outFile, "# Doomsday Help"
    print >> outFile, "# Generated by conhelp.py out of", string.join(components, ', ')

    for mode in modes:
        for com in components:
            print >> outFile, "#\n# CONSOLE", mode.upper() + 'S: %s\n#' % com
            path = os.path.join(com, mode)
            for fn in os.listdir(path):
                if not fn.endswith('.ame'): continue
                name = fn[:-4]
                print '- %s %s: %s' % (com, mode, name)
                print >> outFile, "\n[%s]" % name
            
                templ = '@require{amestd} @begin\n' + \
                        file(os.path.join(path, fn)).read()
            
                print >> outFile, amethyst(templ).strip()

                
if __name__ == '__main__':
    if len(sys.argv) < 3:
        print "Usage: (outname) (inputdirs) ..."
        sys.exit(0)
    makeHelp(sys.argv[1], sys.argv[2:])
