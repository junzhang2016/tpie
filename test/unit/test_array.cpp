// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
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

#include "common.h"

#include <tpie/array.h>
#include <tpie/array_view.h>
#include <tpie/bit_array.h>
#include <tpie/array.h>
#include <tpie/concepts.h>
using namespace tpie;

bool basic_test() {
	array<size_t> hat;
	//Resize
	hat.resize(52, 42);
	TEST_ENSURE(hat.size() == 52, "Wrong size"); 
	for (memory_size_type i=0; i < 52; ++i)
		TEST_ENSURE_EQUALITY(hat[i], 42, "Wrong value");
	
	//Get and set
	for (memory_size_type i=0; i < 52; ++i)
		hat[i] = (i * 104729) % 2251;
  
	const tpie::array<size_t> & hat2(hat);
	for (memory_size_type i=0; i < 52; ++i)
		TEST_ENSURE_EQUALITY(hat2[i], ((i * 104729) % 2251), "Wrong value");

	TEST_ENSURE(!hat.empty(), "Empty");
	hat.resize(0);
	TEST_ENSURE(hat.empty(), "Not empty");
	array<int> a(1,0),b(4,0),c(11,0);
	a[0] = b[0] = c[0] = 1;
	TEST_ENSURE(a[0] && b[0] && c[0], "Wrong value");
	a[0] = b[0] = c[0] = 0;
	TEST_ENSURE(!a[0] && !b[0] && !c[0], "Wrong value")
	return true;
}

class auto_ptr_test_class {
public:
	size_t & dc;
	size_t & cc;
	auto_ptr_test_class(size_t & cc_, size_t & dc_): dc(dc_), cc(cc_) {
		++cc;
	}
	~auto_ptr_test_class() {
		++dc;
	}
	size_t hat() {return 42;}
private:
	auto_ptr_test_class(const auto_ptr_test_class & o): dc(o.dc), cc(o.cc) {}
};


bool auto_ptr_test() {
	size_t s=1234;
	size_t cc=0;
	size_t dc=0;
	array<tpie::auto_ptr<auto_ptr_test_class> > a;
	a.resize(s);
	for(size_t i=0; i < s; ++i) 
		a[i].reset(tpie_new<auto_ptr_test_class, size_t &, size_t &>(cc, dc));
	TEST_ENSURE_EQUALITY(cc, s, "Wrong value");
	TEST_ENSURE_EQUALITY(dc, 0, "Wrong value");
	size_t x=0;
	for(size_t i=0; i < s; ++i) 
		x += a[i]->hat();
	TEST_ENSURE_EQUALITY(x, 42*s, "Wrong value");
	TEST_ENSURE_EQUALITY(cc, s, "Wrong value");
	TEST_ENSURE_EQUALITY(dc, 0, "Wrong value");
	for(size_t i=0; i < s; ++i) 
		a[i].reset(tpie_new<auto_ptr_test_class>(cc, dc));

	TEST_ENSURE_EQUALITY(cc, 2*s, "Wrong value");
	TEST_ENSURE_EQUALITY(dc, s, "Wrong value");
	a.resize(0);
	TEST_ENSURE_EQUALITY(cc, 2*s, "Wrong value");
	TEST_ENSURE_EQUALITY(dc, 2*s, "Wrong value");
	return true;
}

bool segmented_array_test() {
	array<int> h1;
	array_base<int, true> h2;
	size_t z=8388619;
	h1.resize(z);
	h2.resize(z);
	for (size_type i=0; i < z; ++i)
		h2[i] = h1[i] = static_cast<int>((i * 833547)%z);

	array<int>::iterator i1=h1.begin();
	array_base<int, true>::iterator i2=h2.begin();
	
	while (i1 != h1.end() || i2 != h2.end()) {
		TEST_ENSURE(i1 != h1.end(), "Should not be end");
		TEST_ENSURE(i2 != h2.end(), "Should not be end");
		TEST_ENSURE_EQUALITY(*i1, *i2, "Wrong value");
		i1++;
		i2++;
	}
	return true;
}

bool basic_bool_test() {
	tpie::bit_array hat;
  
	//Resize
	hat.resize(52, 1);
	TEST_ENSURE(hat.size() == 52, "Wrong size");
	for (size_type i=0; i < 52; ++i)
		TEST_ENSURE(hat[i] == true, "Wrong value");
  
	//Get and set
	return true;
	for (size_type i=0; i < 52; ++i)
		hat[i] = static_cast<bool>(((i * 104729)>>3) % 2);
  
	const tpie::bit_array & hat2(hat);
	for (size_type i=0; i < 52; ++i)
		TEST_ENSURE_EQUALITY(hat2[i], static_cast<bool>(((i * 104729)>>3) % 2), "Wrong value");

	TEST_ENSURE(!hat.empty(), "Empty");
	hat.resize(0);
	TEST_ENSURE(hat.empty(), "Not empty");
	bit_array a(1,0),b(4,0),c(11,0);
	a[0] = b[0] = c[0] = true;
	TEST_ENSURE(a[0] && b[0] && c[0], "Wrong value");
	a[0] = b[0] = c[0] = false;
	TEST_ENSURE(!a[0] && !b[0] && !c[0], "Wrong value");

	return true;
}


bool iterator_test() {
	array<size_t> hat;
	hat.resize(52);

	for (size_type i=0; i < 52; ++i)
		hat[i] = (i * 104729) % 2251;
	{
		array<size_t>::const_iterator i=hat.begin();
		for (size_t j=0; j < 52; ++j) {
			TEST_ENSURE(i != hat.end(), "Should not be end");
			TEST_ENSURE_EQUALITY(*i,((j * 104729) % 2251), "Wrong value");
			++i;
		}
		TEST_ENSURE(i == hat.end(), "Should be end");
	}
	{
		for (size_t j=0; j < 52; ++j) {
			array<size_t>::iterator i=hat.find(j/2)+(j-j/2);
			TEST_ENSURE(i != hat.end(), "Should not be end");
			TEST_ENSURE_EQUALITY(*i, ((j * 104729) % 2251), "Wrong value");
		}
	}
	{
		array<size_t>::reverse_iterator i=hat.rbegin();
		for (size_t j=0; j < 52; ++j) {
			TEST_ENSURE(i != hat.rend(), "Should not be rend");
			TEST_ENSURE_EQUALITY(*i, (((51-j) * 104729) % 2251), "Wrong value"); 
			++i;
		}
		TEST_ENSURE(i == hat.rend(), "Should be rend");
	}

	std::sort(hat.begin(), hat.end());

	{
		// verify order
		// find two elements in the reverse order where one is less than the other
		array<size_t>::reverse_iterator i=std::adjacent_find(hat.rbegin(), hat.rend(), std::less<size_t>());
		TEST_ENSURE(i == hat.rend(), "Should not exist");
	}
	return true;
}

bool iterator_bool_test() {
	bit_array hat;
	hat.resize(52);

	for (size_type i=0; i < 52; ++i)
		hat[i] = static_cast<bool>(((i * 104729)>>7) % 2);
	{
		bit_array::const_iterator i=hat.begin();
		for (int j=0; j < 52; ++j) {
			TEST_ENSURE(i != hat.end(), "End too soon");
			TEST_ENSURE_EQUALITY(*i, static_cast<bool>(((j * 104729)>>7) % 2), "Wrong value");
			++i;
		}
		TEST_ENSURE(i == hat.end(), "End expected");
	}
	{
		bit_array::reverse_iterator i=hat.rbegin();
		for (int j=51; j >= 0; --j) {
			TEST_ENSURE(i != hat.rend(), "End too soon");
			TEST_ENSURE_EQUALITY(*i, static_cast<bool>(((j * 104729)>>7) % 2), "Wrong value");
			++i;
		}
		TEST_ENSURE(i == hat.rend(), "Rend expected");
	}
  	std::sort(hat.begin(), hat.end());
	return true;
}

template <bool seg>
class array_memory_test: public memory_test {
public:
	array_base<int, seg> a;
	virtual void alloc() {a.resize(1024*1024*32);}
	virtual void free() {a.resize(0);}
	virtual size_type claimed_size() {
		return static_cast<size_type>(array_base<int, seg>::memory_usage(1024*1024*32));
	}
};

class array_bool_memory_test: public memory_test {
public:
	bit_array a;
	virtual void alloc() {a.resize(123456);}
	virtual void free() {a.resize(0);}
	virtual size_type claimed_size() {return static_cast<size_type>(bit_array::memory_usage(123456));}
};

bool copyempty() {
	array<char> a(0);
	array<char> b(0);
	array<char> temp = a;
	a = b;
	b = temp;
	return true;
}

bool arrayarray() {
	array<array<int> > a;
	array<int> prototype(1);
	a.resize(1, prototype);
	a.resize(0);
	return true;
}

bool frontback() {
	size_t sz = 9001;
	size_t base = 42;
	array<int> a(sz);
	for (size_t i = 0; i < sz; ++i) {
		a[i] = base+i;
	}
	TEST_ENSURE_EQUALITY(a.front(), static_cast<int>(base), "Wrong front");
	TEST_ENSURE_EQUALITY(a.back(), static_cast<int>(base+sz-1), "Wrong back");
	const array<int> & b = a;
	TEST_ENSURE_EQUALITY(b.front(), static_cast<int>(base), "Wrong front");
	TEST_ENSURE_EQUALITY(b.back(), static_cast<int>(base+sz-1), "Wrong back");
	return true;
}

int main(int argc, char **argv) {
	BOOST_CONCEPT_ASSERT((linear_memory_structure_concept<array<int> >));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<array<int>::const_iterator>));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<array<int>::const_reverse_iterator>));
	BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIterator<array<int>::iterator>));
	BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIterator<array<int>::reverse_iterator>));
	BOOST_CONCEPT_ASSERT((linear_memory_structure_concept<bit_array >));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<bit_array::const_iterator>));
	BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<bit_array::const_reverse_iterator>));

	return tpie::tests(argc, argv, 128)
		.test(basic_test, "basic")
		.test(iterator_test, "iterators")
		.test(auto_ptr_test, "auto_ptr")
		.test(array_memory_test<false>(), "memory")
		.test(segmented_array_test, "segmented")
		.test(array_memory_test<true>(), "memory_segmented")
		.test(basic_bool_test, "bit_basic")
		.test(iterator_bool_test, "bit_iterators")
		.test(array_bool_memory_test(), "bit_memory")
		.test(copyempty, "copyempty")
		.test(arrayarray, "arrayarray")
		.test(frontback, "frontback");
}
