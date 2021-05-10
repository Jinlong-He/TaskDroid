#include "Parser/ASMParser.hpp"
#include "util/util.hpp"
#include <iostream>
#include <regex>
using std::cout, std::endl;
namespace TaskDroid {
    void ASMParser::parse(const char* fileName, AndroidStackMachine* a) {
        XMLDocument doc;
        doc.LoadFile(fileName);
        XMLElement* root = doc.RootElement();
        cout << root -> FirstChildElement() -> Value();
    }

    void ASMParser::parseManifestTxt(const char* fileName, AndroidStackMachine* a) {
        std::ifstream fin(fileName);
        string line;
        while (getline(fin, line)) {
            if (line.find("activity")) {
                string name, launchModeStr;
                LaunchMode launchMode;
                getline(fin, name);
                getline(fin, launchModeStr);
                if (launchModeStr == "2130968577") launchMode = STD;
            }
        }
    }

    void ASMParser::parseManifest(const char* fileName, AndroidStackMachine* a) {
        string name = fileName;
        if (name.find(".xml", name.length() - 5) == string::npos) {
            parseManifestTxt(fileName, a);
            return;
        }
        XMLDocument doc;
        doc.LoadFile(fileName);
        XMLElement* root = doc.RootElement();
        bool main = false;
        if (!root -> FirstChildElement()) return;
        for (auto appElement = root -> FirstChildElement(); appElement; 
             appElement = appElement -> NextSiblingElement()) {
            if (strcmp(appElement -> Name(), "application") != 0) continue;
            if (!appElement -> FirstChildElement()) continue;
            for (auto actElement = appElement -> FirstChildElement(); actElement; 
                 actElement = actElement -> NextSiblingElement()) {
                if (strcmp(actElement -> Name(), "activity") != 0) continue;
                string name = actElement -> Attribute("android:name");
                LaunchMode launchMode = STD;
                string affinity = "";
                if (actElement -> FindAttribute("android:launchMode")) {
                    string lm = actElement -> Attribute("android:launchMode");
                    if (lm == "singleTask") {
                        launchMode = STK;
                    } else if (lm == "singleTop") {
                        launchMode = STP;
                    } else if (lm == "singleInstance") {
                        launchMode = SIT;
                    } else {
                        launchMode = STD;
                    }
                }
                if (actElement -> FindAttribute("android:taskAffinity")) {
                    affinity = actElement -> Attribute("android:taskAffinity");
                }
                auto activity = a -> mkActivity(name, affinity, launchMode);
                if (main) continue;
                if (!actElement -> FirstChildElement()) continue;
                
                for (auto intentElement = actElement -> FirstChildElement(); 
                     intentElement;
                     intentElement = intentElement -> NextSiblingElement()) {
                    if (!intentElement -> FirstChildElement()) continue;
                    for (auto actionElement = intentElement -> FirstChildElement();
                         actionElement;
                         actionElement = actionElement -> NextSiblingElement()) {
                        if (actionElement -> FindAttribute("android:name") &&
                            strcmp(actionElement -> Attribute("android:name"), 
                                   "android.intent.action.MAIN") == 0) {
                            a -> setMainActivity(activity);
                            activity -> setAffinity(activity -> getName());
                            main = true;
                        }
                    }
                }
            }
        }
    }

    void ASMParser::parseATG(const char* fileName, AndroidStackMachine* a) {
        XMLDocument doc;
        doc.LoadFile(fileName);
        XMLElement* root = doc.RootElement();
        if (root -> FirstChildElement()) {
            auto element = root -> FirstChildElement();
            while (element) {
                string sourceName = element -> Attribute("name");
                auto sourceActivity = a -> getActivity(sourceName);
                if (!sourceActivity) {
                    element = element -> NextSiblingElement();
                    continue;
                }
                if (element -> FirstChildElement()) {
                    auto des = element -> FirstChildElement();
                    while (des) {
                        string targetName = des -> Attribute("name");
                        auto targetActivity = a -> getActivity(targetName);
                        if (!targetActivity) {
                            des = des -> NextSiblingElement();
                            continue;
                        }
                        auto intent = a -> mkIntent(targetActivity);
                        if (des -> FindAttribute("flags")) {
                            string flags = des -> Attribute("flags");
                            for (auto& flag : util::split(flags, " ")) {
                                intent -> addFlag(flag);
                            }
                        }
                        if (des -> FindAttribute("finish")) {
                            string finish = des -> Attribute("finish");
                            if (finish == "true") {
                                a -> addAction(sourceActivity, intent, true);
                            } else {
                                a -> addAction(sourceActivity, intent, false);
                            }
                        } else {
                            a -> addAction(sourceActivity, intent, false);
                        }
                        des = des -> NextSiblingElement();
                    }
                }
                element = element -> NextSiblingElement();
            }
        }
    }

    void ASMParser::parseFragment(const char* fileName, AndroidStackMachine* a) {
        XMLDocument doc;
        doc.LoadFile(fileName);
        XMLElement* root = doc.RootElement();
        if (root -> FirstChildElement()) {
            auto singleMethod = root -> FirstChildElement();
            while (singleMethod) {
                if (!singleMethod -> FirstChildElement()) {
                    singleMethod = singleMethod -> NextSiblingElement();
                    continue;
                }
                string source = singleMethod -> Attribute("source");
                string name = source.substr(1,source.find(":") - 1);
                if (name.find("Fragment") != string::npos) a -> mkFragment(name);
                auto singleFragment = singleMethod -> FirstChildElement();
                while (singleFragment) {
                    if (!singleFragment -> FirstChildElement()) {
                        singleFragment = singleFragment -> NextSiblingElement();
                        continue;
                    }
                    auto element = singleFragment -> FirstChildElement();
                    while (element) {
                        string list = element -> Name();
                        if (list == "setDestinationList") {
                            string names = element -> Attribute("value");
                            for (auto& name : util::split(names, ", ")) {
                                a -> mkFragment(name);
                            }
                        }
                        element = element -> NextSiblingElement();
                    }
                    singleFragment = singleFragment -> NextSiblingElement();
                }
                singleMethod = singleMethod -> NextSiblingElement();
            }
        }

        root = doc.RootElement();
        std::regex addPatten("(add|replace)\\(.*\\)>\\([0-9]+");
        std::regex a2bPatten("(addToBackStack)\\(.*\\)");
        if (root -> FirstChildElement()) {
            auto singleMethod = root -> FirstChildElement();
            while (singleMethod) {
                if (!singleMethod -> FirstChildElement()) {
                    singleMethod = singleMethod -> NextSiblingElement();
                    continue;
                }
                string source = singleMethod -> Attribute("source");
                string name = source.substr(1,source.find(":") - 1);
                auto activity = a -> getActivity(name);
                auto fragment = a -> getFragment(name);
                auto singleFragment = singleMethod -> FirstChildElement();
                while (singleFragment) {
                    if (!singleFragment -> FirstChildElement()) {
                        singleFragment = singleFragment -> NextSiblingElement();
                        continue;
                    }
                    auto element = singleFragment -> FirstChildElement();
                    vector<string> nameVec;
                    while (element) {
                        string list = element -> Name();
                        auto transaction = a -> mkFragmentTransaction();
                        if (list == "setDestinationList") {
                            string names = element -> Attribute("value");
                            nameVec = util::split(names, ", ");
                        } else if (list == "flow") {
                            if (!element -> FirstChildElement()) {
                                element = element -> NextSiblingElement();
                                continue;
                            }
                            auto data = element -> FirstChildElement();
                            ID index = 0;
                            while (data) {
                                if (((string) data -> Name()) == "dataHandleList" ) {
                                    string value = data -> Attribute("value");
                                    std::smatch m;
                                    if (std::regex_search(value, m, addPatten)) {
                                        string result = m[0];
                                        string viewId = result.substr(result.find(">(")+2);
                                        auto mode = 
                                            result.find("add") != string::npos ?
                                            ADD : REP;
                                        auto newFragment = a -> getFragment(
                                                nameVec[index++]);
                                        transaction -> addFragmentAction(
                                                mode, newFragment, viewId);
                                    } else if (std::regex_search(value, m, a2bPatten)) {
                                        transaction -> setAddTobackStack(1);
                                    }
                                        
                                }
                                data = data -> NextSiblingElement();
                            }
                            if (fragment && 
                                transaction -> getFragmentActions().size() > 0) {
                                a -> addFragmentTransaction(fragment, transaction);
                            }
                            if (activity && 
                                transaction -> getFragmentActions().size() > 0) {
                                a -> addFragmentTransaction(activity, transaction);
                            }
                        }
                        element = element -> NextSiblingElement();
                    }
                    singleFragment = singleFragment -> NextSiblingElement();
                }
                singleMethod = singleMethod -> NextSiblingElement();
            }
        }
    }

    void ASMParser::parseFragmentConfig(const char* fileName,
                                        AndroidStackMachine* a,
                                        Configuration<Fragment>* config) {
        XMLDocument doc;
        doc.LoadFile(fileName);
        XMLElement* root = doc.RootElement();
        if (!root -> FirstChildElement()) return;
        auto viewElement = root -> FirstChildElement();
        while (viewElement) {
            string viewID = viewElement -> Attribute("viewID");
            if (viewElement -> FirstChildElement()) {
                auto fragmentElement = viewElement -> FirstChildElement();
                vector<Fragment*> task;
                while (fragmentElement) {
                    string fragmentName = fragmentElement -> Attribute("name");
                    auto fragment = a -> getFragment(fragmentName);
                    task.emplace_back(fragment);
                    fragmentElement = fragmentElement -> NextSiblingElement();
                }
                config -> addTask(viewID, task);
            }
            viewElement = viewElement -> NextSiblingElement();
        }
    }
}
