#include <array>
#include <cstring>
#include <cstdint>
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
#include "compat.hpp"

using namespace ranger;

static auto indent = 0;
template <typename F>
auto describe (std::string const description, F const f) {
	for (auto i = indent; i > 0; --i) std::cerr << ' ';
	indent += 1;
	std::cerr << description << std::endl;
	f([](bool const condition, std::string const test = "") {
		if (condition) return;

		for (auto i = indent; i > 0; --i) std::cerr << ' ';
		std::cerr << "fail";
		if (not test.empty()) std::cerr << " " << test;
		std::cerr << std::endl;
		if (not condition) exit(1);
	});
	indent -= 1;
}

auto const S123 = std::array{1, 2, 3};
auto const S1234 = std::array{1, 2, 3, 4};
auto const S123456 = std::array{1, 2, 3, 4, 5, 6};
auto const S1234567 = std::array{1, 2, 3, 4, 5, 6, 7};
auto const TQBFJ = zstr_range("the quick brown fox jumped");

int main () {
describe("drop / take", [&](auto test) {
	test(range(S1234567).drop(0).size() == 7 - 0);
	test(range(S1234567).drop(3).size() == 7 - 3);
	test(range(S1234567).drop(4).size() == 7 - 4);
	test(range(S1234567).drop(5).size() == 7 - 5);
	test(range(S1234567).drop(7).size() == 7 - 7);

	test(range(S1234567).take(0).size() == 0);
	test(range(S1234567).take(5).size() == 5);
	test(range(S1234567).take(6).size() == 6);
	test(range(S1234567).take(7).size() == 7);

	test(range(S1234567).take(3) == S123);
	test(range(S1234567).take(4) == S1234);
	test(range(S1234567).take(6) == S123456);
	test(range(S1234567).take(6) != S1234567);
	test(range(S1234567).take(7) == S1234567);

	test(range(S1234567).drop_back(0) != S123);
	test(range(S1234567).drop_back(0) != S1234);
	test(range(S1234567).drop_back(0) != S123456);
	test(range(S1234567).drop_back(0) == S1234567);

	test(range(S1234567).drop_back(4) == S123);
	test(range(S1234567).drop_back(3) == S1234);
	test(range(S1234567).drop_back(1) == S123456);
});

describe("size", [&](auto test) {
	test(range(S123).size() == 3);
	test(range(S1234).size() == 4);
	test(range(S123456).size() == 6);
});

describe("operators", [&](auto test) {
	auto const a = range(S1234);
	auto const b = range(std::array{1, 2, 3, 5});
	auto const c = std::array{1, 2, 3, 5};

	test(a < b);
	test(b > a);
	test(a == a);
	test(a != b);
	test(b != a);
	test(b == b);

	test(a < c);
// 		test(c > a);
	test(a == a);
	test(a != c);
// 		test(c != a);
	test(c == c);

	// lexicographical_compare
	{
		auto const min = range(std::array{1,5,0,0});
		auto const max = range(std::array{3,4,0,0});

		for (auto const value : {
			std::array{1,6,0,0},
			std::array{2,4,0,0},
			std::array{3,3,9,9},
		}) {
			test(range(value) > min);
			test(range(value) < max);
		}

		test(range(std::array{5,6,0}) > min);
		test(range(std::array{2,1,0,0,0}) < max);
	}
});

describe("various", [](auto test) {
	auto e = std::vector<uint8_t>(60);
	test(e.size() == 60);
	const auto f = range(e).drop(10);
	test(f.size() == 50);
	test(e.size() == 60);

	auto g = range(e);
	test(e.size() == g.size());

	g = g.drop(30);
	test(g.size() == e.size() - 30);

	auto h = range(g);
	test(h.size() == g.size());

	auto i = range(range(g));
	test(i.size() == g.size());

	const auto ccc = std::array<uint8_t, 4>{0, 1, 2, 3};
	test(range(ccc) == ccc);

	const auto rrr = range(ccc);
	test(rrr == ccc);
	test(rrr.size() == ccc.size());

	auto yy = std::array<uint8_t, 4>{};
	range(yy).put(range(ccc));
	test(range(yy) == ccc);

	const auto rrr2 = range(range(rrr));
	test(rrr2.size() == ccc.size());

	auto yyr = range(yy);
	test(yyr.size() == 4);
	yyr.put(range(ccc));
	test(yyr.size() == 0);
});

describe("until", [](auto test) {
	auto numbers = std::vector<int>{1, 2, 3, 4, 5, 6};

	auto du3 = range(numbers).drop_until([](auto x) { return x == 3; });
	test(du3[0] == 3);
	test(du3.size() == 4);

	auto du4 = range(numbers).drop_until([](auto x) { return x == 4; });
	test(du4[0] == 4);
	test(du4.size() == 3);

	auto dbu3 = range(numbers).drop_back_until([](auto x) { return x == 3; });
	test(dbu3 == range(numbers).take(3));

	auto dbu4 = range(numbers).drop_back_until([](auto x) { return x == 4; });
	test(dbu4 == range(numbers).take(4));

	auto tu4 = range(numbers).take_until([](auto x) { return x == 5; });
	test(tu4[0] == 1);
	test(tu4.size() == 4);

	auto tbu3 = range(numbers).take_back_until([](auto x) { return x == 3; });
	test(tbu3 == range(numbers).drop(3));

	auto tbu4 = range(numbers).take_back_until([](auto x) { return x == 4; });
	test(tbu4 == range(numbers).drop(4));

	auto tt45 = range(numbers)
		.take_back_until([](auto x) { return x == 2; })
		.take_until([](auto x) { return x == 6; })
		.drop_until([](auto x) { return x == 4; });

	test(tt45 == range(numbers).drop(3).take(2));
});

describe("reverse", [](auto test) {
	auto expected = std::array<uint8_t, 4>{3, 2, 1, 0};
	auto xx = std::array<uint8_t, 4>{0, 1, 2, 3};
	auto yy = std::array<uint8_t, 4>{0};

	range(yy).put(reverse(xx));
	test(yy == expected);

	yy = {0}; // reset
	reverse(yy).put(range(xx));
	test(yy == expected);

	reverse(reverse(yy)).put(reverse(xx));
	test(yy == expected);

	memmove(yy.data(), xx.data(), yy.size());
	test(yy == xx);

	reverse(reverse(reverse(reverse(reverse(reverse(yy)))))).put(reverse(range(xx)));
	test(yy == expected);

// 	const auto zz = yy;
// 	memmove(zz.data(), xx.data(), zz.size());

	yy = {9,9,9,9}; // reset
	auto mm = reverse(yy).drop(1);
	range(mm).put(range(xx).drop(1));

	test(yy[0] == 3);
	test(yy[1] == 2);
	test(yy[2] == 1);
	test(yy[3] == 9);

	yy = {9,9,9,9}; // reset
	auto nn = reverse(range(yy).drop(1));
	range(nn).put(range(xx).drop(1));

	test(yy[0] == 9);
	test(yy[1] == 3);
	test(yy[2] == 2);
	test(yy[3] == 1);

	test(reverse(range(yy)).drop(2).size() == 2);
	test(reverse(range(yy)).drop(4).size() == 0);
});

#pragma pack(push, 1)
struct Foo {
	uint8_t a, b, c, d;

	bool operator == (const Foo x) const {
		return a == x.a and b == x.b and c == x.c and d == x.d;
	}
};
#pragma pack(pop)

// TODO: re-do
describe("serial", [](auto test) {
	const auto aaaa = Foo{ 165, 102, 42, 10 };
	auto a = std::array<uint8_t, 4>{165, 102, 42, 10};

	auto x = serial::peek<int32_t>(a);
	test(x == ((a[0] << 0) | (a[1] << 8) | (a[2] << 16) | (a[3] << 24)));

	auto y = serial::peek<int32_t, true>(a);
	test(y == ((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | (a[3] << 0)));

	auto z = serial::peek<Foo>(a);
	test(z == aaaa);

	auto r = range(a);
	serial::read<int32_t>(r);
	test(r.size() == 0);

	auto rr = range(a);
	test(rr.size() == 4);
	serial::put<int16_t>(rr, 2048);
	test(serial::peek<int16_t>(a) == 2048);
	test(rr.size() == 2);

	test(serial::read<int16_t>(range(a)) == 2048);
	serial::put<int16_t>(range(a), 420);
	test(serial::peek<int16_t>(a) == 420);

	serial::place<int16_t>(a, 890);
	test(serial::peek<int16_t>(a) == 890);

	uint32_t value = 0x90ffccde;
	auto expected = std::array<uint8_t, 4>{};
	expected[3] = static_cast<uint8_t>(value & 0xff);
	expected[3 - 1] = static_cast<uint8_t>(value >> 8);
	expected[3 - 2] = static_cast<uint8_t>(value >> 16);
	expected[3 - 3] = static_cast<uint8_t>(value >> 24);
	const auto em = serial::peek<uint32_t, true>(expected);
	test(em == value);

	auto actual = std::array<uint8_t, 4>{};
	serial::place<uint32_t, true>(actual, value);
	const auto am = serial::peek<uint32_t, true>(actual);
	test(am == value);

	auto s = std::array<char, 8>{'\0'};
	range(s).put(zstr_range("hello"));

	auto ss = std::array<uint8_t, 8>{'\0'};
	auto rss = range(ss);
	for (auto c : zstr_range("hello")) {
		rss.put(static_cast<uint8_t>(c));
	}

	test(memcmp(expected.data(), actual.data(), actual.size()) == 0);
});

describe("other containers", [](auto) {
	describe("map", [](auto test) {
		auto map = std::map<char, int>{};
		map['x'] = 3;
		map['y'] = 44;
		map['z'] = 555;

		test(range(map) == std::array{
			std::pair<const char, int>{ 'x', 3 },
			std::pair<const char, int>{ 'y', 44 },
			std::pair<const char, int>{ 'z', 555 },
		});

		test(range(map).drop(0).front().second == 3);
		test(range(map).drop(2).front().second == 555);
	});

	describe("list", [](auto test) {
		auto l = std::list<uint8_t>{};
		l.push_back(0);
		l.push_back(0);
		l.push_back(0);
		l.push_back(0);
		l.push_back(0);
		l.push_back(0);
		l.push_back(0);
		l.push_back(0);

		uint64_t const h = 0xffff007711550033;
		serial::place<uint64_t, true>(range(l), h);

		test(range(l) == range(std::array{
			0xff, 0xff, 0x00, 0x77, 0x11, 0x55, 0x00, 0x33
		}));
	});

	describe("forward list", [](auto test) {
		std::forward_list<uint8_t> f;
		f.push_front(1);
		f.push_front(2);
		f.push_front(3);

		test(range(f) == f);
		test(not f.empty());
		test(range(f).front() == 3);
		test(range(f).drop(1).front() == 2);
		test(range(f).drop(2).front() == 1);
		test(range(f).drop(3).empty());
	});
});

describe("ordered_range", [](auto test) {
	auto v = std::vector<int>{1, 2, 3, 4};

	auto s = ordered(v);
	test(s.contains(1));
	test(s.contains(3));
	test(s.contains(4));
	test(not s.contains(0));
	test(not s.contains(5));
	test(s.lower_bound(5) == s.end());

	test(v.begin() == s.begin());
	// WARN: invalidates `s`
	v.emplace(s.lower_bound(5), 5);
	test(v.begin() != s.begin());

	// safe again
	s = ordered(v);
	test(v.begin() == s.begin());

	const auto l = ordered<std::less<>>(v);
	test(l == s);
	test(l.contains(1));
	test(l.contains(5));
	test(not l.contains(0));
	test(not l.contains(6));
	test(l.lower_bound(6) == l.end());
	test(*l.lower_bound(5) == l.back());

	std::sort(v.begin(), v.end(), std::greater<>());
	// l behaviour is undefined now

	const auto g = ordered<std::greater<>>(v);
	test(g.contains(1));
	test(g.contains(5));
	test(not g.contains(0));
	test(not g.contains(6));
	test(g.lower_bound(6) == g.begin());
	test(*g.lower_bound(5) == g.front());
});

describe("ptr_range", [](auto) {
	describe("nullptr", [](auto) {
		auto a = range<int*>(nullptr, nullptr);
		assert(a.empty());
		assert(a.size() == 0);
		assert(a.take(1).empty());
		assert(a.take(1).size() == 0);
		assert(a.take(10).empty());
		assert(a.take(10).size() == 0);
		// a.front(); // assert or U/B
	});
});

describe("pop_*", [](auto test) {
	auto const numbers = S123456;

	auto a = range(numbers);
	static_assert(decltype(a)::is_forward::value);
	auto af = a.pop_front(3);

	test(af == range(numbers).take(3));
	test( a == range(numbers).drop(3));

	auto b = range(numbers);
	auto bb = b.pop_back(3);

	test(bb == range(numbers).drop(3));
	test( b == range(numbers).take(3));

	auto c = range(numbers);
	auto cf = c.pop_until([](auto x) { return x == 4; });

	test(cf == range(numbers).take(3));
	test( c == range(numbers).drop(3));

	auto d = range(numbers);
	auto df = d.pop_until([](auto x) { return x == 9; }); // doesn't exist

	test(df == range(numbers));
	test( d.empty());
	test( d == range(numbers).drop(6));
	test( d.drop(1000000).empty());

	d.pop_front(0xffffffff);
	d.pop_front(0x7ffffffffffffffe);
	d.pop_front(0x7fffffffffffffff);
// 	d.pop_front(0x8000000000000000); // assert or U/B, exceeds ssize_t
	test( d.empty());
	test( d == range(numbers).drop(6)); // unchanged

	auto e = range(numbers);
	auto ef = e.pop_back_until([](auto x) { return x == 4; });
	test(e  == range(numbers).take(4));
	test(ef == range(numbers).drop(4));

	auto f = range(numbers);
	auto ff = f.pop_back_until([](auto x) { return x == 9; }); // doesn't exist
	test(f  == range(numbers).take(0));
	test(ff == range(numbers).drop(0));
});

describe("input_iterators", [](auto) {
	describe("input_range", [&](auto test){
		auto stream = std::stringstream{"5 7 9"};
		auto a = input_range(std::istream_iterator<int>{stream});
		test(stream.good());
		test(a.front() == 5);
		a.pop_front();
		test(stream.good());
		test(a.front() == 7);
		a.pop_front();
		test(not stream.good());
		test(stream.eof());
		test(a.front() == 9);
		a.pop_front();
		test(a.empty());
		// a.front(); // test or U/B
	});

	describe("iterator lazyness", [&](auto test) {
		auto stream = std::stringstream{"5 7 9"};
		auto a = input_range(std::istream_iterator<int>{stream});

		// exhaust the stream
		range(a).pop_front(10); // >3
		test(not stream.good());
		test(stream.eof());

		test(a.front() == 5); // cached
		a.pop_front();
		test(a.empty()); // actually exhausted
	});

	describe("operator== (no multi-pass)", [&](auto test) {
		auto stream = std::stringstream{"5 7 9"};
		auto a = input_range(std::istream_iterator<int>{stream});

		test(a == range(std::array{5, 7, 9}));
		test(a.front() == 5); // cached
		a.pop_front();
		test(a.empty()); // actually exhausted

		test(not stream.good());
		test(stream.eof());
	});

	describe("operator< (no multi-pass)", [&](auto test) {
		auto stream = std::stringstream{"5 7 9"};
		auto a = input_range(std::istream_iterator<int>{stream});

		test(a < range(std::array{5, 8, 0}));
		test(a.front() == 5); // cached
		a.pop_front();

		test(a.front() == 9); // what operator< reached
		a.pop_front();
		test(a.empty()); // exhausted
	});
});

describe("put", [](auto) {
	describe("element-wise", [&](auto test) {
		auto data = std::array{1, 2, 3, 4};
		auto a = range(data);
		test(a.put(11));
		test(a.put(9));
		test(a.put(7));
		test(a.put(5));
		test(a.empty());
		test(not a.put(5));
		test(data == std::array{11, 9, 7, 5});
	});

	describe("range", [&](auto test) {
		auto data = std::array{1, 2, 3, 4};
		auto a = range(data);
		test(a.put(range(std::array{11, 9, 7, 5})));
		test(a.empty());
		test(data == std::array{11, 9, 7, 5});
	});

	describe("out-of-range", [&](auto test) {
		auto data = std::array{1, 2, 3, 4};
		auto a = range(data);
		auto ok = a.put(range(std::array{11, 9, 7, 5, 0, 0, 0, 0}));
		test(not ok);
		test(a.empty());
		test(data == std::array{11, 9, 7, 5});
	});
});

describe("distance", [](auto test) {
	auto const numbers = S123;
	test(range(numbers).drop(10).empty());
	test(range(numbers).drop(10).end() == numbers.end());
	test(range(numbers).drop_back(10).empty());
	test(range(numbers).drop_back(10).end() == numbers.begin());

	test(range(numbers).drop(10).take(10).empty());
	test(range(numbers).drop(10).take(10).end() == numbers.end());

	test(range(numbers).take_back(3).size() == 3);
	test(range(numbers).take_back(10).size() == 3);
	test(range(numbers).take_back(10) == range(numbers));
	test(range(numbers).take_back(1) == range(numbers).drop(2));
	test(range(numbers).take_back(2) == range(numbers).drop(1));

	// Wconversion warnings, and inner test fails
// 	test(range(numbers).drop_back(-10).empty());
// 	test(range(numbers).drop(-10).empty());
});


describe("count", [](auto test) {
	auto const va = range(S1234);
	test(va.count([](auto x) { return x == 3; }) == 1);
	test(va.count([](auto x) { return x > 0 and x < 4; }) == 3);
	test(va.count([](auto x) { return x == 6; }) == 0);

	// unmodified
	test(va == range(S1234));
});

describe("any", [](auto test) {
	auto const va = range(S1234);
	test(va.any([](auto x) { return x == 3; }));
	test(not va.any([](auto x) { return x == 6; }));
	test(not va.take(0).any([](auto) { return true; }));

	// unmodified
	test(va == range(S1234));
});

describe("all", [](auto test) {
	auto const va = range(S1234);
	test(va.all([](auto x) { return x > 0 and x < 5; }));
	test(not va.all([](auto x) { return x == 3; }));
	test(va.take(0).all([](auto) { return false; }));

	// unmodified
	test(va == range(S1234));
});

describe("contains", [](auto test) {
	auto const va = range(S1234567);

	// contains
	test(va.contains(range(std::array<int, 0>{})));
	test(va.contains(range(va)));

	test(va.contains(range(std::array{1})));
	test(va.contains(range(std::array{4})));
	test(va.contains(range(std::array{4, 5})));
	test(va.contains(range(std::array{4, 5, 6})));
	test(va.contains(range(std::array{7})));
	test(va.contains(range(S1234)));
	test(va.contains(range(S123456)));
	test(va.contains(range(S1234567)));

	test(not va.contains(range(std::array{0}))); // a missing 0
	test(not va.contains(range(std::array{1, 2, 3, 4, 5, 6, 7, 7}))); // a missing second 7
	test(not va.contains(range(std::array{6, 7, 8}))); // a missing 8
	test(not va.contains(range(std::array{8}))); // a missing 8
	test(not va.contains(range(std::array{4, 5, 8}))); // b missing 6, 7
	test(not va.contains(range(std::array{1, 3, 4}))); // b missing 2

	// unmodified
	test(va == range(S1234567));
});

describe("starts_with", [](auto test) {
	auto const va = range(S1234567);

	test(va.starts_with(range(std::array<int, 0>{})));
	test(va.starts_with(range(va)));

	test(va.starts_with(range(std::array{1})));
	test(va.starts_with(range(std::array{1, 2, 3})));
	test(not va.starts_with(range(std::array{0})));
	test(not va.starts_with(range(std::array{1, 2, 4})));
	test(not va.starts_with(range(std::array{1, 2, 3, 4, 5, 6, 7, 8})));
	test(not va.starts_with(range(std::array{1, 2, 3, 4, 5, 6, 7, 8, 9})));
	test(not va.starts_with(range(std::array{2})));
	test(not va.starts_with(range(std::array{2, 3})));
	test(not va.starts_with(range(std::array{6, 7})));
	test(not va.starts_with(range(std::array{7})));

	// unmodified
	test(va == range(S1234567));
});

describe("ends_with", [](auto test) {
	auto const va = range(S1234567);

	test(va.ends_with(range(std::array<int, 0>{})));
	test(va.ends_with(range(va)));

	test(va.ends_with(range(std::array{7})));
	test(va.ends_with(range(std::array{6, 7})));
	test(va.ends_with(range(std::array{5, 6, 7})));
	test(not va.ends_with(range(std::array{0})));
	test(not va.ends_with(range(std::array{1})));
	test(not va.ends_with(range(std::array{1, 2})));
	test(not va.ends_with(range(std::array{6})));
	test(not va.ends_with(range(std::array{6, 5})));
	test(not va.ends_with(range(std::array{1, 2, 4})));
	test(not va.ends_with(range(std::array{1, 2, 3, 4, 5, 6, 7, 8})));

	// zstr
	auto const foobar = zstr_range("foobar");
	test(foobar.starts_with(zstr_range("foo")));
	test(foobar.starts_with(zstr_range("foobar")));
	test(    foobar.contains(zstr_range("ooba")));
	test(     foobar.ends_with(zstr_range("bar")));
	test(  foobar.ends_with(zstr_range("foobar")));

	// unmodified
	test(va == range(S1234567));
});

describe("unto", [](auto test) {
	auto data = zstr_range("the quick brown fox jumped");
	auto save = data;

	save.pop_until([](auto c) { return c == 'b'; });
	data = data.drop_unto(save);
	test(data == zstr_range("brown fox jumped"));

	save.pop_back_until([](auto c) { return c == 'x'; });
	data = data.drop_back_unto(save);
	test(data == zstr_range("brown fox"));

	auto const a = data.drop_unto(zstr_range("out of bounds"));
	test(a.empty() or a == data); // truncated or unchanged, but not U/B

	auto const b = data.drop_back_unto(zstr_range("out of bounds (2)"));
	test(b.empty() or b == data); // truncated or unchanged, but not U/B

	auto const fox = data.drop(6);
	auto const c = data.drop_unto(fox);
	test(c == zstr_range("fox"));

	auto const d = data.drop_back_unto(fox);
	test(d == zstr_range("brown fox"));

	auto const brown = data.drop_back_until([](auto c) { return c == 'n'; });
	test(brown == zstr_range("brown"));

	test(brown.drop_unto(fox).empty()); // truncated
	test(brown.drop_back_unto(fox) == brown); // unchanged

	test(fox.drop_unto(brown) == fox); // unchanged
	test(fox.drop_back_unto(brown).empty()); // truncated
});

describe("compat", [](auto test) {
	auto v = std::vector<char>(10);
	auto va = ptr_range(v);

	test(compat::put_to_chars(va, 10UL));
	va.put(' ');
	test(compat::put_to_chars(va, 432));
	va.put(' ');
	test(compat::put_to_chars(va, -55));
	test(range(v) == zstr_range("10 432 -55"));

	auto vb = ptr_range(v);
	test(compat::read_from_chars(vb, 0UL) == 10UL);
	vb.pop_front();
	test(compat::peek_from_chars(vb, 0) == 432);
	test(compat::read_from_chars(vb, 0) == 432);
	vb.pop_front();
	test(compat::peek_from_chars(vb, 0) == -55);
	test(compat::read_from_chars(vb, 0) == -55);
});

// test nothing has been modified
describe("no modifications", [&](auto test) {
	test(S123 == std::array{1, 2, 3});
	test(S1234 == std::array{1, 2, 3, 4});
	test(S123456 == std::array{1, 2, 3, 4, 5, 6});
	test(S1234567 == std::array{1, 2, 3, 4, 5, 6, 7});
	test(TQBFJ == zstr_range("the quick brown fox jumped"));
});

	return 0;
}
