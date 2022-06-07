import os, sys
import time
def run(cmd, old) :
    apks = os.listdir(sys.argv[1])
    id = 0
    for apk in  apks:
        apk_path = os.path.join(sys.argv[1], apk)
        print(apk_path)
        print("--------------%s/%s---------------"%(str(id), str(len(apks))))
        id += 1
        start = time.time()
        if old == '0' :
            os.system('timeout 30 ./StaticAnalyzer/StaticAnalyzer -i %s -v %s --act=all -o %s.txt'%(apk_path, cmd, cmd))
        if old == '1' :
            os.system('timeout 30 ./StaticAnalyzer/StaticAnalyzer -i %s -v %s --old --act==all -o %s.txt'%(apk_path, cmd, cmd))
        end = time.time()
        f = open('time.txt', 'w')
        f.write(str(end-start))
        f.close()
        os.system('mkdir %s/%s && mv %s.txt %s/%s/'%(cmd, apk, cmd, cmd, apk))
        os.system('mv time.txt %s/%s/'%(cmd, apk))

def test() :
    apks = os.listdir(sys.argv[1])
    id = 0
    for apk in  apks:
        apk_path = os.path.join(sys.argv[1], apk)
        #print(apk_path)
        #print("--------------%s/%s---------------"%(str(id), str(len(apks))))
        id += 1
        path = os.path.join(apk_path, 'AndroidManifest.txt')
        if os.path.exists(path) :
            f = open(os.path.join(apk_path, 'AndroidManifest.txt'))
            for line in f.readlines():
                if 'taskAffinity:' in line :
                    print(apk)
                    print(line)

run(sys.argv[2], sys.argv[3])
#test()
