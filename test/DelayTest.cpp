/*
 * Copyright Thomas M. Grosen 
 * Created on 11/01/2024
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
#include <sstream>
#include <vector>

using namespace monitaal;

BOOST_AUTO_TEST_CASE(delay_test1) {
    TA pos = Parser::parse("models/a-b.xml", "a_leadsto_b");
    TA neg = Parser::parse("models/a-b.xml", "not_a_leadsto_b");
    settings_t setting{false, false, {0, 100}, 0};

    Monitor<delay_state_t> monitor(pos, neg, setting);

    std::vector<timed_input_t> word1 {
        timed_input_t(0, "c"),
        timed_input_t(2, "b"),
        timed_input_t(102, "b"),
        timed_input_t(104, "a"),
        timed_input_t(104, "c"),
        timed_input_t(109, "a"),
        timed_input_t(119, "b"),
        timed_input_t(119, "c"),
        timed_input_t(119, "c")
    };

    std::vector<timed_input_t> word2 {
        timed_input_t(120, "a"),
        timed_input_t(151, "c")
    };

    monitor.input(word1);
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);

    monitor.input(word2);
    BOOST_CHECK(monitor.status() == NEGATIVE);
}

BOOST_AUTO_TEST_CASE(delay_test2) {
    TA pos = Parser::parse("models/a-b.xml", "a_leadsto_b");
    TA neg = Parser::parse("models/a-b.xml", "not_a_leadsto_b");
    settings_t setting{false, false, {0, 100}, 1};

    Monitor<delay_state_t> monitor(pos, neg, setting);

    std::vector<timed_input_t> word1 {
        timed_input_t(0, "c"),
        timed_input_t(2, "b"),
        timed_input_t(102, "b"),
        timed_input_t(104, "a"),
        timed_input_t(104, "c"),
        timed_input_t(109, "a"),
        timed_input_t(119, "b"),
        timed_input_t(119, "c"),
        timed_input_t(119, "c")
    };

    std::vector<timed_input_t> word2 {
        timed_input_t(120, "a"),
        timed_input_t(151, "c")
    };

    std::vector<timed_input_t> word3 {
        timed_input_t(160, "a"),
        timed_input_t(192, "c")
    };

    monitor.input(word1);
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);

    monitor.input(word2);
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);

    monitor.input(word3);
    BOOST_CHECK(monitor.status() == NEGATIVE);
}

BOOST_AUTO_TEST_CASE(delay_test3) {
    TA pos = Parser::parse("models/a-b.xml", "a_leadsto_b");
    TA neg = Parser::parse("models/a-b.xml", "not_a_leadsto_b");
    settings_t setting{false, false, {0, 100}, 0};

    Monitor<delay_state_t> monitor(pos, neg, setting);

    std::vector<timed_input_t> word1 {
        timed_input_t({0, 2}, "c"),
        timed_input_t({2, 2}, "b"),
        timed_input_t({102, 104}, "b"),
        timed_input_t({104, 110}, "a"),
        timed_input_t({104, 110}, "c"),
        timed_input_t({115, 120}, "a"),
        timed_input_t({130, 135}, "b"),
        timed_input_t({130, 135}, "c"),
        timed_input_t({130, 135}, "c")
    };

    std::vector<timed_input_t> word2 {
        timed_input_t({140, 150}, "a"),
        timed_input_t({181, 200}, "c")
    };

    monitor.input(word1);
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);

    monitor.input(word2);
    BOOST_CHECK(monitor.status() == NEGATIVE);
}

BOOST_AUTO_TEST_CASE(delay_test4) {
    TA pos = Parser::parse("models/a-b.xml", "a_leadsto_b");
    TA neg = Parser::parse("models/a-b.xml", "not_a_leadsto_b");
    settings_t setting{false, false, {0, 100}, 1};

    Monitor<delay_state_t> monitor(pos, neg, setting);

    std::vector<timed_input_t> word1 {
        timed_input_t({0, 2}, "c"),
        timed_input_t({2, 2}, "b"),
        timed_input_t({102, 104}, "b"),
        timed_input_t({104, 110}, "a"),
        timed_input_t({104, 110}, "c"),
        timed_input_t({115, 120}, "a"),
        timed_input_t({130, 135}, "b"),
        timed_input_t({130, 135}, "c"),
        timed_input_t({130, 135}, "c")
    };

    std::vector<timed_input_t> word2 {
        timed_input_t({140, 150}, "a"),
        timed_input_t({181, 200}, "c")
    };

    std::vector<timed_input_t> word3 {
        timed_input_t({200, 200}, "a"),
        timed_input_t({232, 250}, "c")
    };

    monitor.input(word1);
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);

    monitor.input(word2);
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);

    monitor.input(word3);
    BOOST_CHECK(monitor.status() == NEGATIVE);
}

BOOST_AUTO_TEST_CASE(delay_test5) {
    TA pos = Parser::parse("models/delay-example.xml", "positive");
    TA neg = Parser::parse("models/delay-example.xml", "negative");
    settings_t setting{true, true, {0, 100}, 2};
    Monitor<delay_state_t> monitor(pos, neg, setting);

    monitor.input({173, "a"});
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);
    monitor.input({271, "b"});
    
    BOOST_CHECK(monitor.status() == NEGATIVE);
}

BOOST_AUTO_TEST_CASE(delay_test6) {
    TA pos = Parser::parse("models/delay-example.xml", "positive");
    TA neg = Parser::parse("models/delay-example.xml", "negative");
    settings_t setting{true, true, {0, 100}, 3};
    Monitor<delay_state_t> monitor(pos, neg, setting);

    monitor.input({173, "a"});
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);
    monitor.input({271, "b"});
    
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);
}

BOOST_AUTO_TEST_CASE(delay_test7) {
    TA pos = Parser::parse("models/delay-example.xml", "positive");
    TA neg = Parser::parse("models/delay-example.xml", "negative");
    settings_t setting{true, true, {0, 100}, 2};

    Monitor<delay_state_t> monitor(pos, neg, setting);

    monitor.input({173, "a"});
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);
    monitor.input({275, "b"});
    
    BOOST_CHECK(monitor.status() == INCONCLUSIVE);
}

BOOST_AUTO_TEST_CASE(delay_test8) {
    TA pos = Parser::parse("models/delay-example.xml", "positive");
    TA neg = Parser::parse("models/delay-example.xml", "negative");
    settings_t setting{true, true, {0, 100}, 2};

    Monitor<delay_state_t> monitor(pos, neg, setting);

    std::cout << "Input: (a, 73)\n";
    monitor.input({173, "a"});


    monitor.print_status(std::cout);

    BOOST_CHECK(monitor.status() == INCONCLUSIVE);
    
    std::cout << "Input: (b, 171)\n";
    monitor.input({275, "b"});
    

    monitor.print_status(std::cout);

    BOOST_CHECK(monitor.status() == INCONCLUSIVE);
}

BOOST_AUTO_TEST_CASE(delay_test9) {
    TA pos = Parser::parse("models/delay-example.xml", "positive");
    TA neg = Parser::parse("models/delay-example.xml", "negative");
    settings_t setting{true, true, {0, 100}, 2};

    Monitor<delay_state_t> monitor(pos, neg, setting);
    monitor.input({173, "a"});

    BOOST_CHECK(monitor.status() == INCONCLUSIVE);
    
    monitor.input({271, "b"});
    
    
    BOOST_CHECK(monitor.status() == NEGATIVE);
}