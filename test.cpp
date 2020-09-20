#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <list>
#include <forward_list>
#include <map>
#include <type_traits>
#include <vector>

#include "ranger.hpp"
#include "serial.hpp"

using namespace ranger;

template <bool text = false, typename R>
void printr (const R r) {
	for (auto s = r; not s.empty(); s.pop_front()) {
		printf(text ? "%c" : "%i ", s.front());
	}
	printf("\n");
}

void rangeTests () {
	auto a = std::array<uint8_t, 32>{};
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

	auto e = d.drop(100); // oh no! past the end
	assert(e.empty());
// 	e.front(); // undefined behaviour!
}

void rangeTests2 () {
	auto e = std::vector<uint8_t>(60);
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

	const auto ccc = std::array<uint8_t, 4>{0, 1, 2, 3};
	assert(range(ccc) == ccc);

	const auto rrr = range(ccc);
	assert(rrr == ccc);
	assert(rrr.size() == ccc.size());

	auto yy = std::array<uint8_t, 4>{};
	range(yy).put(range(ccc));

	const auto rrr2 = range(range(rrr));
	assert(rrr2.size() == ccc.size());
}

// README-ish
void rangeTests3 () {
	auto numbers = std::vector<int>{1, 2, 3};

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

void untilTests () {
	auto numbers = std::vector<int>{1, 2, 3, 4, 5, 6};

	auto du3 = range(numbers).drop_until([](auto x) { return x == 3; });
	assert(du3[0] == 3);
	assert(du3.size() == 4);

	auto du4 = range(numbers).drop_until([](auto x) { return x == 4; });
	assert(du4[0] == 4);
	assert(du4.size() == 3);

	auto tu4 = range(numbers).take_until([](auto x) { return x == 5; });
	assert(tu4[0] == 1);
	assert(tu4.size() == 4);
}

void popTests () {
	auto numbers = std::vector<int>{1, 2, 3, 4, 5, 6};

	auto a = range(numbers);
	auto af = a.pop_front(3);

	assert(af == range(numbers).take(3));
	assert( a == range(numbers).drop(3));

	auto b = range(numbers);
	auto bb = b.pop_back(3);

	assert(bb == range(numbers).drop(3));
	assert( b == range(numbers).take(3));

	auto c = range(numbers);
	auto cf = c.pop_until([](auto x) { return x == 4; });

	assert(cf == range(numbers).take(3));
	assert( c == range(numbers).drop(3));
}

void reverseTests () {
	auto expected = std::array<uint8_t, 4>{3, 2, 1, 0};
	auto xx = std::array<uint8_t, 4>{0, 1, 2, 3};
	auto yy = std::array<uint8_t, 4>{0};

	range(yy).put(reverse(xx));
	assert(yy == expected);

	yy = {0}; // reset
	reverse(yy).put(range(xx));
	assert(yy == expected);

	reverse(reverse(yy)).put(reverse(xx));
	assert(yy == expected);

	memmove(yy.data(), xx.data(), yy.size());
	assert(yy == xx);

	reverse(reverse(reverse(reverse(reverse(reverse(yy)))))).put(reverse(range(xx)));
	assert(yy == expected);

// 	const auto zz = yy;
// 	memmove(zz.data(), xx.data(), zz.size());

	yy = {9,9,9,9}; // reset
	auto mm = reverse(yy).drop(1);
	range(mm).put(range(xx).drop(1));

	assert(yy[0] == 3);
	assert(yy[1] == 2);
	assert(yy[2] == 1);
	assert(yy[3] == 9);

	yy = {9,9,9,9}; // reset
	auto nn = reverse(range(yy).drop(1));
	range(nn).put(range(xx).drop(1));

	assert(yy[0] == 9);
	assert(yy[1] == 3);
	assert(yy[2] == 2);
	assert(yy[3] == 1);

	assert(reverse(range(yy)).drop(2).size() == 2);
	assert(reverse(range(yy)).drop(4).size() == 0);
}

#pragma pack(1)
struct Foo {
	uint8_t a, b, c, d;

	bool operator == (const Foo x) const {
		return a == x.a and b == x.b and c == x.c and d == x.d;
	}
};

void serialTests () {
	const auto aaaa = Foo{ 165, 102, 42, 10 };
	auto a = std::array<uint8_t, 4>{165, 102, 42, 10};

	auto x = serial::peek<int32_t>(a);
	assert(x == ((a[0] << 0) + (a[1] << 8) + (a[2] << 16) + (a[3] << 24)));

	auto y = serial::peek<int32_t, true>(a);
	assert(y == ((a[0] << 24) + (a[1] << 16) + (a[2] << 8) + (a[3] << 0)));

	auto z = serial::peek<Foo>(a);
	assert(z == aaaa);

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
	auto expected = std::array<uint8_t, 4>{};
	expected[3] = static_cast<uint8_t>(value & 0xff);
	expected[3 - 1] = static_cast<uint8_t>(value >> 8);
	expected[3 - 2] = static_cast<uint8_t>(value >> 16);
	expected[3 - 3] = static_cast<uint8_t>(value >> 24);
	const auto em = serial::peek<uint32_t, true>(expected);
	assert(em == value);

	auto actual = std::array<uint8_t, 4>{};
	serial::place<uint32_t, true>(actual, value);
	const auto am = serial::peek<uint32_t, true>(actual);
	assert(am == value);

	printr<true>(zstr_range("foobar is foobar"));

	auto s = std::array<char, 8>{'\0'};
	range(s).put(zstr_range("hello"));
	printr<true>(range(s));

	auto ss = std::array<uint8_t, 8>{'\0'};
	auto rss = range(ss);
	for (auto c : zstr_range("hello")) {
		rss.put(static_cast<uint8_t>(c));
	}
	printr<true>(range(ss));

	assert(memcmp(expected.data(), actual.data(), actual.size()) == 0);
	printr(range(actual));
	printr(range(expected));
}

void otherUsageTests () {
	auto vs = std::map<char, int>{};
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

	// fails, map has no random access iterationc
// 	assert(rm[0].second == 3);
// 	assert(rm[1].second == 44);
// 	assert(rm[2].second == 555);
	// boom (fails on size assertion)
// 	assert(rm[4].second == 3);

	uint64_t h = 0xffff000011110000;
	auto ls = std::list<uint8_t>{};
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

	std::forward_list<uint8_t> f;
	f.push_front(1);
	f.push_front(2);
	f.push_front(3);

	assert(range(f) == f);
	assert(not f.empty());
	assert(range(f).front() == 3);
	assert(range(f).drop(1).front() == 2);
	assert(range(f).drop(2).front() == 1);
	assert(range(f).drop(3).empty());
// 	assert(range(f).back()); // should fail instantiation
// 	assert(range(f).size() == 3); // no matching call
}

void overloadTests () {
// 	std::array<uint8_t, 10> v;
	auto v = std::vector<uint8_t>{1, 2, 3, 4};
	auto vr = ptr_range(v);
	assert(vr.size() == 4);
	assert(vr.back() == 4);
	assert(vr.begin() == vr.data());
	assert(vr.data() == v.data());

// 	range(v).data(); // FAILS :)
	memmove(vr.data(), vr.data(), vr.size()); // OK

	auto vru = range(v.data(), v.data() + v.size());
	assert(vru.size() == 4);
	assert(vru.back() == 4);
	assert(vru.begin() == vru.data());
	assert(vru.data() == v.data());

	auto rvr = reverse(vr);
	while (not rvr.empty()) {
		assert(rvr.back() == vr.front());

		vr.pop_front();
		rvr.pop_back();
	}

	assert(vr.size() == 0);
	assert(rvr.size() == 0);

	// TODO
// 	vr.put(10);
}

struct less {
	template <typename T>
	bool operator() (T a, T b) {
		return a < b;
	}
};

void orderedTests () {
	auto v = std::vector<uint8_t>{1, 2, 3, 4};

	auto s = ordered(v);
	assert(s.contains(1));
	assert(s.contains(3));
	assert(s.contains(4));
	assert(not s.contains(0));
	assert(not s.contains(5));
	assert(s.lower_bound(5) == s.end());

	assert(v.begin() == s.begin());
	// WARN: invalidates `s`
	v.emplace(s.lower_bound(5), 5);
	assert(v.begin() != s.begin());

	// safe again
	s = ordered(v);
	assert(v.begin() == s.begin());

	const auto z = ordered<less>(v);
// 	assert(z == s);
	assert(z.contains(1));
	assert(z.contains(5));
	assert(not z.contains(0));
	assert(not z.contains(6));
	assert(z.lower_bound(6) == z.end());
	assert(*z.lower_bound(5) == z.back());
}

void putTests () {
	auto v = std::vector<uint32_t>{1, 2, 3, 4};
	auto save = range(v);

	save.put(11u);
	save.put(9u);
	save.put(7u);
	save.put(5u);
	assert(v[0] == 11);
	assert(v[1] == 9);
	assert(v[2] == 7);
	assert(v[3] == 5);
}

#include <iostream>
#include <iterator>
#include <sstream>

void iterTests () {
	auto stream = std::stringstream{"5 7 9"};
	auto start = std::istream_iterator<int>{stream};
	auto end = std::istream_iterator<int>{};

	auto a = range(start, end);

// 	const auto d = { 5, 7, 9 };
// 	assert(a == range(d)); // cannot, requires forward_iterator
// 	printf("%zu\n", a.size()); // destructive!
	assert(stream.good());
	assert(a.front() == 5);
	a.pop_front();
	assert(stream.good());
	assert(a.front() == 7);
	a.pop_front();
	assert(stream.eof());
	assert(a.front() == 9);
	a.pop_front();
	assert(a.empty());
// 	a.pop_front(); // undefined behavior

	auto b = range(start, end);
	assert(b.front() == 5); // cached by 'start' iterator...
	b.pop_front();
	assert(b.empty()); // stringstream is depleted

	auto streamb = std::stringstream{"5 7 9"};
	auto c = range(std::istream_iterator<int>{streamb}, std::istream_iterator<int>{});
	assert(streamb.good());

	auto d = c.drop(1); // drops the front
	assert(d.front() == 7);
	assert(d.front() == 7); // cached by 'start' iterator...

	auto e = c.drop(20);
	assert(e.empty());

	d.pop_front();
	assert(d.empty()); // depleted by e
	assert(streamb.eof());
}

int main () {
	rangeTests();
	rangeTests2();
	rangeTests3();
	popTests();
	untilTests();
	iterTests();
	reverseTests();
	putTests();
	orderedTests();
	serialTests();
	otherUsageTests();
	overloadTests();
	orderedTests();

	return 0;
}
