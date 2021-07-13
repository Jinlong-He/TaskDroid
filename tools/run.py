import uiautomator2 as u2
import argparse
import time
import os
import sys

from ui_binder import activity_finder 
from ui_binder import trace_reader 

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', '-i')
    parser.add_argument('--timeout', '-t')
    parser.add_argument('--verify', '-v')
    parser.add_argument('--sdk', '-s')
    args = parser.parse_args()
    apk = ''
    verify = ''
    sdk = ''
    timeout = 10
    if (args.timeout) :
        timeout = args.timeout
    if (args.sdk) :
        sdk = args.sdk
    if (args.input) :
        (apk_dir, apk) = os.path.split(args.input)
    os.system("java -jar ICCExtractor/ICCExtractor.jar  -path "+ apk_dir +" -name "+ apk + " -androidJar "+ sdk +"/platforms -time " + timeout + " -maxPathNumber 100 >> logs/"+ apk + ".txt")
    if (args.verify) :
        verify = args.verify
        if verify == 'boundedness' :
    apk_name = os.path.split(apk)[1][:-4]
    reader = trace_reader("result/"+apk_name+"/out.txt")
    (package, main_activity, pattens, traces) = reader.read()
    os.system("python3 -m uiautomator2 init")
    device = u2.connect()
    path_dic = {}
    for i in range(len(traces)) :
        if (len(traces[i]) == 0) : continue
        os.system("adb install " + apk)
        time.sleep(3)
        device.app_start(package)
        time.sleep(3)
        finder = activity_finder(package, apk, main_activity, device, device.app_current()['activity'], path_dic, 1)
        (click_path, path_dic) = finder.find(traces[i])
        device.app_stop(package)
        os.system("adb uninstall " + package)
        time.sleep(3)
        if (len(click_path)) :
            os.system("adb install " + apk)
            time.sleep(3)
            device.app_start(package)
            time.sleep(3)
            finder.execute(click_path, traces[i], pattens[i], 2)
            device.app_stop(package)
            os.system("adb uninstall " + package)
