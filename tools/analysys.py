import os, re
import sys
import shutil


def analyze(path):
    apk_num = 0
    no_act_num = 0
    amm_num = 0

    act_num = 0
    frag_num = 0
    trans_num = 0
    frag_num = 0

    patten_num = 0
    trace_num = 0
    error_apk_num = 0

    patten_trace_nums = {}

    patten_re = '\[(.*)\].*'

    time = 0
    apks = os.listdir(path)
    for apk in apks:
        apk_num += 1
        files = os.listdir(os.path.join(path, apk))
        for file in files :
            f = open(os.path.join(path, apk, file), 'r', encoding='UTF-8')
            if 'time' in file :
                for line in f.readlines() :
                    time += float(line.strip())
                continue
            pflag = False
            tflag = False
            nflag = False
            actflag = False
            fragflag = False
            transflag = False
            flag = False
            lines = f.readlines()
            afts = set()
            if (len(lines) == 1) :
                no_act_num += 1
                nflag = True
                continue
            patten = ''
            for line in lines :
                if "no graph" in line :
                    no_act_num += 1
                    nflag = True
                    break
                if "-Activities:" in line :
                    actflag = True
                    continue
                if "-Activity Transition:" in line :
                    actflag = False
                    transflag = True
                    continue
                if "-Fragments: " in line :
                    transflag = False
                    fragflag = True
                    continue
                if 'Found' in line:
                    transflag = False
                if "-Fragment Transition:" in line :
                    transflag = True
                    fragflag = False
                    continue
                if fragflag :
                    frag_num += 1
                if actflag :
                    aft = line.split('(')[1].split(')')[0]
                    afts.add(aft)
                    act_num += 1
                if transflag :
                    trans_num += 1
                if "Patten Found" in line :
                    #if "REAL" in line :
                    #    print(apk)
                    patten_num += 1
                    #print(line)
                    patten = re.match(patten_re, line).group(1)
                    if patten in patten_trace_nums :
                        patten_trace_nums[patten][0] += 1
                    else :
                        patten_trace_nums[patten] = [1,0]
                if "Trace Found" in line :
                    if "REAL" in patten :
                        print(apk)
                    flag = True
                    trace_num += 1
                    patten_trace_nums[patten][1] += 1
            if (flag) :
                error_apk_num += 1
            if (nflag) :
                continue
            if len(afts) > 1 :
                amm_num += 1
    model_num = apk_num - no_act_num
    print('apk_num:\t%s\nmodel_num:\t%s\naverage_transitions:\t%s\naverage_acts:\t%s\naverage_frags:\t%s\ntime:\t%s\npatten_num:\t%s\ntrace_num:\t%s\nerror_apk_num:%s'%(str(apk_num),str(model_num),str(trans_num/model_num),str(act_num/model_num),str(frag_num/model_num),str(time/apk_num),str(patten_num), str(trace_num), str(error_apk_num)))
    print(patten_trace_nums)
def analyzeTrace(path):
    apk_num = 0
    patten_num = 0
    trace_num = 0
    pop_num = 0
    no_act = 0
    a2b_num = 0
    frag_num = 0
    amm_num = 0
    trans_num = 0
    act_num = 0
    frag_num = 0
    ubbp_new_num = 0
    ubbp_ctp_num = 0
    ubbp_ctk_num = 0
    p_ubbp_new_num = 0
    p_ubbp_ctp_num = 0
    p_ubbp_ctk_num = 0
    real_num = 0
    trans_max_num = 0
    act_max_num = 0
    frag_max_num = 0
    trans_max = ''
    act_max = ''
    frag_max = ''
    apks = os.listdir(path)
    time = 0
    for apk in apks:
        apk_num += 1
        files = os.listdir(os.path.join(path, apk))
        for file in files :
            f = open(os.path.join(path, apk, file), 'r', encoding='UTF-8')
            if 'time' in file :
                for line in f.readlines() :
                    time += float(line.strip())
                continue
            pflag = False
            tflag = False
            nflag = False
            a2bflag = False
            actflag = False
            fragflag = False
            transflag = False
            mtkflag = False
            lines = f.readlines()
            afts = set()
            if (len(lines) == 1) :
                no_act += 1
                nflag = True
                continue
            patten = ''
            this_trans_num = 0
            this_act_num = 0
            this_frag_num = 0
            for line in lines :
                if "no graph" in line :
                    no_act += 1
                    nflag = True
                    break
                if "-Activities:" in line :
                    actflag = True
                    continue
                if "-Activity Transition:" in line :
                    actflag = False
                    transflag = True
                    continue
                if "-Fragments: " in line :
                    transflag = False
                    fragflag = True
                    continue
                if 'Found' in line:
                    transflag = False
                if "-Fragment Transition:" in line :
                    transflag = True
                    fragflag = False
                    continue
                if fragflag :
                    frag_num += 1
                    this_frag_num += 1
                if actflag :
                    aft = line.split('(')[1].split(')')[0]
                    afts.add(aft)
                    this_act_num += 1
                    act_num += 1
                if transflag :
                    trans_num += 1
                    this_trans_num +=1
                if "New Patten" in line :
                    p_ubbp_new_num += 1
                if "CTP Patten" in line :
                    p_ubbp_ctp_num += 1
                if "CTK Patten" in line :
                    p_ubbp_ctk_num += 1
                    ubbp_ctk_num += 1
                if "Patten Found" in line :
                    pflag = True
                    patten = line
                if "Trace" in line :
                    tflag = True
                if "Coverage" in line and '/' in line and not '0/' in line :
                    real_num += 1
                    if 'New' in patten :
                        ubbp_new_num += 1
                    if 'CTP' in patten :
                        tflag = True
                        ubbp_ctp_num += 1
                    if 'CTK' in patten :
                        ubbp_ctk_num += 1
                if "addToBackStack" in line :
                    a2bflag = True
            if (nflag) :
                continue
            if (pflag) :
                patten_num += 1
            #if (pflag and not tflag) :
            #    print(apk)
            if (tflag) :
                trace_num += 1
            if (a2bflag) :
                a2b_num += 1
            if len(afts) > 1 :
                amm_num += 1
            if pflag :
                if this_trans_num > trans_max_num :
                    trans_max_num = this_trans_num
                    trans_max = apk
                if this_act_num > act_max_num :
                    act_max_num = this_act_num
                    act_max = apk
                if this_frag_num > frag_max_num :
                    frag_max_num = this_frag_num
                    frag_max = apk
    print('apk_num:\t%s\nmodel_num:\t%s\naverage_transitions:\t%s\nmax_transitions:\t%s\naverage_acts:\t%s\nmax_activities:\t%s\naverage_frags:\t%s\nmax_frags:\t%s\ntime:\t%s\npatten_num:\t%s\ntrace_num:\t%s\nubbp_new:\t%s\t%s\nubbp_ctp:\t%s\t%s\nubbp_ctk:\t%s\t%s\ntrace:\t%s'%(str(apk_num),str(apk_num-no_act),str(trans_num/(apk_num-no_act)),str(trans_max_num),str(act_num/(apk_num-no_act)),str(act_max_num),str(frag_num/(apk_num-no_act)),str(frag_max_num), str(time/apk_num),str(patten_num), str(trace_num),str(ubbp_new_num), str(p_ubbp_new_num), str(ubbp_ctp_num), str(p_ubbp_ctp_num), str(ubbp_ctk_num), str(p_ubbp_ctk_num),str(real_num)))
    print(trans_max, act_max, frag_max)
    #print(apk_num, a2b_num, amm_num, trans_num, patten_num, trace_num, no_act)

def analyzeTime(path) :
    time = 0
    num = 0
    apks = os.listdir(path)
    for apk in apks:
        file = os.path.join(path, apk, 'time.txt')
        if os.path.exists(file) :
            f = open(file)
            num += 1
            for line in f.readlines() :
                time += float(line.strip().split(': ')[1])
    print(num, time/num)

def analyzeManifest(path) :
    apks = os.listdir(path)
    apk_num = 0
    flag_rtf_num = 0
    flag_mtk_num = 0
    flag_rtf_ntk_num = 0
    lm_ctp_num = 0
    lm_stk_num = 0
    for apk in apks:
        apk_num += 1
        files = os.listdir(os.path.join(path, apk))
        for file in files :
            f = open(os.path.join(path, apk, file), 'r', encoding='UTF-8')
            for line in f.readlines() :
                if 'FLAG_ACTIVITY_REORDER_TO_FRONT' in line :
                    flag_rtf_num += 1
                if 'FLAG_ACTIVITY_MULTIPLE_TASK' in line :
                    flag_mtk_num += 1
                if 'FLAG_ACTIVITY_REORDER_TO_FRONT' in line and 'FLAG_ACTIVITY_NEW_TASK' in line:
                    flag_rtf_ntk_num += 1
                if '[1]' in line :
                    lm_ctp_num += 1
                if '[2]' in line :
                    lm_stk_num += 1
    print(flag_rtf_num, flag_mtk_num, flag_rtf_ntk_num, lm_ctp_num, lm_stk_num)



if __name__ == '__main__' :
    path = sys.argv[1]
    if len(sys.argv) == 2 :
        analyze(path)
    else :
        if (sys.argv[2] == 'trace') :
            analyzeTrace(path)
        elif (sys.argv[2] == 'time') :
            analyzeTime(path)
        elif (sys.argv[2] == 'mani') :
            analyzeManifest(path)
