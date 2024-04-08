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
#include "monitaal/EventParser.h"

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

    std::vector<timed_input_t> word1 = {
            timed_input_t(0, "c"),
            timed_input_t(2, "b"),
            timed_input_t(102, "b"),
            timed_input_t(104, "a"),
            timed_input_t(104, "c"),
            timed_input_t(109, "a"),
            timed_input_t(119, "b"),
            timed_input_t(119, "c"),
            timed_input_t(119, "c")};

    std::vector<timed_input_t> word2 = {
            timed_input_t(119, "a"),
            timed_input_t(220, "c")};

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

    auto ne_states = Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(converge_ta);
    auto ne_states_delay = Fixpoint<delay_state_t>::buchi_accept_fixpoint(converge_ta);

    BOOST_CHECK(ne_states.is_empty());
    BOOST_CHECK(ne_states_delay.is_empty());
}

BOOST_AUTO_TEST_CASE(time_divergence_test_1) {
    TA pos = Parser::parse("models/time-must-pass.xml", "positive");
    TA neg = Parser::parse("models/time-must-pass.xml", "negative");

    pos.intersection(TA::time_divergence_ta({"a"}, true));
    neg.intersection(TA::time_divergence_ta({"a"}, true));

    BOOST_CHECK(Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(neg).is_empty());
    BOOST_CHECK(not Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(pos).is_empty());
    
    Interval_monitor monitor_int(pos, neg);
    Concrete_monitor monitor_con(pos, neg);

    BOOST_CHECK(monitor_int.status());
    BOOST_CHECK(monitor_con.status());

}

BOOST_AUTO_TEST_CASE(absentBQR_test1) {
    TA pos = Parser::parse("models/absentBQR.xml", "positive");
    TA neg = Parser::parse("models/absentBQR.xml", "negative");
    
    std::filebuf fb;
    fb.open("models/absentBQRinput.txt", std::ios::in);

    std::istream stream(&fb);

    auto events = EventParser::parse_input(&stream, 0);
    fb.close();

    Concrete_monitor monitor(pos, neg);

    int count = 0;

    for (const auto& e : events) {
        monitor.input(e);
        ++count;
        if (count < 10011)
            BOOST_CHECK(monitor.status() == INCONCLUSIVE);
        else
            BOOST_CHECK(monitor.status() == NEGATIVE);
    }
}

BOOST_AUTO_TEST_CASE(absentAQ_test1) {
    TA pos = Parser::parse("models/absentAQ.xml", "positive");
    TA neg = Parser::parse("models/absentAQ.xml", "negative");
    
    std::filebuf fb;
    fb.open("models/absentAQinput.txt", std::ios::in);

    std::istream stream(&fb);

    auto events = EventParser::parse_input(&stream, 0);
    fb.close();

    Concrete_monitor monitor(pos, neg);

    int count = 0;

    for (const auto& e : events) {
        monitor.input(e);
        ++count;

        if (count < 10028)
            BOOST_CHECK(monitor.status() == INCONCLUSIVE);
        else
            BOOST_CHECK(monitor.status() == NEGATIVE);
    }
}

BOOST_AUTO_TEST_CASE(absentBR_test1) {
    TA pos = Parser::parse("models/absentBR.xml", "positive");
    TA neg = Parser::parse("models/absentBR.xml", "negative");
    
    std::filebuf fb;
    fb.open("models/absentBRinput.txt", std::ios::in);

    std::istream stream(&fb);

    auto events = EventParser::parse_input(&stream, 0);
    fb.close();
    
    Concrete_monitor monitor(pos, neg);

    int count = 0;

    for (const auto& e : events) {
        monitor.input(e);
        ++count;

        if (count < 10028)
            BOOST_CHECK(monitor.status() == INCONCLUSIVE);
        else
            BOOST_CHECK(monitor.status() == NEGATIVE);
    }
}

BOOST_AUTO_TEST_CASE(recurBQR_test1) {
    TA pos = Parser::parse("models/recurBQR.xml", "positive");
    TA neg = Parser::parse("models/recurBQR.xml", "negative");
    
    std::filebuf fb;
    fb.open("models/recurBQRinput.txt", std::ios::in);

    std::istream stream(&fb);

    auto events = EventParser::parse_input(&stream, 0);
    fb.close();

    Concrete_monitor monitor(pos, neg);

    int count = 0;

    for (const auto& e : events) {
        monitor.input(e);
        ++count;

        if (count < 10014)
            BOOST_CHECK(monitor.status() == INCONCLUSIVE);
        else
            BOOST_CHECK(monitor.status() == NEGATIVE);
    }
}

BOOST_AUTO_TEST_CASE(recurGLB_test1) {
    TA pos = Parser::parse("models/recurGLB.xml", "positive");
    TA neg = Parser::parse("models/recurGLB.xml", "negative");
    
    std::filebuf fb;
    fb.open("models/recurGLBinput.txt", std::ios::in);

    std::istream stream(&fb);

    auto events = EventParser::parse_input(&stream, 0);
    fb.close();

    Concrete_monitor monitor(pos, neg);

    int count = 0;

    for (const auto& e : events) {
        // if (monitor.status() != INCONCLUSIVE) {
        //     std::cout << "status: " << monitor.status() << " count: " << count << '\n';
        //     break;
        // }
        monitor.input(e);
        ++count;

        if (count < 10015)
            BOOST_CHECK(monitor.status() == INCONCLUSIVE);
        else
            BOOST_CHECK(monitor.status() == NEGATIVE);
    }
}