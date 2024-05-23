/*
 * Copyright Thomas M. Grosen 
 * Created on 23/05/2022
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

#ifndef MONITAAL_FIXPOINT_H
#define MONITAAL_FIXPOINT_H

#include "TA.h"
#include "types.h"
#include "state.h"

namespace monitaal {

    template<class state_t>
    class Fixpoint {
    public:
        /**
         * Calculates the set of states that can reach the given states (taking at least one transition).
         * @param states: Set of states of the TA to be reached.
         * @param T: The Timed Automaton.
         * @return Set of states in a map that can reach the given states.
         */
        static state_map_t<state_t> reach(const state_map_t<state_t>& states, const TA& T);

        /**
         * Fetches all the states (symbolic) that are in accepting locations.
         * @param T: The Timed Automaton.
         * @return A set of symbolic states encapsulating accept states.
         */
        static state_map_t<state_t> accept_states(const TA& T);

        /**
         * Calculates the set of states that can infinitely often reach an accepting state.
         * @param T: The Timed Automaton.
         * @return The maximum set of symbolic states that can reach an accepting state infinitely.
         */
        static state_map_t<state_t> buchi_accept_fixpoint(const TA& T);

        /**
         * Calculates the set of states that can be reached at a given time point
         * by following unobservable transitions.
         * @param states: Set of states the reachability analysis explores from.
         * @param T: The Timed Automaton.
         * @param observables: Observable transitions are not explored.
         * @param time: Amount of time that will elapse.
         * @return The set of states that can be reached by unobservable transitions within the time frame.
         */
//        static state_map_t
//        restricted_unobservable_reach(const state_map_t& states, const TA& T,
//                                      std::vector<std::string> observables, symb_time_t time);

        /**
         * Calculates the set of states that can be reached within a given time interval
         * by following unobservable transitions.
         * @param states: Set of states the reachability analysis explores from.
         * @param T: The Timed Automaton.
         * @param observables: Observable transitions are not explored.
         * @param time: Time interval of the reachability analysis.
         * @return The set of states that can be reached by unobservable transitions within the time frame.
         */
//        static state_map_t
//        restricted_unobservable_reach(const state_map_t& states, const TA& T,
//                                      std::vector<std::string> observables, interval_t time);
    };

}

#endif //MONITAAL_FIXPOINT_H
