# generatcestaticbp.py
import sys
import datetime

bitwidth = 0
#if __name__ == "__main__":
#    bitwidth = int(sys.argv[1])
#    print bitwidth

d = datetime.datetime.now()
size = [8, 16,32,64]
for inputdatatype in size:
    for outputdatatype in size:
        for bitwidth_dec in range(inputdatatype):
	    print "testcase_correctness<String<decltype(\"StaticBP\"_tstr)>, uint" + "% s" % inputdatatype + "_t, uint" + "% s" % outputdatatype +"_t, 0, " + "% s" % (2**(bitwidth-1) - 1 + 2**(bitwidth-1)) + "U, sizeof(uint" + "% s" % outputdatatype + "_t)*8*4, false, statbp<uint" + "% s" % inputdatatype + "_t, "+ "% s" % bitwidth + ", uint" + "% s" % outputdatatype +"_t>>::apply();"
	    f = open("staticbp_"+ "% s" % inputdatatype+ "_" + "% s" % outputdatatype + "_" + "% s" % bitwidth + ".h", "w")
            f.write("/*\n" \
            " * To change this license header, choose License Headers in Project Properties.\n" \
            " * To change this template file, choose Tools | Templates\n" \
            " * and open the template in the editor.\n" \
            " */\n"\
            "\n"\
            "/* \n" \
            " * File:   staticbp_"+ "% s" % inputdatatype+ "_" + "% s" % outputdatatype + "_" + "% s" % bitwidth + ".h\n" \
            " * Author: jule\n" \
            " *\n" \
            " * Created on " + "% s" % d.strftime("%d") + ". " + d.strftime("%B") + " " + "% s" % d.year + "," + "% s" % d.strftime("%H")+ ":" + "% s" % d.strftime("%M") + "\n" \
            " */\n" \
            "\n"\
            "#ifndef STATICBP_"+ "% s" % inputdatatype+ "_" + "% s" % outputdatatype + "_" + "% s" % bitwidth + "_H\n" \
            "#define STATICBP_"+ "% s" % inputdatatype+ "_" + "% s" % outputdatatype + "_" + "% s" % bitwidth + "_H\n\n")
            
            f.write("struct staticbp_"+ "% s" % inputdatatype+ "_" + "% s" % outputdatatype + "_" + "% s" % bitwidth + " {\n" \
            "    using base_t = uint" + "% s" % inputdatatype + "_t;\n" \
            "    using compressedbase_t = uint" + "% s" % outputdatatype + "_t;\n" \
            "    static const size_t bitwidth = " + "% s" % bitwidth + ";\n" \
            "\n" \
            "    static size_t compress(\n" \
            "        const uint8_t * & in8, const size_t countInLog, uint8_t * out8\n" \
            "        ) {\n" \
            "            const uint"+ "% s" % inputdatatype + "_t * inBase = (const uint"+ "% s" % inputdatatype + "_t *)in8;\n" \
            "            uint"+ "% s" % outputdatatype+"_t * outBase = (uint"+ "% s" % outputdatatype+"_t *)out8;\n" \
            "\n" \
            "            for (size_t i = " + "% s" % outputdatatype + "; i  <= countInLog; i += " + "% s" % outputdatatype + ") {\n")
            bitpos = 0
            cast = "*inBase"
            if inputdatatype < outputdatatype:
	        cast = "((uint" + "% s" % outputdatatype + "_t ) *inBase )" 
            for v in range(outputdatatype ):
                f.write("                // " + "% s" % (v+1) + ". value\n");
                if bitpos == 0:
                    f.write("                *outBase = *inBase;\n")
                else:
                    f.write("                *outBase |= " + cast + " << " + "% s" % bitpos + ";\n")
                newbitpos = bitpos + bitwidth
                cnt = 0
                while newbitpos >= outputdatatype:
                    cnt = cnt + 1
                    f.write("                outBase++;\n")
                    if newbitpos > outputdatatype:
            	        #f.write("                outBase++;\n")
                        #if newbitpos > outputdatatype:
            	        f.write("                *outBase = " + cast + " >> " + "% s" % (cnt*outputdatatype - bitpos) + ";\n")
                    newbitpos = newbitpos - outputdatatype
                f.write("                inBase++;\n")
                bitpos = (bitpos + bitwidth) % outputdatatype
            if (bitpos != 0):
                f.write("                outBase++;\n")
            f.write("            }\n" \
            "            return (uint8_t*) outBase-out8;\n" \
            "    }\n" \
            "\n" \
            "    static size_t decompress(\n" \
            "                const uint8_t * & in8, const size_t countInLog, uint8_t * out8\n" \
            "                ) {\n" \
            "\n" \
            "            const uint"+ "% s" % outputdatatype + "_t * inBase = (const uint"+ "% s" % outputdatatype + "_t *)in8;\n" \
            "            uint"+ "% s" % inputdatatype+"_t * outBase = (uint"+ "% s" % inputdatatype+"_t *)out8;\n\n" \
            "            for (size_t i = " + "% s" % outputdatatype + "; i <= countInLog; i += " + "% s" % outputdatatype + ") {\n")
            #bitpos = 0
            bitposout = 0
            mask = (2**bitwidth)-1;
            cast = "*inBase"
            if inputdatatype > outputdatatype:
	        cast = "((uint" + "% s" % inputdatatype + "_t ) *inBase )" 
            for v in range(outputdatatype):
                f.write("            // " + "% s" % (v+1) + ". value\n");
                if bitposout == 0:
		    if bitposout + bitwidth == outputdatatype:
                        f.write("           *outBase = *inBase;\n")
		    else:
                        f.write("            *outBase = *inBase & " + "% s" % mask + "U;\n")
                else:
                    f.write("            *outBase |= (" + cast +" >> " + "% s" % bitposout + ") & " + "% s" % mask+ "U;\n")
		newbitpos = bitposout + bitwidth
                cnt = 0
                while newbitpos >= outputdatatype:
                    cnt = cnt + 1
                    f.write("            inBase++;\n")
                    if newbitpos > outputdatatype:
                        #f.write("            inBase++;\n")
                        #if newbitpos > outputdatatype:
                        f.write("            *outBase |= (" + cast +" << " + "% s" % (cnt*outputdatatype - bitposout) + ") & " + "% s" % mask + "U;\n");
                    newbitpos = newbitpos - outputdatatype
                f.write("            outBase++;\n")
                #bitpos = (bitpos + bitwidth) % inputdatatype
                bitposout = (bitposout + bitwidth) % outputdatatype
                if (bitpos != 0):
                    f.write("            inBase++;\n")
            f.write("    }\n" \
            "    return (uint8_t*) outBase-out8;\n" \
            "    }\n" \
            "};\n#endif /* STATICBP_"+ "% s" % inputdatatype+ "_" + "% s" % outputdatatype + "_" + "% s" % bitwidth + "_H */")
            f.close()'''
