// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2012, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

///////////////////////////////////////////////////////////////////////////////
/// \file priority_queue.h
/// \brief External memory priority queue implementation.
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_PRIORITY_QUEUE_H_
#define _TPIE_PRIORITY_QUEUE_H_

#include <tpie/config.h>
#include "portability.h"
#include "tpie_log.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <string>
#include <sstream>
#include <tpie/err.h>
#include <tpie/stream.h>
#include <tpie/array.h>
#include <tpie/internal_priority_queue.h>
#include <boost/filesystem.hpp>
#include <tpie/parallel_sort.h>

namespace tpie {

	struct priority_queue_error : public std::logic_error {
		priority_queue_error(const std::string& what) : std::logic_error(what)
		{ }
	};

///////////////////////////////////////////////////////////////////////////////
/// \class priority_queue
/// \brief External memory priority queue implementation.
///
/// Originally implemented by Lars Hvam Petersen for his Master's thesis
/// titled "External Priority Queues in Practice", June 2007.
/// This implementation, named "PQSequence3", is the fastest among the
/// priority queue implementations studied in the paper.
/// Inspiration: Sanders - Fast priority queues for cached memory (1999).
///
/// For an overview of the algorithm, refer to Sanders (1999) section 2 and
/// figure 1, or Lars Hvam's thesis, section 4.4.
///
/// In the debug log, the priority queue reports two values setting_k and
/// setting_m. The priority queue has a maximum capacity which is on the order
/// of setting_m * setting_k**setting_k elements (where ** denotes
/// exponentiation).
///
/// However, even with as little as 8 MB of memory, this maximum capacity in
/// practice exceeds 2**48, corresponding to a petabyte-sized dataset of 32-bit
/// integers.
///////////////////////////////////////////////////////////////////////////////

template<typename T, typename Comparator = std::less<T> >
class priority_queue {
	typedef memory_size_type run_type;
	typedef run_type group_type;
	typedef run_type slot_type;

	struct merge_heap_element {
		T item;
		run_type source;

		merge_heap_element() {}

		merge_heap_element(const T & item, run_type source)
			: item(item)
			, source(source)
		{
		}
	};

	struct merge_comp_type {
		typedef merge_heap_element first_argument_type;
		typedef merge_heap_element second_argument_type;
		typedef bool result_type;
		Comparator comp_;
		merge_comp_type(Comparator & c) : comp_(c) {}
		bool operator()(const merge_heap_element & a, const merge_heap_element & b) {
			return comp_(a.item, b.item);
		}
	};

	typedef internal_priority_queue<merge_heap_element, merge_comp_type> merge_heap;
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Constructor.
	///
	/// \param f Factor of memory that the priority queue is allowed to use.
	/// \param b Block factor
	///////////////////////////////////////////////////////////////////////////
	priority_queue(double f=1.0, float b=0.0625);

#ifndef DOXYGEN
	// \param mmavail Number of bytes the priority queue is allowed to use.
	// \param b Block factor
	priority_queue(memory_size_type mm_avail, float b=0.0625);
#endif


    /////////////////////////////////////////////////////////
    ///
    /// Insert an element into the priority queue
    ///
    /// \param x The item
    ///
    /////////////////////////////////////////////////////////
    void push(const T& x);

    /////////////////////////////////////////////////////////
    ///
    /// Remove the top element from the priority queue
    ///
    /////////////////////////////////////////////////////////
    void pop();

    /////////////////////////////////////////////////////////
    ///
    /// See what's on the top of the priority queue
    ///
    /// \return Top element
    ///
    /////////////////////////////////////////////////////////
    const T& top();

    /////////////////////////////////////////////////////////
    ///
    /// Returns the size of the queue
    ///
    /// \return Queue size
    ///
    /////////////////////////////////////////////////////////
    stream_size_type size() const;

    /////////////////////////////////////////////////////////
    ///
    /// Return true if queue is empty otherwise false
    ///
    /// \return Boolean - empty or not
    ///
    /////////////////////////////////////////////////////////
    bool empty() const;

    /////////////////////////////////////////////////////////
    ///
    /// Pop all elements with priority equal to that of the
    /// top element, and process each by invoking f's call
    /// operator on the element.
    ///
    /// \param f - assumed to have a call operator with parameter of type T.
    ///
    /// \return The argument f
    ///
    /////////////////////////////////////////////////////////
    template <typename F> F pop_equals(F f);

private:
	struct run_state_type {
		memory_size_type start;
		memory_size_type size;
	};

    Comparator comp_;

    T min;
    bool min_in_buffer;

	/** Overflow priority queue (for buffering inserted elements). Capacity m. */
	internal_priority_queue<T, Comparator> opq;

	/** Deletion buffer containing the m' top elements in the entire structure. */
	tpie::array<T> buffer;

	/** Group buffers contain at most m elements all less or equal to elements
	 * in the corresponding group slots. Elements in group buffers are *not*
	 * repeated in actual group slots. For efficiency, we keep group buffer 0
	 * in memory. */
	tpie::array<T> gbuffer0;

	class cyclic_array_iterator : public boost::iterator_facade<cyclic_array_iterator, T, boost::random_access_traversal_tag> {
		tpie::array<T> & arr;
		memory_size_type idx;
		memory_size_type first;

	public:
		cyclic_array_iterator(tpie::array<T> & arr, memory_size_type idx, memory_size_type first)
			: arr(arr), idx(idx % arr.size()), first(first)
		{
		}

	private:
		friend class boost::iterator_core_access;
		T & dereference() const {
			return arr[idx];
		}
		void increment() {
			if (idx+1 == arr.size()) idx = 0;
			else ++idx;
		}
		void decrement() {
			if (idx == 0) idx = arr.size() - 1;
			else --idx;
		}
		void advance(memory_size_type n) {
			idx = (idx + n) % arr.size();
		}
		memory_size_type from_beginning() const {
			return (idx < first) ? (idx + arr.size() - first) : (idx - first);
		}
		memory_offset_type distance_to(const cyclic_array_iterator & other) const {
			return static_cast<memory_offset_type>(other.from_beginning())
				-static_cast<memory_offset_type>(from_beginning());
		}
		bool equal(const cyclic_array_iterator & other) const {
			return idx == other.idx;
		}
	};

	template <typename IT>
	void assert_sorted(IT i, IT j, memory_size_type n) {
		if (j-i != n) {
			log_error() << "Bad distance" << std::endl;
		}
		IT k = i;
		++k;
		--n;
		while (k != j) {
			if (comp_(*k, *i)) {
				log_error() << "Not a sorted sequence" << std::endl;
			}
			++k, ++i;
			--n;
		}
		if (n != 0) {
			log_error() << "Bad steps" << std::endl;
		}
	}

	/** 2*(#slots) integers. Slot i contains its elements in cyclic ascending order,
	 * starting at index slot_state[2*i]. Slot i contains slot_state[2*i+1] elements.
	 * Its data is in data file i. */
	tpie::array<run_state_type> slot_state;

	/** 2*(#groups) integers. Group buffer i has its elements in cyclic ascending order,
	 * starting at index group_state[2*i]. Gbuffer i contains group_state[2*i+1] elements. */
	tpie::array<run_state_type> group_state;

	/** k, the fanout of each group and the max fanout R. */
	memory_size_type setting_k;
	/** Number of groups in use. */
	memory_size_type current_r;
	/** m, the size of a slot and the size of the group buffers. */
	memory_size_type setting_m;
	/** m', the size of the deletion buffer. */
	memory_size_type setting_mmark;

    stream_size_type m_size;
    memory_size_type buffer_size;
    memory_size_type buffer_start;

	float block_factor;

	void init(memory_size_type mm_avail);

    array<temp_file> datafiles;
    array<temp_file> groupdatafiles;

    void write_slot(slot_type slotid, array<T> & arr, memory_size_type len);
    slot_type free_slot(group_type group);
    void empty_group(group_type group);
    void fill_buffer();
    void fill_group_buffer(group_type group);
    void validate();
    void remove_group_buffer(group_type group);
    void dump();
};

#include "priority_queue.inl"

    namespace ami {
		using tpie::priority_queue;
    }  //  ami namespace

}  //  tpie namespace

#endif
