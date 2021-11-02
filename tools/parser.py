import os, sys

class static_parser :
    def __init__(self) :
        self.path_dict = {}

    def parse_gator(self, info) :
        out = open(os.path.join(info, 'gator_dict.txt'), 'w')
        transitions = ""
        for path_file in os.listdir(info) :
            if 'path_' in path_file :
                source = path_file.split('path_')[1].split('#')[0]
                target = path_file.split('path_')[1].split('#')[1].split('.txt')[0]
                f = open(os.path.join(info, path_file))
                flag = False
                path_set = set()
                for line in f.readlines() :
                    if '========  all analysis finished ========' in line :
                        flag = True
                        continue
                    if flag :
                        if source in line and line not in path_set :
                            path_set.add(line)
                f.close()
                exec_paths = []
                for path in path_set :
                    views = path.strip().split(' ===> ')
                    exec_paths.append(views)
                if len(exec_paths) > 0 :
                    self.path_dict[(source, target)] = exec_paths
                    transitions += source + " -> " + target + '\n'
                    for path in exec_paths :
                        transitions += str(path) + '\n'
                    transitions += '==END==\n'
        out.write(transitions)
        out.close()



    def parse_static_path(file) :
        f = open(file)
        flag = False
        paths = []
        path = []
        for line in f.readlines() :
            if '-Activity Trace Found' in line :
                path = []
                flag = True
                continue
            if '---Trace END---' in line :
                paths.append(path)
                flag = False
                continue
            if flag :
                if 'back' == line.strip() :
                    path.append('back')
                else :
                    source = line.strip().split(' -> ')[0]
                    target = line.strip().split(' -> ')[1]
                    path.append(self.path_dict[(source, target)])
        f.close()

parser = static_parser()
parser.parse_gator(sys.argv[1])
