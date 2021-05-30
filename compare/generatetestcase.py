# generatcestaticbp.py
import sys
import datetime

bitwidth = 0
#if __name__ == "__main__":
#    bitwidth = int(sys.argv[1])
#    print bitwidth

size = [8, 16,32,64]
for outputdatatype in size:
    for inputdatatype in size:
        for bitwidth_dec in range(inputdatatype):
            bitwidth = bitwidth_dec+1
            upper = (2**(bitwidth-1) - 1 + 2**(bitwidth-1))
            print "  #if CRITERION(" + "% s" % outputdatatype + ", " + "% s" % inputdatatype + ", " + "% s" % bitwidth + ")"
	    s = ""
	    if (inputdatatype != outputdatatype):
		s =", "  + "uint" + "% s" % inputdatatype +"_t"
            print "    testcaseCorrectness < String < decltype(\"StaticBP\"_tstr) >, 0, " + format(upper, '#X') + ", sizeof(uint" + "% s" % outputdatatype + "_t) * 8 * countInLog, false, statbp <scalar<v" + "% s" % outputdatatype + "<uint" + "% s" % outputdatatype + "_t>>, " + "% s" % bitwidth + s + " > >::apply();"
            print "  #endif"

