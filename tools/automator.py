import uiautomator2 as u2
import time
import os
import sys

def get_activity(package) :
    activity = d.app_current()['activity']
    if (package in activity) :
        return activity[len(package):]
    else :
        return activity 

def goto_activity(info_path) :
    for info in info_path :
        for elem in d(clickable = 'true') :
            if (info == str(elem.info)) :
                elem.click()
                time.sleep(2)
                break

def get_info(elems) :
    res = ''
    for elem in elems :
        res += str(elem.info) + ','
    return res 

def restart(package, path) :
    d.app_stop(package)
    d.app_start(package)
    time.sleep(2)
    goto_activity(path)

def find_activity(package, activity, path, pre_keys, pre_key, history_path, visited) :
    time.sleep(2)
    current_activity = get_activity(package)
    elems = d(clickable = 'true')
    keys = get_info(elems)
    elems_num = elems.count
    new_path = path
    if (pre_keys not in visited) : visited[pre_keys] = set()
    if (keys not in visited) : visited[keys] = set()
    current_visited = visited[keys]
    for elem in elems :
        key = str(elem.info)
        if (key in current_visited) : continue
        new_path.append(str(elem.info))
        elem.click()
        #print(key)
        #print('---------------click-------------')
        time.sleep(2)
        if (len(d(clickable = 'true')) == 0 and
            get_activity(package) == current_activity) : 
            visited[keys].add(key)
            continue
        if (get_activity(package) == activity) :
            return new_path
        if (get_activity(package) == current_activity) :
            new_elems = d(clickable = 'true')
            new_keys = get_info(new_elems)
            new_elems_num = new_elems.count
            if (new_keys in visited) : 
                visited[keys].add(key)
            if (keys == new_keys) :
                #print(2222)
                visited[keys].add(key)
                visited[pre_keys].add(pre_key)
                restart(package, history_path)
                return find_activity(package, activity, [], hash(""), None, history_path, visited)
            if (new_keys in visited and len(visited[new_keys]) == new_elems_num) :
                #print(111111)
                visited[keys].add(key)
                restart(package, history_path)
                return find_activity(package, activity, [], hash(""), None, history_path, visited)
            #print(33333333)
            return find_activity(package, activity, new_path, keys, key, history_path, visited)
        else :
            visited[keys].add(key)
            if (len(visited[keys]) == elems_num) :
                visited[pre_keys].add(pre_key)
            restart(package, history_path)
            return find_activity(package, activity, [], hash(""), None, history_path, visited)
    return new_path

def find_activities(package, acts) :
    if (get_activity(package) == acts[0]) :
        return find_activities(package, acts[1:])
    print(acts)
    path = []
    for act in acts :
        visited = {}
        new_path = (find_activity(package, act, [], hash(""), None, path, visited))
        path.extend(new_path)
    return path

def read_trace(filename) :
    f = open(filename, 'r')
    paths = []
    pattens = []
    path = []
    patten = []
    patten_flag = False
    trace_flag = False
    package = ""
    main_activity = ""
    for line in f.readlines() :
        if ('package' in line) : package = line.strip()[9:]
        if ('mainActivity' in line) : main_activity = line.strip()[14:]
        if ('Activity Trace Found:' in line) : 
            trace_flag = True
            continue
        if ('Patten Found:' in line) : 
            patten_flag = True
            continue
        if ('---Trace END---' in line) : 
            trace_flag = False
            if (len(path)) :
                paths.append(path)
                path = []
            continue
        if ('---Patten END---' in line) : 
            patten_flag = False
            if (len(patten)) :
                pattens.append(patten)
                patten = []
            continue
        if (trace_flag) :
            path.append(line.strip())
        if (patten_flag) :
            patten.append(line.strip())
    f.close()
    return (package, main_activity, paths, pattens)


if __name__ == '__main__' :
    apk = sys.argv[1]
    apk_name = os.path.split(apk)[1][:-4]
    (package, paths) = read_trace("result/"+apk_name+"/out.txt")
    os.system("python3 -m uiautomator2 init")
    d = u2.connect()
    for path in paths :
        os.system("adb install " + apk)
        time.sleep(10)
        d.app_start(package)
        click_path = find_activities(package, path)
        d.app_stop(package)
        os.system("adb uninstall " + package)
        time.sleep(5)
