import os
import sys
import shutil

def analyze(path):
    apk_num = 0
    no_fragment_num = 0
    fragment_num = 0
    loop_num = 0
    apks = os.listdir(path)
    for apk in apks:
        apk_num += 1
        files = os.listdir(os.path.join(path, apk))
        for file in files :
            f = open(os.path.join(path, apk, file))
            for line in f.readlines() :
                if "No Fragments" in line :
                    no_fragment_num += 1
                    print(apk)
                if "Has Fragments" in line :
                    fragment_num += 1
                    print(apk)
                if "Loop" in line :
                    loop_num += 1
                    print("Loop: " + apk)
    print(no_fragment_num, fragment_num, loop_num)

def analyze1(path):
    apk_num = 0
    patten_num = 0
    trace_num = 0
    no_act = 0
    apks = os.listdir(path)
    for apk in apks:
        apk_num += 1
        files = os.listdir(os.path.join(path, apk))
        for file in files :
            f = open(os.path.join(path, apk, file))
            pflag = False
            tflag = False
            for line in f.readlines() :
                if "no graph" in line :
                    no_act += 1
                if "Activity Trace" in line :
                    tflag = True
                if "Patten" in line :
                    pflag = True
                #if "task num" in line :
                #    if (int(line[line.find(": ") + 2:]) > 1) :
                #        print(line)
                #        flag = True
                #    print(apk)
                #    print(line)
                #if "activity num" in line :
                #    print(line)
            if (tflag) :
                print(apk)
                trace_num += 1
            if (pflag) :
                print(apk)
                patten_num += 1
    print(apk_num, patten_num, trace_num, no_act)
if __name__ == '__main__' :
    path = sys.argv[1]
    analyze1(path)
