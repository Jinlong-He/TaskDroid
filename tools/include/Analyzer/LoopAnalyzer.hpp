//
//  LoopAnalyzer.hpp
//  TaskDroid 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2021 Jinlong He.
//

#ifndef LoopAnalyzer_hpp 
#define LoopAnalyzer_hpp 

#include <queue>
#include "../AndroidStackMachine/AndroidStackMachine.hpp"
using std::queue;
namespace TaskDroid {
    template <class Symbol>
    class LoopAnalyzer {
    public:
        typedef unordered_map<Symbol*, unordered_map<Symbol*, int> > Graph;
        typedef vector<Symbol*> Path;
        typedef vector<Path> Paths;
        typedef unordered_map<Symbol*, Paths> PathsMap;
        static void getLoop(const Graph& graph, Symbol* init, vector<Path>& loops) {
            PathsMap paths({{init, Paths({Path({init})})}});
            unordered_map<Symbol*, int> dis({{init, 0}});
            queue<Symbol*> q({init});
            unordered_set<Symbol*> visited;
            while (!q.empty()) {
                auto source = q.front();
                q.pop();
                if (graph.count(source) == 0) return;
                for (auto& [target, weight] : graph.at(source)) {
                    if (visited.count(target) > 0) continue;
                    q.push(target);
                    if (dis.count(target) == 0) {
                        dis[target] = dis[source] + weight;
                        auto& ps = paths.at(source);
                        for (auto& p : ps) {
                            Path newPath = p;
                            newPath.push_back(target);
                            paths[target].push_back(newPath);
                        }
                    } else {
                        int w = dis.at(target);
                        if (dis.at(source) + weight > w) {
                            dis[target] = dis.at(source) + weight;
                            auto& ps = paths.at(source);
                            Paths newPs;
                            for (auto& p : ps) {
                                Path newPath = p;
                                newPath.push_back(target);
                                loops.push_back(newPath);
                                newPs.push_back(newPath);
                            }
                            paths[target] = newPs;
                            visited.insert(target);
                        } 
                    }
                }
            }
        }
    private:
    };
}
#endif /* LoopAnalyzer_hpp */
