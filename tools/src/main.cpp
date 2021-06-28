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
    int k = 0;
    opts.add_options()
    ("help,h", "produce help message")
    ("verify,v", po::value<string>(), "")
    ("engine,e", po::value<string>(), "")
    ("k", po::value<int>(&k)->default_value(10), "k")
    ("output-file,o", po::value<string>(), "")
    ("manifest-input-file,m", po::value<string>(), "manifest file")
    ("aftm-input-file,a", po::value<string>(), "activity fragment transition model file");
    string manifestFileName = "", aftmFileName = "", outputFileName = "out.txt";
    try {
        po::store(po::parse_command_line(argc, argv, opts), vm);
        po::notify(vm);
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
        AndroidStackMachine a;
        ASMParser::parseManifest(manifestFileName.c_str(), &a);
        ASMParser::parseATG(aftmFileName.c_str(), &a);
        a.fomalize();
        if (vm.count("output-file")) {
            outputFileName = vm["output-file"].as<std::string>();
        }
        std::ofstream out(outputFileName);
        out << "package: " << a.getPackageName() << endl;
        if (vm.count("verify")) {
            string verify = vm["verify"].as<std::string>();
            if (verify == "boundedness") {
                if (vm.count("engine")) {
                    string engine = vm["engine"].as<std::string>();
                    if (engine == "nuxmv") {
                        MultiTaskAnalyzer analyzer(k);
                        analyzer.loadASM(&a);
                        analyzer.analyzeBoundedness(out);
                    }
                }
            }
        }
    } catch(string str) {
    }
    //AndroidStackMachine a;
    //ASMParser::parseManifest(argv[1], &a);
    //ASMParser::parseATG(argv[2], &a);
    //ASMParser::parseFragment(argv[3], &a);
    //a.fomalize();
    //std::ofstream out(argv[4]);
    //out << "package: " + a.getPackageName() << endl;
    //out << "mainActivity: " + a.getMainActivity() -> getName() << endl;

    //bool flag = true;
    //for (auto& [name, activity] : a.getActivityMap()) {
    //    if (a.getActivityTransactionMap().count(activity) != 0) {
    //        if (a.getFragmentTransactionMap(activity).size() > 0) {
    //            flag = false;
    //            //FragmentAnalyzer analyzer(2,3,&a,activity);
    //            //analyzer.analyzeBoundedness(out);
    //        }
    //    }
    //}
    //if (flag) {
    //    out << "No Fragments" << endl;
    //}
    //if (!flag) {
    //    out << "Has Fragments" << endl;
    //}
    //ASMParser::parseATG(argv[2], &a);
    //MultiTaskAnalyzer::analyzeReachability(&a, 5);
    //MultiTaskAnalyzer analyzer(6);
    //analyzer.loadASM(&a);
    //analyzer.analyzeBoundedness();
    return 0;
}
