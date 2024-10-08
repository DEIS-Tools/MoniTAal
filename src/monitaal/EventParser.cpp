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

    void skip_space(std::istream* stream) {
        char c = stream->peek();
        while(c == ' ' || c == '\t') {
            stream->get(); 
            c = stream->peek();
        }
        return;
    }

    void read_seperator(std::istream* stream) {
        char c;
        skip_space(stream);
        if (not (*stream >> c && c == '@'))
            throw base_error("Expected a @ separator at ", stream->tellg(), " but got \"", c, "\"");
        return;
    }

    interval_t read_time(std::istream* stream) {
        int lower, upper;
        char c;
        *stream >> std::ws;
        
        if (stream->peek() == '[') {
            if ( *stream >> c &&
                *stream >> lower &&
                *stream >> std::ws >> c && c == ',' &&
                *stream >> upper &&
                *stream >> std::ws >> c && c == ']')
                return {lower, upper};
            else {
                throw base_error("Error parsing interval input at ", stream->tellg(), " got: \"", c, "\" but expected interval on the form [l, u]");
            }
        } else {
            *stream >> lower;
            return {lower, lower};
        }
    }

    std::string read_observation(std::istream* stream) {
        std::string observation = "";
        char c;
        
        skip_space(stream);

        c = stream->peek();
        while (c != '\n' && c != '@' && !stream->eof() && c != '\000' && c != '\t' && c != ' ') {
            observation += stream->get();
            c = stream->peek();
        }

        return observation;
    }

    std::vector<timed_input_t> EventParser::parse_input(std::istream* stream, uint32_t limit) {
        std::string observation;
        interval_t time;

        std::vector<timed_input_t> events;

        uint32_t counter = 0;

        *stream >> std::ws;
        while (not stream->eof() && stream->peek() != '\000' && (limit == 0 || counter < limit)) {
            read_seperator(stream);
            time = read_time(stream);

            observation = read_observation(stream);

            events.emplace_back(time, observation);
            ++counter;

            *stream >> std::ws;
        }

        return events;
    }

}