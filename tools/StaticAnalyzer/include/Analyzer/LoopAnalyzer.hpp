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
#include <iostream>
#include "../AndroidStackMachine/AndroidStackMachine.hpp"
using std::queue;
namespace TaskDroid {
    template <class Symbol>
    class LoopAnalyzer {
    public:
        typedef vector<Symbol*> Path;
        typedef vector<Path> Paths;
        typedef unordered_map<Symbol*, Paths> PathsMap;
        typedef unordered_map<Symbol*, unordered_map<Symbol*, int> > Graph;

        static void getPositiveLoop(const Graph& graph, Paths& loops) {
            Paths newLoops;
            getLoop(graph, newLoops);
            for (auto& loop : newLoops) {
                ID count = graph.at(loop[loop.size() - 1]).at(loop[0]);
                for (ID i = 0; i < loop.size() - 1; i++) 
                    count += graph.at(loop[i]).at(loop[i + 1]);
                if (count) loops.push_back(loop);
            }
        }

        static void getLoop(const Graph& graph, Paths& loops) {
            for (auto& [symbol, map] : graph) {
                Paths newLoops, newerLoops;
                getLoop(graph, symbol, newLoops);
                if (loops.size() == 0) loops = newLoops;
                bool flag = true;
                for (auto& newLoop : newLoops) {
                    for (auto& loop : loops) 
                        if (isSame(loop, newLoop)) flag = false;
                    if (flag) newerLoops.push_back(newLoop);
                }
                loops.insert(loops.end(), newerLoops.begin(), newerLoops.end());
            }
        }

        static bool isSame(Path& path1, Path& path2) {
            if (path1.size() != path2.size()) return false;
            for (ID i = 0; i < path1.size(); i++) {
                if (path1[i] == path2[0]) {
                    bool flag = true;
                    for (ID j = i; j < path1.size(); j++) {
                        if (path1[j] != path2[j - i]) {
                            flag = false;
                            break;
                        }
                    }
                    if (!flag) continue;
                    for (ID j = 0; j < i; j++) {
                        if (path1[j] != path2[j + path1.size() - i]) {
                            flag = false;
                            break;
                        }
                    }
                    if (flag) return true;
                }
            }
            return false;
        }

        static void getLoop(const Graph& graph, Symbol* init, Paths& loops) {
            queue<Symbol*> q({init});
            unordered_set<Symbol*> visited({init}), sources;
            PathsMap pathsMap({{init, Paths({Path({init})})}});
            while (!q.empty()) {
                auto source = q.front();
                q.pop();
                if (graph.count(source) == 0) continue;
                auto& paths = pathsMap.at(source);
                for (auto& [target, weight] : graph.at(source)) {
                    if (target == init) 
                        sources.insert(source);
                    if (!visited.insert(target).second) continue;
                    q.push(target);
                    auto& newPaths = pathsMap[target];
                    for (auto& path : paths) {
                        Path newPath = path;
                        newPath.push_back(target);
                        newPaths.push_back(newPath);
                    }
                }
            }
            for (auto source : sources) {
                auto& paths = pathsMap.at(source);
                loops.insert(loops.end(), paths.begin(), paths.end());
            }
        }

        static void getLoopWeight(const Graph& graph, Symbol* init, Paths& loops) {
            PathsMap paths({{init, Paths({Path({init})})}});
            unordered_map<Symbol*, int> dis({{init, 0}});
            queue<Symbol*> q({init});
            unordered_set<Symbol*> visited;
            while (!q.empty()) {
                auto source = q.front();
                q.pop();
                if (graph.count(source) == 0) continue;
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
