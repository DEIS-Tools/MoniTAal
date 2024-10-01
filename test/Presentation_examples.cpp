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
    TA pos = Parser::parse_file("models/a-b30.xml", "a_leadsto_b");
    TA neg = Parser::parse_file("models/a-b30.xml", "not_a_leadsto_b");
    auto div = TA::time_divergence_ta({"a", "b", "c"}, true);

    std::cout << "<<<<<< Parsing models >>>>>>\n\nPositive Model:\n" << pos << "\nNegative Model:\n" << neg;

    /* Fixpoint states */

    std::cout << "<<<<<< Calculating fixpoints >>>>>>\n\nPositive fixpoint states:\n";
    Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(pos).print(std::cout, pos);

    std::cout << "Negative fixpoint states:\n";
    Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(neg).print(std::cout, neg);



    /* Conjunction with Divergence automaton */

    pos.intersection(div);
    neg.intersection(div);

    std::cout << "\n<<<<<< Conjuntion with divergence automaton >>>>>>\n\n" << "Positive Model:\n" << pos << "\nNegative Model:\n" << neg << "\n\n";

    std::cout << "<<<<<< Calculating fixpoints >>>>>>\n\nPositive fixpoint states:\n";
    Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(pos).print(std::cout, pos);

    std::cout << "Negative fixpoint states:\n";
    Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(neg).print(std::cout, neg);

    std::cout << "<<<<<< Monitoring >>>>>>\n\n";
    std::vector<timed_input_t> word1 = {
            timed_input_t({0, 1}, "c"),
            timed_input_t({3, 6}, "b"),
            timed_input_t({50, 100}, "b"),
            timed_input_t({102, 103}, "a"),
            timed_input_t({103, 103}, "c"),
            timed_input_t({105, 110}, "a"),
            timed_input_t({115, 150}, "b"),
            timed_input_t({140, 150}, "c")};


    std::vector<timed_input_t> word2 = {
            timed_input_t({160, 170}, "a"),
            timed_input_t({201, 210}, "c")};

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
    TA pos = Parser::parse_file("models/a-b30.xml", "a_leadsto_b");
    TA neg = Parser::parse_file("models/a-b30.xml", "not_a_leadsto_b");
    auto div = TA::time_divergence_ta({"a", "b", "c"}, true);

    std::cout << "<<<<<< Parsing models >>>>>>\n\nPositive Model:\n" << pos << "\nNegative Model:\n" << neg;


    /* Fixpoint states */

    std::cout << "<<<<<< Calculating fixpoints >>>>>>\n\nPositive fixpoint states:\n";
    Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(pos).print(std::cout, pos);

    std::cout << "Negative fixpoint states:\n";
    Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(neg).print(std::cout, neg);



    /* Conjunction with Divergence automaton */

   pos.intersection(div);
   neg.intersection(div);

    std::cout << "\n TIME DIV automata\n" << div << "\n";

    std::cout << "\n<<<<<< Conjuntion with divergence automaton >>>>>>\n\n" << "Positive Model:\n" << pos << "\nNegative Model:\n" << neg << "\n\n";

    std::cout << "<<<<<< Calculating fixpoints >>>>>>\n\nPositive fixpoint states:\n";
    Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(pos).print(std::cout, pos);

    std::cout << "Negative fixpoint states:\n";
    Fixpoint<symbolic_state_t>::buchi_accept_fixpoint(neg).print(std::cout, neg);



    /* Monitoring */

    std::cout << "<<<<<< Monitoring >>>>>>\n\n";
    std::vector<timed_input_t> word1 = {
            timed_input_t(0, "c"),
            timed_input_t(2, "b"),
            timed_input_t(102, "b"),
            timed_input_t(104, "a"),
            timed_input_t(105, "c"),
            timed_input_t(110, "a"),
            timed_input_t(120, "b"),
            timed_input_t(120, "c"),
            timed_input_t(120, "c")};

    std::vector<timed_input_t> word2 = {
            timed_input_t(120, "a"),
            timed_input_t(221, "c")};

    Concrete_monitor monitor(pos, neg);

    std::cout << "Monitoring word: ";
    for (const auto& c : word1)
        std::cout << "(" << c.label << ", " << c.time.first <<  ") ";

    monitor.input(word1);

    std::cout << "\nConclusion: " << monitor.status() << "\nState estimate positive:\n";
    BOOST_CHECK(monitor.status() == monitor_answer_e::INCONCLUSIVE);
    
    int i;
    if (monitor.positive_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.positive_state_estimate()) {
            s.print(std::cout, pos);
        }

    std::cout << "State estimate negative:\n";
    if (monitor.negative_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.negative_state_estimate()) {
            s.print(std::cout, neg);
        }

    std::cout << "\nMonitoring word: ";
    for (const auto& c : word2)
        std::cout << "(" << c.label << ", " << c.time.first <<  ") ";

    monitor.input(word2);

    std::cout << "\nConclusion: " << monitor.status() << "\nState estimate positive:\n";
    BOOST_CHECK(monitor.status() == monitor_answer_e::NEGATIVE);

    if (monitor.positive_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.positive_state_estimate()) {
            s.print(std::cout, pos);
        }

    std::cout << "State estimate negative:\n";
    if (monitor.negative_state_estimate().empty())
        std::cout << "empty\n";
    else
        for (const auto& s : monitor.negative_state_estimate()) {
            s.print(std::cout, neg);
        }
}