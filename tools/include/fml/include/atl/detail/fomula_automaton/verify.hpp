//
//  verify.hpp
//  atl 
//
//  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//  SPDX-License-Identifier: MIT
//  Copyright (c) 2020 Jinlong He.
//

#ifndef atl_detail_fomula_automaton_verify_hpp 
#define atl_detail_fomula_automaton_verify_hpp 

#include <iostream>
#include <fstream>
#include <atl/detail/fomula_automaton/fomula_automaton.hpp>
using std::endl;

namespace atl::detail {
    struct verify_invar_nuxmv_impl {
        template <FOA_PARAMS>
        static void 
        apply(const FOA& foa,
              const ll::atomic_proposition& p,
              const string& source_file) {
            std::ofstream out("out.smv");
            out << "MODULE main" << endl;
            translate_nuxmv(foa, out);
            out << "INVARSPEC ! (" + p.to_string() + ")" << endl;
            out.close();
            string commond = "nuXmv -source " + source_file + " out.smv";
            system(commond.c_str());
        }
    };
};

namespace atl {
    template <FOA_PARAMS>
    inline void 
    verify_invar_nuxmv(const FOA& foa,
                       const ll::atomic_proposition& p,
                       const string& source_file) {
        return detail::verify_invar_nuxmv_impl::apply(foa, p, source_file);
    }
}

#endif /* atl_detail_fomula_automaton_verify_hpp */
