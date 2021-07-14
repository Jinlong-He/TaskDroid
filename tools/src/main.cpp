#include <iostream>
#include <boost/program_options.hpp>
#include "AndroidStackMachine/AndroidStackMachine.hpp"
#include "Parser/ASMParser.hpp"
#include "Analyzer/MultiTaskAnalyzer.hpp"
#include "Analyzer/FragmentAnalyzer.hpp"
using namespace TaskDroid;
using std::cout, std::endl;
namespace po = boost::program_options;
int main (int argc, char* argv[]) {
    po::options_description opts("Allowed options");
    po::variables_map vm;
    int k = 0, h = 0;
    opts.add_options()
    ("help,h", "produce help message")
    ("verify,v", po::value<string>(), "")
    ("act", po::value<string>(), "")
    ("engine,e", po::value<string>()->default_value("nuxmv"), "")
    ("k", po::value<int>(&k)->default_value(10), "k")
    ("h", po::value<int>(&h)->default_value(3), "h")
    ("output-file,o", po::value<string>(), "")
    ("input-files,i", po::value<string>(), "input files")
    ("manifest-input-file,m", po::value<string>(), "manifest file")
    ("fragment-input-file,f", po::value<string>(), "fragment file")
    ("aftm-input-file,a", po::value<string>(), "activity fragment transition model file");
    string manifestFileName = "", aftmFileName = "", fragmentFileName = "",
           outputFileName = "out.txt";
    try {
        po::store(po::parse_command_line(argc, argv, opts), vm);
        po::notify(vm);
        if (vm.count("input-files")) {
            string dir = vm["input-files"].as<std::string>();
            manifestFileName = dir + "/AndroidManifest.txt";
            aftmFileName = dir + "/ictgMerge.xml";
            fragmentFileName = dir + "/SingleObject_entry.xml";
        } else {
            if (vm.count("manifest-input-file")) {
                manifestFileName = vm["manifest-input-file"].as<std::string>();
            } else {
                cout << "no manifest file!" << endl;;
                return 0;
            }
            if (vm.count("aftm-input-file")) {
                aftmFileName = vm["aftm-input-file"].as<std::string>();
            } else {
                cout << "no aftm file!" << endl;;
                return 0;
            }
            if (vm.count("fragment-input-file")) {
                aftmFileName = vm["fragment-input-file"].as<std::string>();
            } else {
                cout << "no fragment file!" << endl;;
                return 0;
            }
        }
        AndroidStackMachine a;
        ASMParser::parseManifest(manifestFileName.c_str(), &a);
        ASMParser::parseATG(aftmFileName.c_str(), &a);
        ASMParser::parseFragment(fragmentFileName.c_str(), &a);
        a.fomalize();
        if (vm.count("output-file")) {
            outputFileName = vm["output-file"].as<std::string>();
        }
        std::ofstream out(outputFileName);
        if (a.getAffinityMap().size() == 0 || !a.getMainActivity()) {
            out << "no graph" << endl;
            //cout << manifestFileName << endl;
            return 0;
        }
        a.print(out);
        if (a.getAffinityMap().size() > 1) cout << manifestFileName << endl;
        if (vm.count("verify")) {
            string verify = vm["verify"].as<string>();
            if (verify == "boundedness") {
                if (vm.count("engine")) {
                    string engine = vm["engine"].as<std::string>();
                    if (engine == "nuxmv") {
                        MultiTaskAnalyzer analyzer(k, &a);
                        analyzer.analyzeBoundedness(out);
                    }
                }
            } else if (verify == "backHijacking") {
                if (vm.count("engine")) {
                    string engine = vm["engine"].as<std::string>();
                    if (engine == "nuxmv") {
                        MultiTaskAnalyzer analyzer(k, &a);
                        analyzer.analyzeBackHijacking(out);
                    }
                }
            } else if (verify == "frag-boundedness") {
                string act = vm["act"].as<string>();
                cout << act << endl;
                if (act == "all") {
                    for (auto& [activity, transactions] : a.getActivityTransactionMap()) {
                        FragmentAnalyzer analyzer(k, h, &a, activity);
                        analyzer.analyzeBoundedness(out);
                    }
                } else {
                    auto activity = a.getActivity(act);
                    if (a.getActivityTransactionMap().count(activity)) {
                        FragmentAnalyzer analyzer(k, h, &a, activity);
                        analyzer.analyzeBoundedness(out);
                    }
                }
            }
        }
        out.close();
    } catch(string str) {
    }
    return 0;
}
