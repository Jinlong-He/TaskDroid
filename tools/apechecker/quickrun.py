import os, commands, collections,sys,shutil, subprocess,time,csv
from multiprocessing import Process
import config,parse,getTopCaller

home = os.environ['HOME']
PATH = os.getcwd()

def runCGGenerator(apk_path, info_dir, apk_name):
    #cg_folder = sys.argv[3]
    os.system('java -Xmx4g -jar %s %s' % (os.path.join(PATH, 'CGGenerator.jar'), apk_path))
    if not os.path.exists(home+'/CGs/'+apk_name.replace('.apk', '_cg.txt')):
        return ''
    else:
        os.system('cp %s %s' % (home+'/CGs/'+ apk_name.replace('.apk', '_cg.txt'), info_dir))
        #os.system('cp %s %s' % ('../scidonthego_cg.txt', info_dir))
    #if not os.path.exists('../icc_out/cg.txt'):
    #    return ''
    #else:
    #    os.system('cp %s %s' % ('../icc_out/cg.txt', info_dir))

    # if not os.path.exists(info_dir+'/'+apk_name.replace('.apk','_cg.txt')):
    #     if not os.path.exists(home+'/CGs/'+apk_name.replace('.apk','_cg.txt')):
    #         os.system('java -Xmx4g -jar %s %s'%(os.path.join(PATH, 'CGGenerator.jar'),apk_path))
    #     os.system('cp %s %s'%(os.environ['HOME']+'/CGs/'+apk_name.replace('.apk','_cg.txt'), info_dir))
        return info_dir+'/'+apk_name.replace('.apk','_cg.txt')
        #return info_dir+'/cg.txt'
def staticScanning(apk_path,PATH, info_dir):
    cmd = 'java -jar %s %s %s'%(os.path.join(PATH, 'StaticDetection.jar'), apk_path, info_dir)
    print cmd
    subprocess.call(cmd, shell=True)
def runGator(apk_path, info_dir):
    print('run gator')
    #gator_home = os.environ['GatorRoot']
    #gator_home = config.gator_home
    #os.chdir(gator_home)
    apk_filename = os.path.split(apk_path)
     #gator's result is saved in home dir by default
    GUIHierarchy_file = '%s/%s_arrow.xml' % (info_dir, apk_filename[1].split('.apk')[0])
    if not os.path.exists(GUIHierarchy_file):
        cmd = 'timeout 5m ../gator/gator/gator a -p %s -client GUIHierarchyPrinterClient --api 28'%apk_path
        print cmd
        r = os.popen(cmd)
        gator_out = r.read()
        r.close()
        gui_file = gator_out.split('[GUIHierarchyPrinterClient] \x1b[32mVERBOSE\x1b[0m XML file: ')[1].split('\n')[0]
        try:
            os.system('mv %s %s'%(gui_file, GUIHierarchy_file))
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

def static_execute(PATH):
    if not os.path.exists(info_dir):
        os.mkdir(info_dir)
        print info_dir

    launchActivity, pkg_name = launcher(apk_path)
    open(info_dir+'/launcher.txt','wb').write(launchActivity)

    cg_start = time.time()
    cg_file = ''
    cg_file = runCGGenerator(apk_path, info_dir, apk)
    #if not os.path.exists('../icc_out/%s_cg.txt'%(apk[:-4])):
    #    cg_file = ''
    #else:
    #    os.system('cp %s %s' % ('../icc_out/*_cg.txt', info_dir))
    #    cg_file = info_dir + '/%s_cg.txt' %(apk[:-4])
    if cg_file == '':
        return
    cg_end = time.time()
    timedict['cg'] = int(cg_end-cg_start)

    try:
        dict = parse.parseCG(cg_file,pkg_name)
    except Exception:
        return
    parse.parseCG2(cg_file,pkg_name) # parse for event sequence generation
    gator_start = time.time()
    runGator(apk_path, info_dir)
    gator_end = time.time()
    timedict['gator']= int(gator_end-gator_start)
    return

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

def guided_path(apk_path,info_dir,current_activity, target_activity):

    py_path = sys.path[0]
    PATH = py_path
    load_time = []
    exe_time = []
    mtd = '%s: void onCreate(android.os.Bundle)' %(target_activity)
    cmd = 'timeout 5m java -jar %s %s "%s" "%s" "%s"'%(os.path.join(PATH, 'guided.jar'), apk_path, mtd, mtd, current_activity)
    print cmd
    os.system(cmd)
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
    print v_h_dict

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
    if len(sys.argv) < 2:
        sys.exit("Please input an apk path or a folder containing apks...\n python run.py [apk_dir/apk_path] [timecsvfile.csv]")
    input = sys.argv[1] # folder or apk file
    call_trace_file_name = sys.argv[2]
    timedict = {'apk': 'null', 'cg': 0, 'staticload': 0, 'staticexe': 0,
                'ic3': 0, 'gator': 0, 'pathload': 0, 'pathexe': 0,
                'total': 0}

    exe_start_time = time.time()
    apk = os.path.basename(input)
    apk_path = input
    info_dir = os.path.split(input)[1].split('.apk')[0]
    static_execute(PATH)
    call_trace_file = open(call_trace_file_name)
    calls = []
    flag = False
    call_set = set()
    for line in call_trace_file.readlines() :
        if '-Activity Transition' in line :
            flag = True
            continue
        if '-Fragment' in line :
            flag = False 
            continue
        if flag :
            new_line = ''
            if '(' in line :
                new_line = line.split('(')[0]
            else :
                new_line = line.strip()
            if new_line not in call_set :
                call_set.add(line)
                call = line.strip().split(' -> ')
                calls.append((call[0], call[1]))
    ps = []
    for call in calls :
        p = Process(target = guided_path, args=(apk_path, info_dir, call[0], call[1],))
        ps.append(p)
        p.start()
    for p in ps :
        p.join()
    for f in os.listdir(info_dir):
        if f.startswith('path_'):
            guided_path_f = info_dir + '/' + f
            mapToView(info_dir, guided_path_f)
    print 'Done!!!'


