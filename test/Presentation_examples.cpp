/*
 * Copyright Thomas M. Grosen 
 * Created on 19/09/2022
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


BOOST_AUTO_TEST_CASE(presentation_interval) {
    TA pos = Parser::parse("models/a-b30.xml", "a_leadsto_b");
    TA neg = Parser::parse("models/a-b30.xml", "not_a_leadsto_b");
    auto div = TA::time_divergence_ta({"a", "b", "c"}, true);

    std::cout << "<<<<<< Parsing models >>>>>>\n\nPositive Model:\n" << pos << "\nNegative Model:\n" << neg;

    /* Fixpoint states */

    std::cout << "<<<<<< Calculating fixpoints >>>>>>\n\nPositive fixpoint states:\n";
    Fixpoint::buchi_accept_fixpoint(pos).print(std::cout, pos);

    std::cout << "Negative fixpoint states:\n";
    Fixpoint::buchi_accept_fixpoint(neg).print(std::cout, neg);



    /* Conjunction with Divergence automaton */

    pos.intersection(div);
    neg.intersection(div);

    std::cout << "\n<<<<<< Conjuntion with divergence automaton >>>>>>\n\n" << "Positive Model:\n" << pos << "\nNegative Model:\n" << neg << "\n\n";

    std::cout << "<<<<<< Calculating fixpoints >>>>>>\n\nPositive fixpoint states:\n";
    Fixpoint::buchi_accept_fixpoint(pos).print(std::cout, pos);

    std::cout << "Negative fixpoint states:\n";
    Fixpoint::buchi_accept_fixpoint(neg).print(std::cout, neg);

    std::cout << "<<<<<< Monitoring >>>>>>\n\n";
    std::vector<interval_input> word1 = {
            interval_input({0, 1}, "c"),
            interval_input({2, 5}, "b"),
            interval_input({50, 100}, "b"),
            interval_input({2, 3}, "a"),
            interval_input({0, 0}, "c"),
            interval_input({1, 5}, "a"),
            interval_input({0, 30}, "b"),
            interval_input({1, 20}, "c")};


    std::vector<interval_input> word2 = {
            interval_input({0, 10}, "a"),
            interval_input({31, 40}, "c")};

    Interval_monitor monitor(pos, neg);

    std::cout << "Monitoring word: ";
    for (const auto& c : word1)
        std::cout << "(" << c.label << ", " << c.time.first << "-" << c.time.second <<  ") ";

    monitor.input(word1);

    std::cout << "\nConclusion: " << monitor.status() << "\nState estimate positive:\n";
    BOOST_CHECK(monitor.status() == monitor_answer_e::INCONCLUSIVE);

    if (monitor.positive_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.positive_state_estimate()) {
            std::cout << pos.locations().at(s.location()).name() << s.federation() << "\n";
        }

    std::cout << "\nState estimate negative:\n";

    if (monitor.negative_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.negative_state_estimate()) {
            std::cout << neg.locations().at(s.location()).name() << s.federation() << "\n";
        }



    std::cout << "Monitoring word: ";
    for (const auto& c : word2)
        std::cout << "(" << c.label << ", " << c.time.first << "-" << c.time.second <<  ") ";

    monitor.input(word2);

    std::cout << "\nConclusion: " << monitor.status() << "\nState estimate positive:\n";
    BOOST_CHECK(monitor.status() == monitor_answer_e::NEGATIVE);

    if (monitor.positive_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.positive_state_estimate()) {
            std::cout << pos.locations().at(s.location()).name() << s.federation() << "\n";
        }

    std::cout << "\nState estimate negative:\n";

    if (monitor.negative_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.negative_state_estimate()) {
            std::cout << neg.locations().at(s.location()).name() << s.federation() << "\n";
        }

}

BOOST_AUTO_TEST_CASE(presentation_concrete) {
    TA pos = Parser::parse("models/a-b30.xml", "a_leadsto_b");
    TA neg = Parser::parse("models/a-b30.xml", "not_a_leadsto_b");
    auto div = TA::time_divergence_ta({"a", "b", "c"}, true);

    std::cout << "<<<<<< Parsing models >>>>>>\n\nPositive Model:\n" << pos << "\nNegative Model:\n" << neg;


    /* Fixpoint states */

    std::cout << "<<<<<< Calculating fixpoints >>>>>>\n\nPositive fixpoint states:\n";
    Fixpoint::buchi_accept_fixpoint(pos).print(std::cout, pos);

    std::cout << "Negative fixpoint states:\n";
    Fixpoint::buchi_accept_fixpoint(neg).print(std::cout, neg);



    /* Conjunction with Divergence automaton */

   pos.intersection(div);
   neg.intersection(div);

    std::cout << "\n TIME DIV automata\n" << div << "\n";

    std::cout << "\n<<<<<< Conjuntion with divergence automaton >>>>>>\n\n" << "Positive Model:\n" << pos << "\nNegative Model:\n" << neg << "\n\n";

    std::cout << "<<<<<< Calculating fixpoints >>>>>>\n\nPositive fixpoint states:\n";
    Fixpoint::buchi_accept_fixpoint(pos).print(std::cout, pos);

    std::cout << "Negative fixpoint states:\n";
    Fixpoint::buchi_accept_fixpoint(neg).print(std::cout, neg);



    /* Monitoring */

    std::cout << "<<<<<< Monitoring >>>>>>\n\n";
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

    std::cout << "Monitoring word: ";
    for (const auto& c : word1)
        std::cout << "(" << c.label << ", " << c.time <<  ") ";

    monitor.input(word1);

    std::cout << "\nConclusion: " << monitor.status() << "\nState estimate positive:\n";
    BOOST_CHECK(monitor.status() == monitor_answer_e::INCONCLUSIVE);
    
    int i;
    if (monitor.positive_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.positive_state_estimate()) {
            std::cout << pos.locations().at(s.location()).name() << " : ";
            i = 0;
            for (const auto& v : s.valuation())
                std::cout << pos.clock_name(i++) << " = " << v << ", ";
            std::cout << "\n";
        }

    std::cout << "State estimate negative:\n";
    if (monitor.negative_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.negative_state_estimate()) {
            std::cout << neg.locations().at(s.location()).name() << " : ";
            i = 0;
            for (const auto& v : s.valuation())
                std::cout << neg.clock_name(i++) << " = " << v << ", ";
            std::cout << "\n";
        }

    std::cout << "\nMonitoring word: ";
    for (const auto& c : word2)
        std::cout << "(" << c.label << ", " << c.time <<  ") ";

    monitor.input(word2);

    std::cout << "\nConclusion: " << monitor.status() << "\nState estimate positive:\n";
    BOOST_CHECK(monitor.status() == monitor_answer_e::NEGATIVE);

    if (monitor.positive_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.positive_state_estimate()) {
            std::cout << pos.locations().at(s.location()).name() << " : ";
            i = 0;
            for (const auto& v : s.valuation())
                std::cout << pos.clock_name(i++) << " = " << v << ", ";
            std::cout << "\n";
        }

    std::cout << "State estimate negative:\n";
    if (monitor.negative_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.negative_state_estimate()) {
            std::cout << neg.locations().at(s.location()).name() << " : ";
            i = 0;
            for (const auto& v : s.valuation()) {
                std::cout << neg.clock_name(i++) << " = " << v << ", ";
            }
            std::cout << "\n";
        }
}