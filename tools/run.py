import uiautomator2 as u2
import argparse
import time
import os
import sys
import subprocess

from ui_binder import activity_finder 
from ui_binder import trace_reader 

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', '-i')
    parser.add_argument('--timeout', '-t')
    parser.add_argument('--verify', '-v')
    parser.add_argument('--engine', '-e')
    parser.add_argument('--sdk', '-s')
    args = parser.parse_args()
    apk = ''
    verify = ''
    engine = 'nuxmv'
    sdk = '/Users/hejl/Library/Android/sdk'
    timeout = 10
    output = 'out.txt'
    if (args.timeout) :
        timeout = args.timeout
    if (args.sdk) :
        sdk = args.sdk
    if (args.input) :
        (apk_dir, apk) = os.path.split(args.input)
    else :
        print('no input apk')
        sys.exit(0)
    apk_name = os.path.split(apk)[1][:-4]
    icc_dir = 'outputDir/'+apk_name+'/'
    manifest = icc_dir + 'manifest/AndroidManifest.txt'
    ictg = icc_dir + 'ictg/ictgMerge.xml'
    fragment = icc_dir + 'fragment/SingleObject_entry.xml'
    print('ICCExtractor Start...')
    icc_cmd = "java -jar ICCExtractor/ICCExtractor.jar  -path "+ apk_dir +" -name "+ apk + " -androidJar "+ sdk +"/platforms -time " + str(timeout) + " -maxPathNumber 100 >> logs/"+ apk + ".txt"
    os.system(icc_cmd)
    if (not (os.path.exists(manifest) and os.path.exists(ictg) and os.path.exists(fragment))) :
        print('ICCExtractor Failed!')
        sys.exit(0)
    print('ICCExtractor End!')
    print('Static Analysis Start...')
    if (not os.path.exists('icc_out')) :
        os.system('mkdir icc_out')
    os.system('cp ' + manifest + ' icc_out/')
    os.system('cp ' + ictg + ' icc_out/')
    os.system('cp ' + fragment + ' icc_out/')
    os.system('rm -r outputDir')
    if (args.engine) :
        engine = args.engine
    if (args.verify) :
        verify = args.verify
        os.system('./TaskDroid -i icc_out -o static_out.txt -v ' + verify + ' -e ' + engine)
    print('Static Analysis End!')
    print('Binding Start...')
    reader = trace_reader("static_out.txt")
    (package, main_activity, pattens, traces) = reader.read()
    os.system("python3 -m uiautomator2 init")
    device = u2.connect()
    path_dic = {}
    for i in range(len(traces)) :
        if (len(traces[i]) == 0) : continue
        os.system("adb install " + args.input)
        time.sleep(3)
        device.app_start(package)
        time.sleep(3)
        finder = activity_finder(package, args.input, main_activity, device, device.app_current()['activity'], path_dic, 1)
        (click_path, path_dic) = finder.find(traces[i])
        device.app_stop(package)
        os.system("adb uninstall " + package)
        time.sleep(3)
        if (len(click_path)) :
            os.system("adb install " + args.input)
            time.sleep(3)
            device.app_start(package)
            time.sleep(3)
            finder.execute(click_path, traces[i], pattens[i], 2)
            device.app_stop(package)
            os.system("adb uninstall " + package)
