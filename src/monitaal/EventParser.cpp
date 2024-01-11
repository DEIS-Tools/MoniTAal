/*
 * Copyright Thomas M. Grosen 
 * Created on 11/01/2022
 */

/*
 * This file is part of MoniTAal
 *
 * MoniTAal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MoniTAal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MoniTAal. If not, see <https://www.gnu.org/licenses/>.
 */

#include "EventParser.h"
#include "TA.h"
#include "errors.h"

#include <stdio.h>
#include <string>
#include <istream>
#include <iostream>
#include <locale>

namespace monitaal {

    void read_seperator(std::istream* stream) {
        char c;
        *stream >> std::ws;
        if (not (*stream >> c && c == '@'))
            throw base_error("Expected a @ separator at ", stream->tellg(), " but got \"", stream->peek(), "\"");
        return;
    }

    concrete_time_t read_concrete_time(std::istream* stream) {
        concrete_time_t time;
        *stream >> time;
        return time;
    }

    interval_t read_interval(std::istream* stream) {
        int lower, upper;
        char c;

        if ( *stream >> std::ws >> c && c == '[' &&
             *stream >> lower &&
             *stream >> std::ws >> c && c == ',' &&
             *stream >> upper &&
             *stream >> std::ws >> c && c == ']')
            return std::pair(lower, upper);
        else {
            throw base_error("Error parsing interval input at ", stream->tellg(), " but got \"", stream->peek(), "\"");
        }
    }

    std::string read_observation(std::istream* stream) {
        std::string observation = "";
        char c;
        
        *stream >> std::ws;

        c = stream->peek();
        while (c != ' ' && c != '\t' && c != '\n' && c != '@') {
            observation += stream->get();
            c = stream->peek();
        }

        if (observation.length() == 0)
            throw base_error("Expected a label string not containing whitespace or @ at ", stream->tellg(), " but got \"", stream->peek(), "\"");

        return observation;
    }

    std::vector<concrete_input> EventParser::parse_concrete_input(std::istream* stream) {
        char separator;
        std::string observation;
        concrete_time_t time;

        std::vector<concrete_input> events;

        *stream >> std::ws;
        while (not stream->eof()) {
            read_seperator(stream);
            time = read_concrete_time(stream);
            observation = read_observation(stream);

            events.emplace_back(time, observation);

            *stream >> std::ws;
        }

        return events;
    }

    std::vector<interval_input> EventParser::parse_interval_input(std::istream* stream) {
        std::string observation;
        interval_t time;

        std::vector<interval_input> events;

        *stream >> std::ws;
        while (not stream->eof()) {
            read_seperator(stream);
            time = read_interval(stream);
            observation = read_observation(stream);

            events.emplace_back(time, observation);
            
            *stream >> std::ws;
        }

        return events;
    }

}