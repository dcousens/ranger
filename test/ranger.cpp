#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <type_traits>
#include <vector>

#include "../ranger.hpp"
#include "../serial.hpp"

using namespace ranger;

template <bool text = false, typename R>
void printr (const R r) {
	for (auto s = r; not s.empty(); s.pop_front()) {
		printf(text ? "%c" : "%i ", s.front());
	}
	printf("\n");
}

void rangeTests () {
	std::array<uint8_t, 32> a;
	for (size_t i = 0; i < a.size(); ++i) a[i] = static_cast<uint8_t>((i * 2) + 1);

	auto b = range(a).drop(10);
	assert(a.size() == 32);
	assert(b.size() == 22);

	assert(b.front() == 21);
	assert(b.front() == b[0]);

	const auto c = b.drop(21);
	assert(a.size() == 32);
	assert(b.size() == 22);
	assert(c.size() == 1);
	assert(c.front() == 63);
	assert(c.back() == 63);
	assert(c[0] == 63);

	auto d = range(a);
	d[0] = 95;
	d[d.size() - 2] = 54;
	d[d.size() - 1] = 17;
	assert(d.front() == 95);
	d.front() = 96;
	assert(d.front() == 96);
	assert(d.back() == 17);
	d.back() = 11;
	assert(d.back() == 11);
	d.pop_back();
	assert(d.back() == 54);

	d = d.drop(20);
	assert(d[0] == a[20]);
}

void rangeTests2 () {
	std::vector<uint8_t> e(60);
	assert(e.size() == 60);
	const auto f = range(e).drop(10);
	assert(f.size() == 50);
	assert(e.size() == 60);

	auto g = range(e);
	assert(e.size() == g.size());

	g = g.drop(30);
	assert(g.size() == e.size() - 30);

	auto h = range(g);
	assert(h.size() == g.size());

	auto i = range(range(g));
	assert(i.size() == g.size());

	const std::array<uint8_t, 4> ccc = {0, 1, 2, 3};
	assert(range(ccc) == ccc);

	const auto rrr = range(ccc);
	assert(rrr == ccc);
	assert(rrr.size() == ccc.size());

	std::array<uint8_t, 4> yy;
	range(yy).put(range(ccc));

	const auto rrr2 = range(range(rrr));
	assert(rrr2.size() == ccc.size());
}

// README-ish
void rangeTests3 () {
	std::vector<int> numbers = {1, 2, 3};

	auto a = range(numbers).take(2); // {1, 2}
	a[1] = 8;

	auto b = a.take(2); //
	assert(a.size() == 2);
	assert(b.size() == 2);
	a = a.drop(2);
	assert(a.size() == 0);
	assert(b.size() == 2);
	assert(b[0] == numbers[0]);
	assert(b[1] == numbers[1]);
// 	assert(b[2] == numbers[2]); // b.size() == 2, fails
}

void retroTests () {
	std::array<uint8_t, 4> expected = {3, 2, 1, 0};
	std::array<uint8_t, 4> xx = {0, 1, 2, 3};
	std::array<uint8_t, 4> yy = {0};

	range(yy).put(retro(xx));
	assert(yy == expected);

	yy = {0}; // reset
	retro(yy).put(range(xx));
	assert(yy == expected);

	retro(retro(yy)).put(retro(xx));
	assert(yy == expected);

	memmove(yy.data(), xx.data(), yy.size());
	assert(yy == xx);

	retro(retro(retro(retro(retro(retro(yy)))))).put(retro(range(xx)));
	assert(yy == expected);

// 	const auto zz = yy;
// 	memmove(zz.data(), xx.data(), zz.size());

	yy = {9,9,9,9}; // reset
	auto mm = retro(yy).drop(1);
	range(mm).put(range(xx).drop(1));

	assert(yy[0] == 3);
	assert(yy[1] == 2);
	assert(yy[2] == 1);
	assert(yy[3] == 9);

	yy = {9,9,9,9}; // reset
	auto nn = retro(range(yy).drop(1));
	range(nn).put(range(xx).drop(1));

	assert(yy[0] == 9);
	assert(yy[1] == 3);
	assert(yy[2] == 2);
	assert(yy[3] == 1);

	assert(retro(range(yy)).drop(2).size() == 2);
	assert(retro(range(yy)).drop(4).size() == 0);
}

void serialTests () {
	std::array<uint8_t, 4> a = {165, 102, 42, 10};

	auto x = serial::peek<int32_t>(a);
	assert(x == ((a[0] << 0) + (a[1] << 8) + (a[2] << 16) + (a[3] << 24)));

	auto y = serial::peek<int32_t, true>(a);
	assert(y == ((a[0] << 24) + (a[1] << 16) + (a[2] << 8) + (a[3] << 0)));

	auto r = range(a);
	serial::read<int32_t>(r);
	assert(r.size() == 0);

	auto rr = range(a);
	assert(rr.size() == 4);
	serial::put<int16_t>(rr, 2048);
	assert(serial::peek<int16_t>(a) == 2048);
	assert(rr.size() == 2);

	assert(serial::read<int16_t>(range(a)) == 2048);
	serial::put<int16_t>(range(a), 420);
	assert(serial::peek<int16_t>(a) == 420);

	serial::place<int16_t>(a, 890);
	assert(serial::peek<int16_t>(a) == 890);

	uint32_t value = 0x90ffccde;
	std::array<uint8_t, 4> expected = {};
	expected[3] = static_cast<uint8_t>(value & 0xff);
	expected[3 - 1] = static_cast<uint8_t>(value >> 8);
	expected[3 - 2] = static_cast<uint8_t>(value >> 16);
	expected[3 - 3] = static_cast<uint8_t>(value >> 24);
	const auto em = serial::peek<uint32_t, true>(expected);
	assert(em == value);

	std::array<uint8_t, 4> actual = {};
	serial::place<uint32_t, true>(actual, value);
	const auto am = serial::peek<uint32_t, true>(actual);
	assert(am == value);

	printr<true>(zstr_range("foobar is foobar"));

	std::array<char, 8> s = {'\0'};
	range(s).put(zstr_range("hello"));
	printr<true>(range(s));

	std::array<uint8_t, 8> ss = {'\0'};
	auto rss = range(ss);
	for (auto x : zstr_range("hello")) {
		rss.put(static_cast<uint8_t>(x));
	}
	printr<true>(range(ss));

	assert(memcmp(expected.data(), actual.data(), actual.size()) == 0);
	printr(range(actual));
	printr(range(expected));
}

void otherUsageTests () {
	std::map<char, int> vs;
	vs['x'] = 3;
	vs['y'] = 44;
	vs['z'] = 555;

	auto rm = range(vs);
	for (auto i = rm; not i.empty(); i.pop_front()) {
		printf("%c-%i ", i.front().first, i.front().second);
	}

	for (auto v : range(vs)) {
		printf("v-%i ", v.first);
	}
	printf("\n");

	assert(rm[0].second == 3);
	assert(rm[1].second == 44);
	assert(rm[2].second == 555);
	// boom (fails on size assertion)
// 	assert(rm[4].second == 3);

	uint64_t h = 0xffff000011110000;
	std::list<uint8_t> ls;
	ls.push_back(0);
	ls.push_back(0);
	ls.push_back(0);
	ls.push_back(0);
	ls.push_back(0);
	ls.push_back(0);
	ls.push_back(0);
	ls.push_back(0);

	serial::place<uint64_t>(range(ls), h);
	// compare, byte for byte again h
	size_t i = 0;
	for (auto x = ls.begin(); x != ls.end(); ++x) {
		auto hp = reinterpret_cast<uint8_t*>(&h);
		assert(hp[i] == *x);
		++i;
	}
}

void overloadTests () {
// 	std::array<uint8_t, 10> x;
	std::vector<uint8_t> x = {1, 2, 3, 4};
	auto xr = ptr_range(x);
	assert(xr.size() == 4);
	assert(xr.back() == 4);
	assert(xr.begin() == xr.data());

// 	range(x).data(); // FAILS :)
	memmove(xr.data(), xr.data(), xr.size()); // OK

	auto rxr = retro(xr);
	while (not rxr.empty()) {
		assert(rxr.back() == xr.front());

		xr.pop_front();
		rxr.pop_back();
	}

	assert(xr.size() == 0);
	assert(rxr.size() == 0);

	// TODO
// 	xr.put(10);
}

void orderedTests () {
	std::vector<uint8_t> x = {1, 2, 3, 4};

	auto s = ordered(x);
	assert(s.contains(1));
	assert(s.contains(3));
	assert(s.contains(4));
	assert(not s.contains(0));
	assert(not s.contains(5));
	assert(s.lower_bound(5) == s.end());

	// !!! invalidates `s`!
	assert(x.begin() == s.begin());
	x.emplace(s.lower_bound(5), 240);
	assert(x.begin() != s.begin());

	// safe again
	s = ordered(x);
	assert(x.begin() == s.begin());

	const auto z = ordered(x, [](auto a, auto b) {
		return a < b;
	});
	assert(s == z);

	assert(ordered(range(x), [](auto a, auto b) {
		return a < b;
	}) == s);
}

void putTests () {
	std::vector<uint32_t> x = {1, 2, 3, 4};
	auto save = range(x);

	save.put(11u);
	save.put(9u);
	save.put(7u);
	save.put(5u);
	assert(x[0] == 11);
	assert(x[1] == 9);
	assert(x[2] == 7);
	assert(x[3] == 5);
}

#include <iostream>
#include <iterator>
#include <sstream>

void iterTests () {
	std::stringstream ss{"5 7 9"};
	std::istream_iterator<int> start{ss};
	std::istream_iterator<int> end;

	auto a = iter_range(start, end);

// 	printf("%zu\n", a.size()); // destructive!
	assert(a.front() == 5);
	a.pop_front();
	assert(a.front() == 7);
	a.pop_front();
	assert(a.front() == 9);
	a.pop_front();
	assert(a.empty());
// 	a.pop_front(); // throws

	auto b = iter_range(start, end);
	assert(b.front() == 5); // cached by 'start' iterator...
	b.pop_front();
	assert(b.empty()); // stringstream is depleted
}

int main () {
	rangeTests();
	rangeTests2();
	rangeTests3();
	iterTests();
	retroTests();
	putTests();
	orderedTests();
	serialTests();
	otherUsageTests();
	overloadTests();
	orderedTests();

	return 0;
}
