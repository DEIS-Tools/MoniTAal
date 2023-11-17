/*
 * Copyright Thomas M. Grosen 
 * Created on 10/06/2022
 */

/*
 * This file is part of monitaal
 *
 * monitaal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * monitaal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with monitaal. If not, see <https://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE MONITAAL

#include "monitaal/Monitor.h"
#include "monitaal/Parser.h"

#include <boost/test/unit_test.hpp>
#include <filesystem>

using namespace monitaal;

BOOST_AUTO_TEST_CASE(time_div_print) {
    auto div = TA::time_divergence_ta({"a", "b", "c"}, false);

    //std::cout << div;
}

BOOST_AUTO_TEST_CASE(monitor_test1) {
    TA pos = Parser::parse("models/a-b.xml", "a_leadsto_b");
    TA neg = Parser::parse("models/a-b.xml", "not_a_leadsto_b");

    std::vector<concrete_input> word1 = {
            concrete_input(0, "c"),
            concrete_input(2.5, "b"),
            concrete_input(100, "b"),
            concrete_input(2.1, "a"),
            concrete_input(0, "c"),
            concrete_input(5, "a"),
            concrete_input(10, "b"),
            concrete_input(0, "c"),
            concrete_input(0, "c")};

    std::vector<concrete_input> word2 = {
            concrete_input(0, "a"),
            concrete_input(101, "c")};

    Concrete_monitor monitor(pos, neg);

    BOOST_CHECK(monitor.input(word1) == INCONCLUSIVE);
    BOOST_CHECK(monitor.input(word2) == NEGATIVE);
}

BOOST_AUTO_TEST_CASE(intersection_test_1) {
    TA T1 = Parser::parse("models/small1.xml", "small1");
    TA T2 = Parser::parse("models/small2.xml", "small2");

    //std::cout << T1 << T2;

    T1.intersection(T2);

    //std::cout << T1;
}

BOOST_AUTO_TEST_CASE(time_converge_test_1) {
    TA converge_ta = Parser::parse("models/time_converge.xml", "time_convergence");

    auto diverge_ta = TA::time_divergence_ta({"a"}, false);

    converge_ta.intersection(diverge_ta);

    auto non_empty_states = Fixpoint::buchi_accept_fixpoint(converge_ta);

    BOOST_CHECK(non_empty_states.is_empty());
}