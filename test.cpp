#include <array>
#include <cassert>
#include <cstring>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <sstream>
#include <type_traits>
#include <vector>

#include "ranger.hpp"
#include "serial.hpp"

using namespace ranger;

template <bool text = false, typename R>
void printr (R r) {
	while (not r.empty()) {
		printf(text ? "%c" : "%i ", r.front());
		r.pop_front();
	}
	printf("\n");
}

void rangeTests () {
	auto a = std::array<int, 32>{};
	for (size_t i = 0; i < a.size(); ++i) a[i] = static_cast<int>((i * 2) + 1);

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
	assert(e.empty()); // thats OK
	// e.front(); // assert or U/B
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
	assert(range(yy) == ccc);

	const auto rrr2 = range(range(rrr));
	assert(rrr2.size() == ccc.size());

	auto yyr = range(yy);
	assert(yyr.size() == 4);
	yyr.put(range(ccc));
	assert(yyr.size() == 0);
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

// signed distance tests
void signedDistanceTests () {
	auto numbers = std::vector<int>{1, 2, 3};

	assert(range(numbers).drop(10).empty());
	assert(range(numbers).drop(10).end() == numbers.end());
	assert(range(numbers).drop_back(10).empty());
	assert(range(numbers).drop_back(10).end() == numbers.begin());

	assert(range(numbers).drop(10).take(10).empty());
	assert(range(numbers).drop(10).take(10).end() == numbers.end());

	assert(range(numbers).take_back(3).size() == 3);
	assert(range(numbers).take_back(10).size() == 3);
	assert(range(numbers).take_back(10) == range(numbers));
	assert(range(numbers).take_back(1) == range(numbers).drop(2));
	assert(range(numbers).take_back(2) == range(numbers).drop(1));

	// Wconversion warnings, and inner assert fails
// 	assert(range(numbers).drop_back(-10).empty());
// 	assert(range(numbers).drop(-10).empty());
}

void untilTests () {
	auto numbers = std::vector<int>{1, 2, 3, 4, 5, 6};

	auto du3 = range(numbers).drop_until([](auto x) { return x == 3; });
	assert(du3[0] == 3);
	assert(du3.size() == 4);

	auto du4 = range(numbers).drop_until([](auto x) { return x == 4; });
	assert(du4[0] == 4);
	assert(du4.size() == 3);

	auto dbu3 = range(numbers).drop_back_until([](auto x) { return x == 3; });
	assert(dbu3 == range(numbers).take(3));

	auto dbu4 = range(numbers).drop_back_until([](auto x) { return x == 4; });
	assert(dbu4 == range(numbers).take(4));

	auto tu4 = range(numbers).take_until([](auto x) { return x == 5; });
	assert(tu4[0] == 1);
	assert(tu4.size() == 4);

	auto tbu3 = range(numbers).take_back_until([](auto x) { return x == 3; });
	assert(tbu3 == range(numbers).drop(3));

	auto tbu4 = range(numbers).take_back_until([](auto x) { return x == 4; });
	assert(tbu4 == range(numbers).drop(4));

	auto tt45 = range(numbers)
		.take_back_until([](auto x) { return x == 2; })
		.take_until([](auto x) { return x == 6; })
		.drop_until([](auto x) { return x == 4; });

	assert(tt45 == range(numbers).drop(3).take(2));
}

void popTests () {
	auto numbers = std::vector<int>{1, 2, 3, 4, 5, 6};

	auto a = range(numbers);
	static_assert(decltype(a)::is_forward::value);
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

	auto d = range(numbers);
	auto df = d.pop_until([](auto x) { return x == 9; }); // doesn't exist

	assert(df == range(numbers));
	assert( d.empty());
	assert( d == range(numbers).drop(6));
	assert( d.drop(1000000).empty());

	d.pop_front(0xffffffff);
	d.pop_front(0x7ffffffffffffffe);
	d.pop_front(0x7fffffffffffffff);
// 	d.pop_front(0x8000000000000000); // assert or U/B, exceeds ssize_t
	assert( d.empty());
	assert( d == range(numbers).drop(6)); // unchanged

	auto e = range(numbers);
	auto ef = e.pop_back_until([](auto x) { return x == 4; });
	assert(e  == range(numbers).take(4));
	assert(ef == range(numbers).drop(4));

	auto f = range(numbers);
	auto ff = f.pop_back_until([](auto x) { return x == 9; }); // doesn't exist
	assert(f  == range(numbers).take(0));
	assert(ff == range(numbers).drop(0));
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
#pragma pack()

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

	// fails, map has no random access iteration
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
	// assert(range(f).back()); // error: no matching member function
	// assert(range(f).size() == 3); // error: no matching member function
}

void overloadTests () {
	auto v = std::vector<uint8_t>{1, 2, 3, 4};
	auto vp = ptr_range(v);
	assert(vp.size() == 4);
	assert(vp.back() == 4);
	assert(vp.begin() == vp.data());
	assert(vp.data() == v.data());

	// range(v).data(); // error: no matching member function
	memmove(vp.data(), vp.data(), vp.size()); // OK

	auto vpu = range(v.data(), v.data() + v.size());
	assert(vpu.size() == 4);
	assert(vpu.back() == 4);
	assert(vpu.begin() == vpu.data());
	assert(vpu.data() == v.data());

	auto rvp = reverse(vp);
	while (not rvp.empty()) {
		assert(rvp.back() == vp.front());

		vp.pop_front();
		rvp.pop_back();
	}

	assert(vp.size() == 0);
	assert(rvp.size() == 0);

	auto vp2 = range(v);
	vp2.put(static_cast<uint8_t>(10));
	assert(range(std::array{10, 2, 3, 4}) == v);
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

	const auto l = ordered<std::less<>>(v);
	assert(l == s);
	assert(l.contains(1));
	assert(l.contains(5));
	assert(not l.contains(0));
	assert(not l.contains(6));
	assert(l.lower_bound(6) == l.end());
	assert(*l.lower_bound(5) == l.back());

	std::sort(v.begin(), v.end(), std::greater<>());
	// l behaviour is undefined now

	const auto g = ordered<std::greater<>>(v);
	assert(g.contains(1));
	assert(g.contains(5));
	assert(not g.contains(0));
	assert(not g.contains(6));
	assert(g.lower_bound(6) == g.begin());
	assert(*g.lower_bound(5) == g.front());
}

void putTests () {
	auto v = std::vector<uint32_t>{1, 2, 3, 4};
	auto save = range(v);
	save.put(11u);
	save.put(9u);
	save.put(7u);
	save.put(5u);
	assert(save.empty());

	assert(range(std::array{11, 9, 7, 5}) == v);
	assert(range(std::array{11, 9}) == range(v).take(2));
}

void iterTests () {
	auto stream = std::stringstream{"5 7 9"};
	auto begin = std::istream_iterator<int>{stream};
	auto end = std::istream_iterator<int>{};

	auto a = range(begin, end);

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
	// a.front(); // assert or U/B

	auto b = range(begin, end);
	assert(b.front() == 5); // cached by 'begin' iterator...
	b.pop_front();
	assert(b.empty()); // stringstream is depleted

	auto streamc = std::stringstream{"5 7 9"};
	auto c = range(std::istream_iterator<int>{streamc}, std::istream_iterator<int>{});
	assert(streamc.good());

	c.pop_front(21);
	assert(c.empty());
	assert(streamc.eof());

	auto streamd = std::stringstream{"8 9 10"};
	auto d = input_range(std::istream_iterator<int>{streamd});
	assert(streamd.good());
	assert(not streamd.eof());
	d.pop_front(4);

	assert(d.empty());
	assert(not streamd.good());
	assert(streamd.eof());
	// d.front(); // assert or U/B
}

void nullTests () {
	auto a = range<int*>(nullptr, nullptr);
	assert(a.empty());
	assert(a.size() == 0);
	assert(a.take(1).empty());
	assert(a.take(1).size() == 0);
	// a.front(); // assert or U/B
}

void algoTests () {
	auto v = std::vector<uint8_t>{1, 2, 3, 4};
	auto va = range(v);

	assert(va.count([](auto x) { return x == 3; }) == 1);
	assert(va.count([](auto x) { return x > 0 or x < 4; }) == 4);
	assert(va.count([](auto x) { return x == 6; }) == 0);

	assert(va.any([](auto x) { return x == 3; }));
	assert(not va.any([](auto x) { return x == 6; }));

	assert(va.all([](auto x) { return x > 0 or x < 4; }));
	assert(not va.all([](auto x) { return x == 3; }));

	auto nothing = range<int*>(nullptr, nullptr);
	assert(nothing.all([](auto) { return false; }));
	assert(not nothing.any([](auto) { return true; }));
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
	signedDistanceTests();
	nullTests();
	algoTests();

	return 0;
}
