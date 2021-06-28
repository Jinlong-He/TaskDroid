import uiautomator2 as u2
import time

def get_activity() :
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

def restart(path) :
    d.app_stop(package)
    d.app_start(package)
    time.sleep(2)
    goto_activity(path)

def find_activity(activity, path, pre_keys, pre_key, history_path, visited) :
    time.sleep(2)
    current_activity = get_activity()
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
            get_activity() == current_activity) : 
            visited[keys].add(key)
            continue
        if (get_activity() == activity) :
            return new_path
        if (get_activity() == current_activity) :
            new_elems = d(clickable = 'true')
            new_keys = get_info(new_elems)
            new_elems_num = new_elems.count
            if (keys == new_keys) :
                #print(2222)
                visited[keys].add(key)
                visited[pre_keys].add(pre_key)
                restart(history_path)
                return find_activity(activity, [], hash(""), None, history_path, visited)
            if (new_keys in visited and len(visited[new_keys]) == new_elems_num) :
                #print(111111)
                visited[keys].add(key)
                restart(history_path)
                return find_activity(activity, [], hash(""), None, history_path, visited)
            #print(33333333)
            return find_activity(activity, new_path, keys, key, history_path, visited)
        else :
            visited[keys].add(key)
            if (len(visited[keys]) == elems_num) :
                visited[pre_keys].add(pre_key)
            restart(history_path)
            return find_activity(activity, [], hash(""), None, history_path, visited)
    return new_path

def find_activities(acts) :
    path = []
    for act in acts :
        visited = {}
        new_path = (find_activity(act, [], hash(""), None, path, visited))
        path.extend(new_path)
    return path

package = 'uk.co.busydoingnothing.prevo'
d = u2.connect()
d.app_start(package)
#path = find_activities(['.ArticleActivity'])
path = find_activities(['.ArticleActivity', '.SelectLanguageActivity', '.SearchActivity'])
#path = find_activities(['.SelectLanguageActivity', '.SearchActivity'])
print(path)
d.app_stop(package)
d.app_start(package)
time.sleep(2)
goto_activity(path)
time.sleep(5)
d.app_stop("uk.co.busydoingnothing.prevo")
