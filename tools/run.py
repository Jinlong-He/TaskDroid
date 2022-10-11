import argparse, time, os, sys, re
import subprocess
pattern_re = '.*\[(.*)\].*Pattern Found.*'

def read_output(output_file) :
    res = []
    f = open(output_file, 'r')
    pattern_flag = 0
    trace_flag = 0
    prop = ''
    pattern = []
    trace = []
    for line in f.readlines() :
        if not pattern_flag :
            match = re.match(pattern_re, line)
            if match :
                prop = match.group(1)
                pattern_flag = 1
                pattern = []
                trace = []
        else :
            if '---Pattern END---' not in line :
                pattern.append(line.strip())
            else :
                pattern_flag = 0
        if 'Trace Found:' in line :
            trace_flag = 1
            continue
        if trace_flag :
            if '---Trace END---' not in line :
                trace.append(line.strip().split(' -> ')[0])
            else :
                trace_flag = 0
                res.append((prop, pattern, trace))
    return res

parser = argparse.ArgumentParser()
parser.add_argument('-i', '--input', help = 'extracts the model from the apk file')
parser.add_argument('-o', '--output', help = 'Sets the output file')
parser.add_argument('-p', '--property', help = 'verifies the property')
parser.add_argument('-e', '--engine', help = 'Sets the engine for verifying', type = str, default = 'nuxmv')
parser.add_argument('-t', '--timeout', help = 'sets the timeout', type = int, default = 300)
parser.add_argument('-k', '--number', help = 'sets the number of tasks involved for verifying task-unboundedness', type = int, default = 1)
parser.add_argument('-b', '--bound', help = 'sets the bound of the height of stack', type = int, default = 6)

conf = open('.conf','r')
nuxmv_cmd = conf.readline().strip()
timeout_cmd = conf.readline().strip()
sdk_path = 'ICCExtractor/sdk'

args = parser.parse_args()
apk_path = ''
property = ''
timeout = args.timeout
engine = args.engine
k = args.number
h = args.bound
output = 'taskdroid.out'
if args.input :
    apk_path = args.input
else :
    print('Input apk file is (null). You must set the apk file before.')
    exit(0)
if args.output :
    output = args.output
if args.property :
    property = args.property
else :
    print('Property is (null). You must set the property before.')
    exit(0)
(apk_dir, apk) = os.path.split(apk_path)
apk_name = apk[:-4]
icc_dir = os.path.join('outputDir', apk_name)
manifest = os.path.join(icc_dir, 'manifest/AndroidManifest.txt')
ictg = os.path.join(icc_dir, 'ictg/ictgOpt.xml')
fragment = os.path.join(icc_dir, 'fragment/SingleObject_entry.xml')
if not os.path.exists('./log/%s'%(apk_name)) :
    os.system('mkdir log/%s'%(apk_name))
log_file = open('log/%s/%s.log'%(apk_name,apk_name),'w')

print('*** This is TaskDroid 2.0.0')
print('*** Copyrighy (c) 2019-2022, Jinlong He')
print('*** For more information on TaskDroid see https://github.com/Jinlong-He/TaskDroid')
print('*** or email to <hejl@ios.ac.cn>.')
print('-- Extracting model...')
icc_cmd = "java -jar ICCExtractor/ICCExtractor.jar -path %s -name %s -androidJar %s/platforms -time %s -scenarioStack -noWrapperAPI -onlyDummyMain"%(apk_dir, apk, sdk_path, int(timeout/60))
ret,val = subprocess.getstatusoutput(icc_cmd)
log_file.writelines(val)
print('-- Extract model......', end = '')
if (not (os.path.exists(manifest) and os.path.exists(ictg) and os.path.exists(fragment) )) :
    print('\033[0;31;40mFailed!\033[0m')
else :
    print('\033[0;32;40mDone!\033[0m')
os.system('mkdir icc_outs')
os.system('cp ' + manifest + ' icc_outs')
os.system('cp ' + ictg + ' icc_outs')
os.system('cp ' + fragment + ' icc_outs')
os.system('rm -r outputDir')
print('-- Verifying model...')
analysis_cmd = './StaticAnalyzer/StaticAnalyzer -i icc_outs -o %s -v %s -e %s -k %s -n %s'%(output, property, engine, h, k)
ret,val = subprocess.getstatusoutput(analysis_cmd)
log_file.writelines(val)
os.system('rm -r icc_outs')
print('-- Verify model.......', end = '')
print('\033[0;32;40mDone!\033[0m')
res = read_output(output)
id = 1
for prop, pattern, trace in res :
    print('-- Property %s is true'%(prop))
    if 'Task-Unboundedness' in prop :
        print('-- An Activity witness cycle is found:')
        for act in pattern :
            print('\t%s ->'%(act))
        print('\t%s'%(pattern[0]))
        print('-- An Activity trace is found:')
        for act in trace :
            print('\t%s ->'%(act))
        print('\t%s'%(pattern[0]))
    print('<!--########## Property number %s ##########--!>'%(id))
    print()
    id += 1

if not args.output :
    os.system('mv %s log/%s/'%(output, apk_name))
    






