import os, commands, collections,sys,shutil, subprocess,time,csv

import config,parse,getTopCaller

home = os.environ['HOME']
PATH = os.getcwd()

def runCGGenerator(apk_path, info_dir, apk_name):
    #cg_folder = sys.argv[3]
    os.system('java -Xmx4g -jar %s %s' % (os.path.join(PATH, 'CGGenerator.jar'), apk_path))
    if not os.path.exists('CGs/'+apk_name.replace('.apk', '_cg.txt')):
        return ''
    else:
        os.system('cp %s %s' % ('CGs/'+ apk_name.replace('.apk', '_cg.txt'), info_dir))

    # if not os.path.exists(info_dir+'/'+apk_name.replace('.apk','_cg.txt')):
    #     if not os.path.exists(home+'/CGs/'+apk_name.replace('.apk','_cg.txt')):
    #         os.system('java -Xmx4g -jar %s %s'%(os.path.join(PATH, 'CGGenerator.jar'),apk_path))
    #     os.system('cp %s %s'%(os.environ['HOME']+'/CGs/'+apk_name.replace('.apk','_cg.txt'), info_dir))

        return info_dir+'/'+apk_name.replace('.apk','_cg.txt')
def staticScanning(apk_path,PATH, info_dir):
    cmd = 'java -jar %s %s %s'%(os.path.join(PATH, 'StaticDetection.jar'), apk_path, info_dir)
    print cmd
    subprocess.call(cmd, shell=True)
def runGator(apk_path, info_dir):
    #gator_home = os.environ['GatorRoot']
    gator_home = config.gator_home
    #os.chdir(gator_home)
    apk_filename = os.path.split(apk_path)
     #gator's result is saved in home dir by default
    GUIHierarchy_file = '%s/%s_arrow.xml' % (info_dir, apk_filename[1].split('.apk')[0])
    if not os.path.exists(GUIHierarchy_file):
        cmd = 'timeout 5m python AndroidBench/runGatorOnApk.py %s -client GUIHierarchyPrinterClient'%apk_path
        print cmd
        os.system(cmd)
        try:
            os.system('mv %s/gui_transition.xml %s'%(home, GUIHierarchy_file))
        except IOError:
            print 'Fail in Gator....................'
    try:
        parse.parse_gator(GUIHierarchy_file, info_dir)
    except IOError:
        print 'Gator execution failure...'

def launcher(apk_path):
    launchActivity = commands.getoutput(
        'aapt dump badging %s | grep Activity | awk \'{print $2}\' | sed s/name=//g | sed s/\\\'//g'%apk_path)
    if launchActivity == '':
        launchActivity = commands.getoutput(
            'aapt dump badging %s | grep launchable-activity | awk \'{print $2}\' | sed s/name=//g | sed s/\\\'//g' % apk_path)
    pkg_name = commands.getoutput('aapt dump badging %s | grep package | awk \'{print $2}\' | sed s/name=//g | sed s/\\\'//g'%apk_path)

    print '\n----------LaunchActivity----------'
    print 'launchActivity: ' + launchActivity
    return launchActivity, pkg_name

def runic3(apk_path, info_dir):
    sootOutput = info_dir + '/'+ os.path.splitext(apk)[0] + '_SootOuputput'
    ic3cmd = 'timeout 5m java -Xmx4g -jar %s -apkormanifest %s -in %s -cp %s -protobuf %s | grep "PATH: "' % (
        config.ic3_home + 'ic3-0.2.0-full.jar', apk_path, sootOutput, config.ic3_home + 'android.jar', info_dir)
    print ic3cmd

    output_path = commands.getoutput(ic3cmd).split('\n')[-1]
    try:
        tmpresult = output_path.split('PATH: ')[1]
        os.system('mv %s %s' % (tmpresult, info_dir + '/' + os.path.splitext(apk)[0] + '_ic3result.txt'))
    except IndexError:
        print apk_path
    #remove sootOutput dir
    shutil.rmtree(sootOutput)

def static_execute(PATH):
    if not os.path.exists(info_dir):
        os.mkdir(info_dir)
        print info_dir

    launchActivity, pkg_name = launcher(apk_path)
    open(info_dir+'/launcher.txt','wb').write(launchActivity)

    cg_start = time.time()
    cg_file = runCGGenerator(apk_path, info_dir, apk)
    if cg_file == '':
        return
    cg_end = time.time()
    timedict['cg'] = int(cg_end-cg_start)

    try:
        dict = parse.parseCG(cg_file,pkg_name)
    except Exception:
        return

    parse.parseCG2(cg_file,pkg_name) # parse for event sequence generation

    try:
        staticScanning(apk_path, PATH, info_dir)
    except Exception:
        return
    flag = 0
    if not os.path.exists(info_dir + '/' + os.path.splitext(apk)[0]+'_report.txt'):
        return
    else:
        lines = open(info_dir + '/' + os.path.splitext(apk)[0]+'_report.txt','rb').readlines()
        for line in lines:
            if 'statement' in line:
                flag = 1
                break
    if flag == 0:
        return
    targetmethods, apkload, apkexe = parse.parseStaticResult(apk, info_dir)

    timedict['staticload'] = apkload
    timedict['staticexe'] = apkexe

    ic3_start = time.time()
    runic3(apk_path, info_dir)
    ic3_end = time.time()
    timedict['ic3'] = int(ic3_end-ic3_start)

    gator_start = time.time()
    runGator(apk_path, info_dir)
    gator_end = time.time()
    timedict['gator']= int(gator_end-gator_start)

    ic3result = info_dir + '/'+ os.path.splitext(apk)[0] + '_ic3result.txt'
    ATG_dict={}
    try:
       ATG_dict= parse.parseIC3(ic3result)  # Activity Transition Graph
    except IOError:
       print 'Fail in IC3, no such file....'

    atg_file = info_dir + '/atg.txt'
    open(atg_file,'wb').write('')
    for k, v in ATG_dict.items():
        open(atg_file,'ab').write(k + ' ==> '+ v + '\n')

    transition_file = info_dir + '/transition.txt'
    open(transition_file, 'wb').write('')
    description_file = info_dir + '/instru_description.txt'
    open(description_file, 'wb').write('')
    for key in targetmethods.keys():
        mth = key.split('_')[1]
        stm = key.split('_')[0]
        print 'targetMethod: %s' % mth
        open(transition_file, 'ab').write(mth + ';' + stm+ '\n')

        # for instrument
        open(description_file, 'ab').write('\n====%s====: '%mth)
        if ("fragment" in getTopCaller.getClass(mth).lower() or "activity" in getTopCaller.getClass(mth).lower()):
            caller_c = getTopCaller.getClass(mth)
            caller_m = mth
        else:
            caller_c, caller_m = getTopCaller.getCallerActivity(mth, dict)
        open(description_file, 'ab').write('"%s: void onDestroy()" ' % caller_c)
        open(description_file, 'ab').write('"%s" ' % caller_m)
        open(description_file, 'ab').write('"%s" "%s"' % (targetmethods[key], apk))

def guided_path(apk_path,info_dir):

    py_path = sys.path[0]
    PATH = py_path
    load_time = []
    exe_time = []

    transition_file = info_dir + '/transition.txt'
    if os.path.exists(transition_file):
        lines = open(transition_file,'rb').readlines()
        for line in lines:
            mtd = line.strip().split(';')[0]
            stm = line.strip().split(';')[1]
            cmd = 'timeout 5m java -jar %s %s "%s" "%s"'%(os.path.join(PATH, 'guided_path.jar'), apk_path, mtd, stm)
            print cmd
            #success
            path_file = info_dir + '/path_' + mtd + '.txt'
            if not os.system(cmd) and os.path.exists(path_file):
                load_time.append(int(commands.getoutput('grep "Apk load time" "%s" | awk \'{print $4}\''%path_file).strip()))
                exe_time.append(int(commands.getoutput('grep "Apk execution time" "%s" | awk \'{print $4}\''%path_file).strip()))
    if(len(load_time)>0 and len(exe_time)>0):
        timedict['pathload'] = int(sum(load_time)) / len(load_time)
        timedict['pathexe'] = int(sum(exe_time)) / len(exe_time)

    #total time
    exe_end_time = time.time()

    timedict['total'] = int(exe_end_time - exe_start_time)

    #time_writer.writerow((timedict['apk'],timedict['cg'],timedict['staticload'],timedict['staticexe'],
    #                     timedict['ic3'],timedict['gator'],timedict['pathload'],timedict['pathexe'],timedict['total'],float(timedict['total'])/60))
def findView(handler,d):
    view = ''
    for k,v in d.items():
        if v == handler:
            return k
    return view
def mapToView(info_dir, path_f):
    map_file = info_dir + '/view_handler.txt'
    v_h_dict = {}
    lines = open(map_file,'rb').readlines()
    for line in lines:
        v_h_dict[line.split(' ==> ')[0]] = line.split(' ==> ')[1].split(';')[0]

    file = open(path_f)
    flag = 0
    p_list = []
    i = 0
    line = file.readline()
    while line:
        if 'final traces' in line:
            flag = 1
        elif flag == 1:
            if 'Terminated' in line:
                if len(p_list) > 0:
                    for v in p_list:
                        open(path_f,'ab').write(v+' ==> ')
                    open(path_f, 'ab').write('\n')
                p_list = []
                i = i+1
                handler = line.split('Terminated]')[1].split(' ->')[0]
                view = findView(handler, v_h_dict)
                p_list.insert(0,view)
            elif ' -> ' in line:
                handler = line.strip().split(' ->')[0]
                view = findView(handler, v_h_dict)
                p_list.insert(0,view)
        line = file.readline()
    if len(p_list) > 0:
        for v in p_list:
            open(path_f, 'ab').write(v + ' ==> ')
if __name__ == '__main__':

    print 'start...'
    #if len(sys.argv) < 3:
    if len(sys.argv) < 2:
        sys.exit("Please input an apk path or a folder containing apks...\n python run.py [apk_dir/apk_path] [timecsvfile.csv]")
    input = sys.argv[1] # folder or apk file
    #timecsvout = sys.argv[2] # time out file
    #csv.writer(open(timecsvout, 'ab')).writerow(('apk', 'cg', 'static_apk_load', 'static_exe', 'ic3', 'gator', 'path_apk_load', 'path_exe',
                          #'record_total_exe(s)', 'total_time(min)'))

    timedict = {'apk': 'null', 'cg': 0, 'staticload': 0, 'staticexe': 0,
                'ic3': 0, 'gator': 0, 'pathload': 0, 'pathexe': 0,
                'total': 0}
    if os.path.isdir(input):
        for apk in os.listdir(input):
            if not apk.endswith('.apk'):
                continue
            #time_writer = csv.writer(open(timecsvout,'ab'))
            timedict['apk'] = apk
            exe_start_time = time.time()

            apk_path = os.path.join(input, apk)
            info_dir = os.path.join(input, os.path.splitext(apk)[0])
            static_execute(PATH)

            guided_path(apk_path,info_dir)

            for f in os.listdir(info_dir):
                if f.startswith('path_'):
                    guided_path_f = info_dir + '/' + f
                    mapToView(info_dir, guided_path_f)

    elif input.endswith('.apk'):

        exe_start_time = time.time()

        apk = os.path.basename(input)
        apk_path = input
        info_dir = input.split('.apk')[0]
        runGator(apk_path, info_dir)
        #static_execute(PATH)

        #guided_path(apk_path, info_dir)

        for f in os.listdir(info_dir):
            if f.startswith('path_'):
                guided_path_f = info_dir + '/' + f
                mapToView(info_dir, guided_path_f)

    else:
        sys.exit("Please input an apk path or a folder containing apks...")

    print 'Done!!!'


