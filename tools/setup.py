import os, sys
platform = sys.platform
timeout_cmd = ''
if platform == 'darwin' :
    timeout_cmd = 'gtimeout'
else :
    timeout_cmd = 'timeout'

def check_cmd(cmd) :
    for cmdpath in os.environ['PATH'].split(':'):
        if os.path.isdir(cmdpath) and cmd in os.listdir(cmdpath):
            return 1
    return 0

def search_install_cmd() :
    if check_cmd('brew') :
        return 'brew'
    if check_cmd('apt-get') :
        return 'apt-get'
    return ''

install_cmd = search_install_cmd()

def check_boost() :
    print('-- Check for boost........', end='')
    boost_dir = ''
    if os.path.exists('/opt/homebrew/include/boost') :
        boost_dir = '/opt/homebrew/include'
    if os.path.exists('/usr/local/include/boost') :
        boost_dir = '/usr/local/include'
    if os.path.exists('/usr/include/boost') :
        boost_dir = '/usr/include'
    if boost_dir != '' :
        print('\033[0;32;40mOK!\033[0m')
        return 1
    else :
        print('\033[0;31;40mError!\033[0m')
        return 0 

def install_boost() :
    print('-- Installing for boost......')
    os.system('%s install boost'%(install_cmd))
    return check_boost()

def check_nuxmv() :
    print('-- Check for nuXmv........', end='')
    if check_cmd('nuXmv1') :
        print('\033[0;32;40mOK!\033[0m')
        return 1
    else :
        print('\033[0;31;40mError!\033[0m')
        return 0 

def install_nuxmv() :
    print('-- Installing for nuXmv......')
    os.system('%s 30 git clone https://github.com/Jinlong-He/nuXmv.git'%(timeout_cmd))
    print('-- Install for nuXmv......', end = '')
    if os.path.exists('./nuXmv') :
        print('\033[0;32;40mOK!\033[0m')
    else :
        print('\033[0;31;40mError!\033[0m')
        return 0 
    if platform == 'darwin' :
        os.system('mv nuXmv/bin/nuXmv-Darwin ./ && rm -rf nuXmv && mv nuXmv-Darwin nuXmv')
    else :
        os.system('mv nuXmv/bin/nuXmv-Linux64 nuxmv && rm -rf nuXmv')
    return 1

def check_cmake() :
    print('-- Check for cmake........', end='')
    if check_cmd('cmake') :
        print('\033[0;32;40mOK!\033[0m')
        return 1
    else :
        print('\033[0;31;40mError!\033[0m')
        return 0

def install_cmake() :
    print('-- Installing for cmake......')
    os.system('%s install cmake'%(install_cmd))
    return check_cmake()

def check_make() :
    print('-- Check for make.........', end='')
    if check_cmd('make') :
        print('\033[0;32;40mOK!\033[0m')
        return 1
    else :
        print('\033[0;31;40mError!\033[0m')
        return 0

def install_make() :
    print('-- Installing for make......')
    os.system('%s install make'%(install_cmd))
    return check_make()

def config_nuxmv(f) :
    if not check_cmd('nuXmv1') :
        f.write('./nuXmv')

def configure() :
    if check() :
        f = open('.conf','w')
        print('-- Configuring............', end='')
        config_nuxmv(f)
        print('\033[0;32;40mOK!\033[0m')
        f.close()
    else :
        print('-- Configure..............', end='')
        print('\033[0;31;40mError!\033[0m')


def check() :
    if not check_make() :
        if not install_make() :
            return 0
    if not check_cmake() :
        if not install_cmake() :
            return 0
    if not check_boost() :
        if not install_boost() :
            return 0
    if not check_nuxmv() :
        if not install_nuxmv() :
            return 0
    return 1

def build() :
    if os.path.exists('StaticAnalyzer/build') :
        os.system('rm -r StaticAnalyzer/build')
    os.system('cd StaticAnalyzer && mkdir build && cd build && cmake .. && make -j')

if len(sys.argv) < 2 :
    print('\033[0;31;40mError! Use \'python3 setup.py config\'\033[0m')
elif len(sys.argv) > 2 :
    print('\033[0;31;40mError! Too many arguments\033[0m')
else :
    if sys.argv[1] == 'config' :
        configure()
    elif sys.argv[1] == 'build' :
        if os.path.exists('./.conf') :
            build()
        else :
            print('\033[0;31;40mPlease use \'python3 setup.py config\' first\033[0m')

    else :
        print('\033[0;31;40mError! \'Use python3 setup.py config\'\033[0m')

