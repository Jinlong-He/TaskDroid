#include "Parser/ASMParser.hpp"
#include "fml/util/util.hpp"
#include <iostream>
using std::cout, std::endl;
namespace TaskDroid {
    void ASMParser::parse(const char* fileName, AndroidStackMachine* a) {
        XMLDocument doc;
        doc.LoadFile(fileName);
        XMLElement* root = doc.RootElement();
        cout << root -> FirstChildElement() -> Value();
    }

    void ASMParser::parseManifest(const char* fileName, AndroidStackMachine* a) {
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
                        a -> addAction(sourceActivity, intent);
                        des = des -> NextSiblingElement();
                    }
                }
                element = element -> NextSiblingElement();
            }
        }
    }
}
