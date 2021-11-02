import xml.etree.cElementTree as ET
import config, getTopCaller, os
import collections

def getHanlder(view):
    eh = ''
    event = ''
    for handler in view:
        if 'realHandler' in handler.attrib:
            eh =  handler.attrib['realHandler']
            event = handler.attrib['event']
        elif 'handler' in handler.attrib:
            eh =  handler.attrib['handler']
            event = handler.attrib['event']
        else:
            print "no handler"
    return eh, event
def getAllCallbacks(tree):
    callbacks = []
    for elem in tree.iter(tag='EventAndHandler'):
        #print elem.attrib
        flag = 0
        for k in elem.attrib:
            #print k
            if 'realHandler' == k:
                callbacks.append(elem.attrib['realHandler'].split('<')[1].split('>')[0])
                flag = 1
                break
        if flag == 0:
            callbacks.append(elem.attrib['handler'].split('<')[1].split('>')[0])
    #print list(set(callbacks))
    return list(set(callbacks))
def findMapping(item,gator_dict,activity):
    for v in item:
        #print v.tag
        if 'EventAndHandler' in v.tag:
            if 'realHandler' in v.attrib:
                eh = v.attrib['realHandler']
                event = v.attrib['event']
            elif 'handler' in v.attrib:
                eh = v.attrib['handler']
                event = v.attrib['event']
            else:
                print "no handler"
            if not eh == '':
                gator_dict[activity + ';' + item.attrib['type']+';'+item.attrib['id'] + ';' + event].add(eh.split('>')[0].split('<')[1])
        else:
            findMapping(v,gator_dict,activity)
def parse_gator(gator_result, info_dir):
    # format: 'activity;ViewType;id;event' : 'eventhandler'
    gator_dict = collections.defaultdict(set)
    tree = ET.parse(gator_result)
    root = tree.getroot()
    callbacks = getAllCallbacks(tree)
    for item in root:
        if item.tag == "Activity":
            findMapping(item,gator_dict,item.attrib['name'])

    callbacks_file = info_dir+'/callbacks.txt'
    open(callbacks_file,'wb').write('')
    for cb in callbacks:
        open(callbacks_file,'ab').write(cb+'\n')

    viewHandler_file = info_dir + '/view_handler.txt'
    open(viewHandler_file, 'wb').write('')
    for view,handlers in gator_dict.items():
        hs = ''
        for h in handlers:
            hs = hs + h + ';'
        if not 'EmmaInstrument' in view:
            open(viewHandler_file, 'ab').write(view + ' ==> ' + hs + '\n')

def getPath(activity_path, dict, caller, launchActivity):
    for key, value in dict.items():
        for v in value:
            if caller == v.split('_')[1]:
                activity_path.insert(0, key+'(%s)'%v.split('_')[0])
                caller = key
                if caller == launchActivity:
                    return activity_path
                else:
                    activity_path = getPath(activity_path, dict, caller,launchActivity)
                    break
    return activity_path


def parseIC3(file):
    dict = collections.defaultdict()
    f = open(file,'rb')
    line = f.readline()
    flag = -1
    s = 0
    while line:
        if 'components {' in line:
            s = 1
        elif s == 1 and 'name:' in line:
            sourceActivity = line.split(': "')[1].split('"')[0]
            s = 2
        elif s == 2 and 'kind: ACTIVITY' in line:
            flag = 0
        elif flag == 0 and "exit_points" in line:
            flag = 1
        elif flag == 1 and 'statement' in line:
            stm = line.split(': "')[1].split('"')[0]
            flag = 2
        # elif flag == 2 and 'class_name' in line:
        #     sourceActivity = line.split(': "')[1].split('"')[0]
        #     # if "$" in sourceActivity:
            #     sourceActivity = sourceActivity.split('$')[0]
        #    flag = 3
        elif flag == 2 and 'method: "' in line:
            mtd = line.split(': "<')[1].split('>"')[0]
            flag = 3
        elif flag == 3 and 'kind: CLASS' in line:
            flag =4
        elif flag ==4 and 'value' in line:
            if ': "L' in line:
                targetActivity = line.strip().split(': "L')[1].split(';"')[0].replace('/','.')
            else:
                targetActivity = line.strip().split(': "')[1].split(';"')[0].replace('/','.')
            dict[sourceActivity+';'+mtd+';'+stm] = targetActivity
            flag = 0
        else:
            s = 0
        line = f.readline()
    # for k,v in dict.items():
    #     for v1 in v:
    #         print k + '===>'+v1
    return dict
def parseStaticResult(apk,info_dir):
    source_background = {}
    apkload = 0
    apkexe = 0
    result = info_dir + '/' + os.path.splitext(apk)[0]+'_report.txt'
    lines = open(result,'rb').readlines()
    for line in lines:
        if line.startswith('statement ='):
            tmp_key = line.split('statement = ')[1].split(' ==> ')[0] + '_' + line.split('source = ')[1].split(' ==> ')[0]
            source_background[tmp_key] = line.strip('\n').split('backgroundMethod = ')[1]
        if line.startswith('Apk load time'):
            apkload = line.split(': ')[1].split(' Seconds')[0]
        if line.startswith('Apk execution'):
            apkexe = line.split(': ')[1].split(' Seconds')[0]
    return source_background, apkload, apkexe
def parseCG(cg_file,pkg_name):
    dict = collections.defaultdict(set)
    f = open(cg_file, 'rb')
    line = f.readline()
    while line:
        key = line.split(' in <')[1].split('> ==> <')[0]
        value = line.split('> ==> <')[1][0:-1]
        if pkg_name in key and pkg_name in value and not 'EmmaInstrument' in key and not 'EmmaInstrument' in value:
            dict[key].add(value)
        line = f.readline()
    dict_outfile = cg_file.replace('_cg.txt','_cgdict.txt')
    (open(dict_outfile,'wb')).write('')
    for key in dict.keys():
        s = ''
        for v in dict[key]:
            s = v + ';' + s
        (open(dict_outfile, 'ab')).write(key + '==>' + s +'\n')
    return dict

def parseCG2(cg_file, pkg_name):
    cgdict_stm = cg_file.replace('_cg.txt','_cgdict2.txt')
    open(cgdict_stm,'wb').write('')
    f = open(cg_file, 'rb')
    line = f.readline()
    while line:
        stm = line.split('edge: ')[1].split(" in <")[0]
        key = line.split(' in <')[1].split('> ==> <')[0]
        value = line.split('> ==> <')[1][0:-1]
        if pkg_name in key and pkg_name in value and not 'EmmaInstrument' in key and not 'EmmaInstrument' in value:
            open(cgdict_stm,'ab').write('%s;%s==>%s\n'%(key,stm,value))
        line = f.readline()
