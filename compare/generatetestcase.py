# generatcestaticbp.py
import sys
import datetime

# generate testcases for static bp
''' 
size = [8, 16,32,64]
for outputdatatype in size:
    for inputdatatype in size:
        for bitwidth_dec in range(inputdatatype):
            bitwidth = bitwidth_dec+1
            upper = (2**(bitwidth-1) - 1 + 2**(bitwidth-1))
            print "#  if CRITERION_STATBP(" + "% s" % outputdatatype + ", " + "% s" % inputdatatype + ", " + "% s" % bitwidth + ")"
	    s = ""
	    if (inputdatatype != outputdatatype):
		s =", "  + "uint" + "% s" % inputdatatype +"_t"
            print "    testcaseCorrectness < String < decltype(\"StaticBP\"_tstr) >, 0, " + format(upper, '#X') + ", sizeof(uint" + "% s" % outputdatatype + "_t) * 8 * countInLog, false, statbp <scalar<v" + "% s" % outputdatatype + "<uint" + "% s" % outputdatatype + "_t>>, " + "% s" % bitwidth + s + " > >::apply();"
            print "#  endif"
'''
# generate testcases for dynamic bp
'''
size = [8, 16,32,64]
for outputdatatype in size:
    for inputdatatype in size:
        for bitwidth_dec in range(inputdatatype):
            bitwidth = bitwidth_dec+1
            upper = (2**(bitwidth-1) - 1 + 2**(bitwidth-1))
            print "#  if CRITERION_DYNBP(" + "% s" % outputdatatype + ", " + "% s" % inputdatatype + ", " + "% s" % bitwidth + ")"
	    s = ""
	    if (inputdatatype != outputdatatype):
		s =", "  + "uint" + "% s" % inputdatatype +"_t"
            print "    testcaseCorrectness < String < decltype(\"DynamicBP\"_tstr) >, 0, " + format(upper, '#X') + ", sizeof(uint" + "% s" % outputdatatype + "_t) * 8 * countInLog, false, dynbp <scalar<v" + "% s" % outputdatatype + "<uint" + "% s" % outputdatatype + "_t>>, 1" + s + " > >::apply();"
            print "#  endif"
        print ""
'''
# generate testcases for static for static bp
size = [8, 16,32,64]
for outputdatatype in size:
    for inputdatatype in size:
        for bitwidth_dec in range(inputdatatype):
            bitwidth = bitwidth_dec+1
            upper = (2**(bitwidth-1) - 1 + 2**(bitwidth-1))
            print "#  if CRITERION_STATFORSTATBP(" + "% s" % outputdatatype + ", " + "% s" % inputdatatype + ", " + "% s" % bitwidth + ", " + "% s" % format(upper, '#X') + ")"
	    s = ""
	    if (inputdatatype != outputdatatype):
		s =", "  + "uint" + "% s" % inputdatatype +"_t"
            print "    testcaseCorrectness < String < decltype(\"Static FOR Static BP\"_tstr) >, REF_STATFORSTATBP, " + format(upper, '#X') + ", sizeof(uint" + "% s" % outputdatatype + "_t) * 8 * countInLog, false, statforstatbp <scalar<v" + "% s" % outputdatatype + "<uint" + "% s" % outputdatatype + "_t>>, REF_STATFORSTATBP, " + "% s" % bitwidth + s + " > >::apply();"
            print "#  endif"
        print ""
