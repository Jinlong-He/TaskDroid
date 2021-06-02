import os
import sys
import shutil

def analyzeApk(path, sdk):
    if(os.path.exists(path)): 
        apks = os.listdir(path)
        for apk in apks:
            if apk[-4:] ==".apk":
                outPath = "outputDir/" +apk[:-4]+ "/"
                if not os.path.exists("result/" + apk[:-4]) :
                    os.system("cd result && mkdir " + apk[:-4] + " && cd ..")
                if os.path.exists(outPath + "ictg") :
                    size = os.path.getsize(outPath+"fragment/SingleObject_entry.xml")
                    if size > 4000 :
                        print(apk)
                        print(size)
                    os.system("./build/bin/TaskDroid " + 
                            outPath + "manifest/AndroidManifest.txt " +
                            outPath + "ictg/ictgMerge.xml " +
                            outPath + "fragment/SingleObject_entry.xml " +
                            "result/" + apk[:-4] + "/loop")

if __name__ == '__main__' :
    path = sys.argv[1]
    sdk = "/home/hejl/sdk"
    #os.system("mvn clean package")
    #shutil.copy("target/ICCExtractor-1.0-SNAPSHOT.one-jar.jar", "ICCExtractor.jar")
    if(not os.path.exists("logs")): 
        os.makedirs("logs") 
    analyzeApk(path, sdk)
