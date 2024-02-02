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

#include "monitaal/EventParser.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace monitaal;

BOOST_AUTO_TEST_CASE(interval_parsing_test1) {
    std::stringstream stream("@[0, 10] \ta\n@[5, 10] b@[10, 10] a\n@[20, 25] b @[25, 30] a\n\n", std::ios_base::in);

    std::vector<interval_input> input;

    input = EventParser::parse_interval_input(&stream);

    BOOST_CHECK(input.size() == 5);
}

BOOST_AUTO_TEST_CASE(concrete_parsing_test1) {
    std::stringstream stream("@1 a\n@2 b@3 a\n@4b @5 a\n@6.2 a\n", std::ios_base::in);

    std::vector<concrete_input> input;

    input = EventParser::parse_concrete_input(&stream);

    BOOST_CHECK(input.size() == 6);
}

BOOST_AUTO_TEST_CASE(empty_label_test1) {
    std::stringstream stream("@1 \n@2 a\n@3 \n@4 b", std::ios_base::in);

    std::vector<concrete_input> input;

    input = EventParser::parse_concrete_input(&stream);

    BOOST_CHECK(input.size() == 4);
    BOOST_CHECK(input[0].label == "");
    BOOST_CHECK(input[1].label == "a");
    BOOST_CHECK(input[2].label == "");
    BOOST_CHECK(input[3].label == "b");
    BOOST_CHECK(input[0].time == 1);
    BOOST_CHECK(input[1].time == 2);
    BOOST_CHECK(input[2].time == 3);
    BOOST_CHECK(input[3].time == 4);
}