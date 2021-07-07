import os
import sys
import time

def analyzeApk(path, verifier):
    flog = open("log.txt", 'a')
    if(os.path.exists(path)): 
        apks = os.listdir(path)
        count = 0
        count1 = 0
        for apk in apks:
            #if apk[-4:] ==".apk":
            outPath = "outputDir/" +apk
            resPath = "result/" + apk + "/"
            start = time.time()
            #f = open(outPath + "AndroidManifest.txt")
            #flag = False
            #flag1 = False
            #for line in f.readlines() :
            #    if ("affinity" in line or "Affinity" in line) :
            #        flag = True
            #    if ("launch" in line or "Launch" in line) :
            #        flag1 = True
            #if (flag) : 
            #    print(apk)
            #    count = count + 1
            #if (flag1) : 
            #    print(apk)
            #    count1 = count1 + 1
            if not os.path.exists(resPath) :
                os.system("cd result && mkdir " + apk + " && cd ..")
            pre_cmd = "gtimeout 60 ./TaskDroid -i " + outPath + " -e nuxmv -o " + resPath + "out.txt"
            boundedness_cmd = " -v boundedness "
            backhijacking_cmd = " -v backHijacking "
            frag_boundedness_cmd = " -v frag-boundedness --act=all "
            if (verifier == "boundedness") :
                os.system(pre_cmd + boundedness_cmd)
            elif (verifier == "backHijacking") :
                os.system(pre_cmd + backhijacking_cmd)
            elif (verifier == "frag") :
                os.system(pre_cmd + frag_boundedness_cmd)
            end = time.time()
            f = open(resPath + "out.txt", 'a')
            f.write("time: " + str(end-start))
            f.close()
            print("-------" + apk)
    flog.close()
    #print(count, count1)

if __name__ == '__main__' :
    path = sys.argv[1]
    verifier = sys.argv[2]
    analyzeApk(path, verifier)
