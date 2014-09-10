#!/usr/bin/env python2

import sys
import json

with open(sys.argv[1]) as transFile:
    trans = json.load(transFile)

translist = trans.values()
seznam = []
osmaw = []
nobody = []
both = []
for tr in translist:
    if tr["owtime"]==0 and tr["time"]==None:
        nobody.append(tr)
        continue
    if tr["owtime"]==0:
        seznam.append(tr)
        continue
    if tr["time"]==None:
        osmaw.append(tr)
        continue
    both.append(tr)
        

both = sorted(both, key=lambda it : it["owtime"]/it["time"] )

print "No one found ("+str(len(nobody))+"):"
for tr in nobody:
    print str(tr["owid"])+",",

print
print "Seznam only ("+str(len(seznam))+"):"
for tr in seznam:
    print str(tr["owid"])+",",

print 
print "Osmawalk only ("+str(len(osmaw))+"):"
for tr in osmaw:
    print str(tr["owid"])+",",

print
print "Both found ("+str(len(both))+"):"
for tr in both:
    print "{0[owid]}:\t{0[owlength]:.0f} m, {0[owtime]:.0f}\t vs.".format(tr),
    print "{0[length]} m, {0[time]:.0f} min ->\t {1:.2f}".format(tr,tr["owtime"]/tr["time"])

    

