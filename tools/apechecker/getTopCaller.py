import sys

#input_method = sys.argv[1]
#cg_file = sys.argv[2]
# input_method = 'jp.yhonda.MOAInstallerActivity: void install(int)'
# cg_file = '/home/fanlingling/MaximaOnAndroidActivity-debug_cg.txt'
def getCallerClass(input_method, dict):
    # find top caller
    caller_m = input_method
    for k,v in dict.items():
        if caller_m in v:
            caller_m = k
            caller_c = getCallerClass(caller_m, dict)
            break
    if '$' in caller_m:
        caller_c = caller_m.split('$')[0]
    else:
        caller_c = caller_m.split(':')[0]
    #print caller
    return caller_c

def getCallerActivity(input_method, dict):

    caller_m = input_method
    for k,v in dict.items():
        if caller_m in v:
            caller_m = k
            if (("fragment" in getClass(caller_m).lower()) or ("activity" in getClass(caller_m).lower())):
                caller_c = getClass(caller_m)
                return caller_c, caller_m
            else:
                caller_c, caller_m = getCallerActivity(caller_m, dict)
            break
    caller_c = getClass(caller_m)
    return caller_c, caller_m
def getClass(caller_m):
    if '$' in caller_m:
        caller_c = caller_m.split('$')[0]
    else:
        caller_c = caller_m.split(':')[0]
    return caller_c
# if __name__ == '__main__':
#     getCallerClass('jp.yhonda.MOAInstallerActivity: void install(int)', '/home/fanlingling/MaximaOnAndroidActivity-debug_cg.txt')
