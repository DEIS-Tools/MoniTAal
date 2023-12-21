/*
 * Copyright Thomas M. Grosen 
 * Created on 12/12/2023
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
#include "monitaal/Fixpoint.h"

#include <boost/test/unit_test.hpp>
#include <filesystem>

using namespace monitaal;

BOOST_AUTO_TEST_CASE(assumption_test_1) {
    TA pos = Parser::parse("models/knowledge_test.xml", "Positive");
    TA neg = Parser::parse("models/knowledge_test.xml", "Negative");
    TA assumption = Parser::parse("models/knowledge_test.xml", "Assumption");

    pos.intersection(assumption);
    neg.intersection(assumption);

    auto monitor1 = Interval_monitor(pos, neg);
    auto monitor2 = Interval_monitor(pos, neg);


    std::vector<interval_input> word1 = {{{9, 10}, "a[0]"},
                                         {{9, 10}, "a[1]"},
                                         {{9, 10}, "a[2]"},
                                         {{9, 10}, "a[3]"},
                                         {{7, 8}, "a[4]"},
                                         {{8, 9}, "a[5]"}};
                                   // 54 - 60
                                   // 74 - 100 
    int min = 0, max = 0;
    std::cout << "\n---\nMonitoring under assumption test 1\n";
    for (const auto& c : word1) {
        min += c.time.first;
        max += c.time.second;
        std::cout << "Input: (" << c.label << ", [" << min << ", " << max <<  "])\n";
        monitor1.input(c);

        std::cout << monitor1.status() << "\n\n";
    }




    std::vector<interval_input> word2 = {{{5, 6}, "a[0]"},
                                         {{5, 6}, "a[1]"},
                                         {{5, 6}, "a[2]"},
                                         {{5, 6}, "a[3]"},
                                         {{5, 6}, "a[4]"},
                                         {{5, 6}, "a[5]"},
                                         {{5, 6}, "a[6]"},
                                         {{5, 6}, "a[7]"}};
                                        // 40 - 48
                                        // 50 - 68
    
    min = 0; max = 0;
    std::cout << "\n---\nMonitoring under assumption test 2\n";
    for (const auto& c : word2) {
        min += c.time.first;
        max += c.time.second;
        std::cout << "Input: (" << c.label << ", [" << min << ", " << max <<  "])\n";
        monitor2.input(c);

        std::cout << monitor2.status() << "\n\n";
    }
}