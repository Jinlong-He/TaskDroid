import os
import sys
import shutil

def analyzeBackHijaking(path):
    bh_num = 0
    apk_num = 0
    apks = os.listdir(path)
    for apk in apks:
        manifest = os.path.join(os.path.join(path, apk), "AndroidManifest.txt")
        ictg = os.path.join(os.path.join(path, apk), "ictgMerge.xml")
        if (os.path.exists(ictg)) :
            apk_num += 1
            f = open(manifest)
            flag = False
            for line in f.readlines() :
                if ('launchMode: 2' in line) :
                    flag = True
                    break
            f.close()
            f = open(ictg)
            for line in f.readlines() :
                if ('FLAG_ACTIVITY_NEW_TASK' in line) :
                    flag = True
                    break
            f.close()
            if (flag) : bh_num += 1
    print(bh_num, apk_num)

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

def analyzeTrace(path):
    apk_num = 0
    patten_num = 0
    trace_num = 0
    no_act = 0
    apks = os.listdir(path)
    for apk in apks:
        apk_num += 1
        files = os.listdir(os.path.join(path, apk))
        for file in files :
            f = open(os.path.join(path, apk, file), 'r', encoding='UTF-8')
            pflag = False
            tflag = False
            for line in f.readlines() :
                if "no graph" in line :
                    no_act += 1
                if "Trace" in line :
                    tflag = True
                if "Patten" in line :
                    pflag = True
            if (pflag) :
                print('Patten Found: ' + apk)
                patten_num += 1
            if (tflag) :
                trace_num += 1
                print('Trace Found: ' + apk)
    print(apk_num, patten_num, trace_num, no_act)
if __name__ == '__main__' :
    path = sys.argv[1]
    if (sys.argv[2] == 'bh') :
        analyzeBackHijaking(path)
    elif (sys.argv[2] == 'trace') :
        analyzeTrace(path)
    #bh: (7591, 9613)
