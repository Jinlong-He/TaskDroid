import uiautomator2 as u2
import time
import os

class activity_finder :
    def __init__(self, package, apk, main_activity, device, init_activity, path_dic, sleep_time = 2) :
        self.package = package
        self.apk = apk 
        self.main_activity = main_activity
        self.device = device
        self.init_activity = init_activity 
        self.path_dic = path_dic
        self.sleep_time = sleep_time 

    def get_activity(self) :
        name = self.device.app_current()['activity']
        if (self.package in name) :
            return name
        elif ('.' == name[0]) :
            return self.package + name
        else :
            return self.package + '.' + name

    def is_same_activity(self, activity1, activity2) :
        if (self.package in activity1 and self.package in activity2) :
            return activity1 == activity2
        return activity1.split(".")[-1] == activity2.split(".")[-1]
    
    def is_current_activity(self, activity) :
        return self.is_same_activity(self.get_activity(), activity)

    def update(self, source, target, path) :
        if (source, target) in self.path_dic.keys() and len(self.path_dic[(source, target)]) > len(path) or (source, target) not in self.path_dic.keys() :
                self.path_dic[(source, target)] = path
                for (s, t), p in list(self.path_dic.items()) :
                    if source == t :
                        self.update(s, target, p + path)
                    if target == s :
                        self.update(source, t, path + p)


    def get_path(self, source, target) :
        if (source, target) in self.path_dic.keys() :
            return self.path_dic[(source, target)]
        return []

    def get_trace_path(self, trace) :
        path = []
        for i in range(len(trace)) :
            if (i + 1 < len(trace)) :
                path += self.get_path(trace[i], trace[i+1])
        return path

    def goto_activity(self, info_path, target, history_path = [], source = "") :
        if (source == "") : source = self.init_activity
        if (self.is_current_activity(source)) :
            for info in info_path :
                for elem in self.device(clickable = 'true') :
                    if (info == str(elem.info)) :
                        elem.click()
                        time.sleep(self.sleep_time)
                        break
        elif (self.is_current_activity(target)) :
            return
        else :
            current_activity = self.get_activity()
            path = self.get_path(current_activity, target)
            if (len(path)) :
                self.goto_activity(path, target, [], current_activity)
                return
            path = self.get_path(current_activity, source)
            if (len(path)) :
                self.goto_activity(path, source, [], current_activity)
                self.goto_activity(info_path, target, [], source)
                return
            self.restart(reinstall=True)
            self.goto_activity(history_path, source, [], self.init_activity)
            self.goto_activity(info_path, target, history_path, source)

    def get_info(self, elems) :
        res = ''
        for elem in elems :
            res += str(elem.info) + ','
        return res 

    def restart(self, reinstall = False) :
        self.device.app_stop(self.package)
        if (reinstall) :
            os.system("adb uninstall " + self.package)
            os.system("adb install " + self.apk)
            time.sleep(self.sleep_time)
        self.device.app_start(self.package)
        time.sleep(self.sleep_time)

    def find_activity(self, activity, path, init_keys, pre_keys, pre_key, history_path, visited) :
        time.sleep(self.sleep_time)
        current_activity = self.get_activity()
        elems = self.device(clickable = 'true')
        keys = self.get_info(elems)
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
            time.sleep(self.sleep_time)
            new_activity = self.get_activity()
            if (len(self.device(clickable = 'true')) == 0 and
                self.is_current_activity(current_activity)) :
                visited[keys].add(key)
                new_path.pop()
                continue
            if (self.is_current_activity(activity)) :
                return new_path
            if (self.is_current_activity(current_activity)) :
                new_elems = self.device(clickable = 'true')
                new_keys = self.get_info(new_elems)
                new_elems_num = new_elems.count
                if (new_keys == init_keys) :
                    visited[keys].add(key)
                    return self.find_activity(activity, [], init_keys, hash(""), None, history_path, visited)

                if (keys == new_keys) :
                    visited[keys].add(key)
                    #visited[pre_keys].add(pre_key)
                    new_path.pop()
                    continue
                if (new_keys in visited) : 
                    visited[keys].add(key)
                    if (len(visited[new_keys]) == new_elems_num) :
                        visited[keys].add(key)
                        self.device.press("back")
                        time.sleep(self.sleep_time)
                        newer_elems = self.device(clickable = 'true')
                        newer_keys = self.get_info(new_elems)
                        if newer_keys == keys :
                            new_path.pop()
                            continue
                        else :
                            self.restart()
                            self.goto_activity(history_path, current_activity)
                            return self.find_activity(activity, [], init_keys, hash(""), None, history_path, visited)
                return self.find_activity(activity, new_path, init_keys, keys, key, history_path, visited)
            else :
                visited[keys].add(key)
                if (len(visited[keys]) == elems_num) :
                    visited[pre_keys].add(pre_key)
                self.update(current_activity, new_activity, new_path)
                self.device.press("back")
                time.sleep(self.sleep_time)
                new_elems = self.device(clickable = 'true')
                new_keys = self.get_info(new_elems)
                if new_keys == keys :
                    continue
                else :
                    if (self.is_current_activity(current_activity)) :
                        return self.find_activity(activity, [], init_keys, hash(""), None, history_path, visited)
                    else :
                        self.restart()
                        self.goto_activity(history_path, current_activity)
                        return self.find_activity(activity, [], init_keys, hash(""), None, history_path, visited)
        return new_path

    def find(self, trace) :
        self.trace = trace
        new_trace = trace
        path = []
        if (self.is_current_activity(trace[0])) :
            new_trace = trace[1:]
        source = self.get_activity()
        for target in new_trace :
            new_path = []
            visited = {}
            if ((source, target) in self.path_dic) :
                new_path = self.path_dic[(source, target)]
                self.goto_activity(new_path, path, source)
            else :
                new_path = self.find_activity(target, [], self.get_info(self.device(clickable = 'true')), hash(""), None, path, visited)
                self.update(source, target, new_path)
            source = self.get_activity()
            path.extend(new_path)
        return (path, self.path_dic)

    def execute(self, exec_path, path, loop=[], num=0) :
        if (num == 0) :
            return self.goto_activity(exec_path, path[-1], [], self.init_activity)
        pre_path = []
        for i in range(0,len(path)-len(loop)+1) :
            flag = True
            for j in range(len(loop)) :
                if path[i+j] != loop[j] :
                    flag = False
                    break
            if (flag) :
                pre_path = path[0:i+len(loop)]
                break
        pre_path.append(loop[0])
        loop.append(loop[0])
        pre_exec_path = self.get_trace_path(pre_path)
        self.goto_activity(pre_exec_path, pre_path[-1], [], self.init_activity)
        loop_exec_path = self.get_trace_path(loop)
        for i in range(num) :
            self.goto_activity(loop_exec_path, loop[0], pre_exec_path, pre_path[0])


class trace_reader() :
    def __init__(self, filename) :
        self.filename = filename

    def read(self) :
        f = open(self.filename, 'r')
        package = ""
        main_activity = ""
        patten = []
        pattens = []
        patten_flag = False
        trace = []
        traces = []
        trace_flag = False
        for line in f.readlines() :
            if ('-Package:' in line) : package = line.strip()[10:]
            if ('-Main Activity:' in line) : main_activity = line.strip()[16:]
            if ('Patten Found:' in line) : 
                patten_flag = True
                continue
            if ('Activity Trace Found:' in line) : 
                trace_flag = True
                continue
            if ('---Patten END---' in line) : 
                patten_flag = False
                if (len(patten)) :
                    pattens.append(patten)
                    patten = []
                continue
            if ('---Trace END---' in line) : 
                trace_flag = False
                if (len(trace)) :
                    traces.append(trace)
                    trace = []
                continue
            if (trace_flag) :
                trace.append(line.strip())
            if (patten_flag) :
                patten.append(line.strip())
        f.close()
        return (package, main_activity, pattens, traces)

