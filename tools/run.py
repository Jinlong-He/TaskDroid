import uiautomator2 as u2
import argparse
import time
import os
import sys
import subprocess

from parser import static_parser

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', '-i')
    parser.add_argument('--timeout', '-t')
    parser.add_argument('--verify', '-v')
    parser.add_argument('--gen', '-g')
    parser.add_argument('--engine', '-e')
    parser.add_argument('--sdk', '-s')
    parser.add_argument('--bind', '-b')
    #parser.add_argument('--static')
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
        apk_path = os.path.abspath(args.input)
    else :
        print('no input apk')
        sys.exit(0)
    apk_name = os.path.split(apk)[1][:-4]
    os.system('mkdir icc_outs/%s'%(apk_name))
    time_file = open('icc_outs/%s/time.txt'%(apk_name), 'w')
    times = ""
    icc_dir = os.path.join('outputDir', apk_name)
    manifest = os.path.join(icc_dir, 'manifest/AndroidManifest.txt')
    ictg = os.path.join(icc_dir, 'ictg/ictgMerge.xml')
    fragment = os.path.join(icc_dir, 'fragment/SingleObject_entry.xml')
    cg = os.path.join(icc_dir, 'CallGraphInfo/*_cg.txt')
    if (args.bind) :
        icc_dir = os.path.join(args.bind, apk_name)
        manifest = os.path.join(icc_dir, 'AndroidManifest.txt')
        ictg = os.path.join(icc_dir, 'ictgMerge.xml')
        fragment = os.path.join(icc_dir, 'SingleObject_entry.xml')
        #icc_dir = args.bind
    if (not args.bind) :
        print('ICCExtractor Start...')
        start = time.time()
        icc_cmd = "java -jar ICCExtractor/ICCExtractor.jar  -path "+ apk_dir +" -name "+ apk + " -androidJar "+ sdk +"/platforms -time " + str(timeout) + " -maxPathNumber 100 >> logs/"+ apk + ".txt -scenarioStack -noWrapperAPI -onlyDummyMain"
        os.system(icc_cmd)
        if (not (os.path.exists(manifest) and os.path.exists(ictg) and os.path.exists(fragment))) :
            print('ICCExtractor Failed!')
            sys.exit(0)
        if (not os.path.exists('icc_out')) :
            os.system('mkdir icc_out')
        os.system('cp ' + manifest + ' icc_out/')
        os.system('cp ' + ictg + ' icc_out/')
        os.system('cp ' + fragment + ' icc_out/')
        os.system('cp ' + manifest + ' icc_outs/%s'%(apk_name))
        os.system('cp ' + ictg + ' icc_outs/%s'%(apk_name))
        os.system('cp ' + fragment + ' icc_outs/%s'%(apk_name))
        os.system('rm -r outputDir')
        #icc_cmd = "java -jar ICCExtractor/ICCBot.jar  -path "+ apk_dir +" -name "+ apk + " -androidJar "+ sdk +"/platforms -time " + str(timeout) + " -maxPathNumber 100 >> logs/"+ apk + ".txt -client CallGraphClient"
        #os.system(icc_cmd)
        end = time.time()
        times += "icc: " + str(round(end - start, 5)) + '\n'
        #os.system('cp ' + cg + ' icc_out/')
        #os.system('cp ' + cg + ' icc_outs/%s'%(apk_name))
        #os.system('rm -r outputDir')
        print('ICCExtractor End!')
    if (args.bind) :
        os.system('./StaticAnalyzer/StaticAnalyzer -i ' + icc_dir + ' -o static_out.txt')
    else :
        os.system('./StaticAnalyzer/StaticAnalyzer -i icc_out -o static_out.txt')
    print('Static Analysis Start...')
    print('Gator Start...')
    start = time.time()
    os.system('cd apechecker && python quickrun.py %s ../static_out.txt' %(apk_path))
    end = time.time()
    times += "gator: " + str(round(end - start, 5)) + '\n'
    print('Gator End!')
    os.system('mv apechecker/%s gator_out/'%(apk_name))
    os.system('rm static_out.txt')
    gator_file = 'gator_out/%s' %(apk_name)
    parser = static_parser()
    parser.parse_gator(gator_file)
    gator_file += '/gator_dict.txt'
    start = time.time()
    if (args.engine) :
        engine = args.engine
    if (args.gen) :
        g = args.gen
        if (args.bind) :
            os.system('./StaticAnalyzer/StaticAnalyzer -i %s -o static_out.txt -g %s -t %s' %(icc_dir, g,gator_file))
        else :
            os.system('./StaticAnalyzer/StaticAnalyzer -i icc_out -o static_out.txt -g %s -t %s' %(g,gator_file))
    if (args.verify) :
        verify = args.verify
        if (args.bind) :
            os.system('./StaticAnalyzer/StaticAnalyzer -i %s -o static_out.txt -v %s -t %s' %(icc_dir, verify, gator_file))
        else :
            os.system('./StaticAnalyzer/StaticAnalyzer -i icc_out -o static_out.txt -v %s -t %s' %(verify, gator_file))
    end = time.time()
    times += "taskdroid: " + str(round(end - start, 5)) + '\n'
    print('Static Analysis End!')
    os.system('mv static_out.txt gator_out/%s/'%(apk_name))
    #os.system('rm static_out.txt')
    #os.system('rm out.smv')
    time_file.write(times)
    time_file.close()
