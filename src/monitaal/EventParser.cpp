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

    long double read_concrete_time(std::istream* stream) {
        long double time;
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

    template<bool is_interval> typename std::conditional_t<is_interval, interval_t, long double> 
    read_time(std::istream* stream);

    template<> long double read_time<false>(std::istream* stream) {
        long double time;
        *stream >> time;
        return time;
    }

    template<> interval_t read_time<true>(std::istream* stream) {
        int lower, upper;
        char c;

        if ( *stream >> std::ws >> c && c == '[' &&
            *stream >> lower &&
            *stream >> std::ws >> c && c == ',' &&
            *stream >> upper &&
            *stream >> std::ws >> c && c == ']')
            return {lower, upper};
        else {
            throw base_error("Error parsing interval input at ", stream->tellg(), " but got \"", stream->peek(), "\"");
        }
    }

    std::string read_observation(std::istream* stream) {
        std::string observation = "";
        char c;
        
        skip_space(stream);

        c = stream->peek();
        while (c != '\n' && c != '@' && !stream->eof() && c != '\000') {
            observation += stream->get();
            c = stream->peek();
        }

        return observation;
    }

    std::vector<concrete_input> EventParser::parse_concrete_input(std::istream* stream) {
        char separator;
        std::string observation;
        concrete_time_t time;

        std::vector<concrete_input> events;

        *stream >> std::ws;
        while (not stream->eof() && stream->peek() != '\000') {
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

    template<bool is_interval>
    std::vector<typename std::conditional_t<is_interval, interval_input, concrete_input>> 
    EventParser::parse_input(std::istream* stream) {
        std::string observation;
        typename std::conditional_t<is_interval, interval_t, long double> time;

        std::vector<typename std::conditional_t<is_interval, interval_input, concrete_input>> events;

        *stream >> std::ws;
        while (not stream->eof() && stream->peek() != '\000') {
            read_seperator(stream);
            time = read_time<is_interval>(stream);

            observation = read_observation(stream);

            events.emplace_back(time, observation);

            *stream >> std::ws;
        }

        return events;
    }

    template std::vector<interval_input> EventParser::parse_input<true>(std::istream* stream);
    template std::vector<concrete_input> EventParser::parse_input<false>(std::istream* stream);

}