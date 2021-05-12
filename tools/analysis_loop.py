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
if __name__ == '__main__' :
    path = sys.argv[1]
    analyze(path)
