#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <type_traits>

namespace __ranger {
	template <typename I>
	struct Range;

	template <typename I, typename R = Range<I>, bool Condition = R::is_forward::value>
	typename std::conditional_t<Condition, R, void>
	pop_front (R& a, size_t const un) {
		auto const n = static_cast<typename R::distance_type>(un);
		if constexpr(std::is_signed<typename R::distance_type>::value) {
			assert(n >= 0);
		}

		if constexpr(not R::is_forward::value) {
			if (a.empty()) return;
			std::advance(a._begin, n);
			return;
		} else {
			if (a.empty()) return a;
			auto it = a._begin;
			std::advance(a._begin, n);
			if constexpr(R::is_random_access::value) {
				if (a._begin > a._end) {
					a._begin = a._end;
				}
			}

			return R(it, a._begin);
		}
	}

	template <typename I, typename R = Range<I>, bool Condition = R::is_bidirectional::value>
	typename std::conditional_t<Condition, R, void>
	pop_back (R& a, size_t const un) {
		auto const n = static_cast<typename R::distance_type>(un);
		if constexpr(std::is_signed<typename R::distance_type>::value) {
			assert(n >= 0);
		}

		if constexpr(not R::is_forward::value) {
			if (a.empty()) return;
			std::advance(a._end, -n);
			return;
		} else {
			if (a.empty()) return a;
			auto it = a._end;
			std::advance(a._end, -n);
			if constexpr(R::is_random_access::value) {
				if (a._end < a._begin) {
					a._end = a._begin;
				}
			}

			return R(a._end, it);
		}
	}

	template <typename I, typename R = Range<I>, typename F>
	auto pop_until (R& a, F const f) {
		if constexpr(not R::is_forward::value) {
			while (not a.empty()) {
				if (f(a.front())) break;
				a.pop_front();
			}
		} else {
			auto copy = a;

			while (not a.empty()) {
				if (f(a.front())) break;
				a.pop_front();
			}

			return R(copy.begin(), a.begin());
		}
	}

	template <typename I, typename R = Range<I>, typename F>
	auto pop_back_until (R& a, F const f) {
		if constexpr(not R::is_forward::value) {
			while (not a.empty()) {
				if (f(a.back())) break;
				a.pop_back();
			}
		} else {
			auto copy = a;

			while (not a.empty()) {
				if (f(a.back())) break;
				a.pop_back();
			}

			return R(a.end(), copy.end());
		}
	}

	template <typename I>
	auto put (Range<I>& a, typename Range<I>::value_type v) {
		if (a.empty()) return false;
		a.front() = v;
		a.pop_front();
		return true;
	}

	template <typename A, typename B>
	auto put (Range<A>& a, Range<B> b) {
		while (not b.empty()) {
			if (a.empty()) return false;
			a.front() = b.front();
			a.pop_front();
			b.pop_front();
		}

		return true;
	}

	template <typename A, typename B>
	bool contains (Range<A> a, Range<B> const b_) {
		auto b = b_;

		while (not (a.empty() or b.empty())) {
			if (a.front() == b.front()) {
				a.pop_front();
				b.pop_front();
				continue;
			}

			a.pop_front();
			b = b_; // reset
		}

		return b.empty();
	}

	template <typename A, typename B>
	bool starts_with (Range<A> a, Range<B> const b_) {
		auto b = b_;

		while (not (a.empty() or b.empty())) {
			if (a.front() == b.front()) {
				a.pop_front();
				b.pop_front();
				continue;
			}

			return false;
		}

		return b.empty();
	}

	template <typename A, typename B>
	bool ends_with (Range<A> a, Range<B> const b_) {
		auto b = b_;

		while (not (a.empty() or b.empty())) {
			if (a.back() == b.back()) {
				a.pop_back();
				b.pop_back();
				continue;
			}

			return false;
		}

		return b.empty();
	}

	template <typename I>
	struct Range {
		I _begin;
		I _end;

		using iterator = I;
		using value_type = typename std::remove_const_t<
			typename std::remove_reference_t<decltype(*I())>
		>;
		using distance_type = decltype(std::distance(I(), I()));
		using iterator_category = typename std::iterator_traits<I>::iterator_category;
		using is_bidirectional = std::is_base_of<std::bidirectional_iterator_tag, iterator_category>;
		using is_forward = std::is_base_of<std::forward_iterator_tag, iterator_category>;
		using is_input = std::is_base_of<std::input_iterator_tag, iterator_category>;
		using is_random_access = std::is_base_of<std::random_access_iterator_tag, iterator_category>;

		Range (I begin, I end) : _begin(begin), _end(end) {
			if constexpr(is_random_access::value) {
				assert(end >= begin);
			}
		}

		auto begin () const { return this->_begin; }
		auto end () const { return this->_end; }
		auto empty () const { return this->begin() == this->end(); }

		template <bool Condition = std::is_pointer_v<I>>
		typename std::enable_if_t<Condition, I>
		data () const {
			return this->begin();
		}

		template <bool Condition = is_forward::value>
		typename std::enable_if_t<Condition, Range>
		drop (size_t const un) const {
			auto copy = *this;
			copy.pop_front(un);
			return copy;
		}

		template <typename F, bool Condition = is_forward::value>
		typename std::enable_if_t<Condition, Range>
		drop_until (F const f) const {
			auto copy = *this;
			copy.pop_until(f);
			return copy;
		}

		template <bool Condition = is_bidirectional::value>
		typename std::enable_if_t<Condition, Range>
		drop_back (size_t const un) const {
			auto copy = *this;
			copy.pop_back(un);
			return copy;
		}

		template <typename F, bool Condition = is_bidirectional::value>
		typename std::enable_if_t<Condition, Range>
		drop_back_until (F const f) const {
			auto copy = *this;
			copy.pop_back_until(f);
			return copy;
		}

		auto drop_unto (Range const b) const {
			if (b.begin() < this->begin()) return *this;
			if (b.begin() > this->end()) return Range(this->end(), this->end());
			return Range(b.begin(), this->end());
		}

		auto drop_back_unto (Range const b) const {
			if (b.end() > this->end()) return *this;
			if (b.end() < this->begin()) return Range(this->begin(), this->begin());
			return Range(this->begin(), b.end());
		}

		auto take (size_t const un) const {
			return Range(this->begin(), this->drop(un).begin());
		}

		template <typename F>
		auto take_until (F const f) const {
			return Range(this->begin(), this->drop_until(f).begin());
		}

		auto take_back (size_t const un) const {
			return Range(this->drop_back(un).end(), this->end());
		}

		template <typename F>
		auto take_back_until (F const f) const {
			return Range(this->drop_back_until(f).end(), this->end());
		}

		auto& front () {
			assert(not this->empty());
			return *this->begin();
		}

		auto& front () const {
			assert(not this->empty());
			return *this->begin();
		}

		auto& back () {
			return this->take_back(1).front();
		}

		auto& back () const {
			return this->take_back(1).front();
		}

		template <bool Condition = is_random_access::value>
		typename std::enable_if_t<Condition, size_t>
		size () const {
			assert(this->end() >= this->begin());
			return static_cast<size_t>(std::distance(this->begin(), this->end()));
		}

		template <bool Condition = is_random_access::value>
		typename std::enable_if_t<Condition, value_type&>
		operator[] (size_t const i) {
			return this->drop(i).front();
		}

		template <bool Condition = is_random_access::value>
		typename std::enable_if_t<Condition, value_type>
		operator[] (size_t const i) const {
			return this->drop(i).front();
		}

		template <typename B>
		bool operator< (B const& b) const {
			return std::lexicographical_compare(this->begin(), this->end(), b.begin(), b.end());
		}

		template <typename B>
		bool operator== (B const& b) const {
			return std::equal(this->begin(), this->end(), b.begin(), b.end());
		}

		template <typename F>
		bool any (F const f) const {
			for (auto x : *this) {
				if (f(x)) return true;
			}

			return false;
		}

		template <typename F>
		bool all (F const f) const {
			for (auto x : *this) {
				if (not f(x)) return false;
			}

			return true;
		}

		template <typename B>
		bool contains (B const& b) const {
			return __ranger::contains(*this, b);
		}

		template <typename B>
		bool starts_with (B const& b) const {
			return __ranger::starts_with(*this, b);
		}

		template <typename B>
		bool ends_with (B const& b) const {
			return __ranger::ends_with(*this, b);
		}

		template <typename F>
		size_t count (F const f) const {
			size_t result = 0;

			for (auto x : *this) {
				if (f(x)) ++result;
			}

			return result;
		}

		// mutators
		auto pop_back () {
			return __ranger::pop_back<I>(*this, 1);
		}

		auto pop_back (size_t const un) {
			return __ranger::pop_back<I>(*this, un);
		}

		auto pop_front () {
			return __ranger::pop_front<I>(*this, 1);
		}

		auto pop_front (size_t const un) {
			return __ranger::pop_front<I>(*this, un);
		}

		template <typename F>
		auto pop_until (F const f) {
			return __ranger::pop_until<I>(*this, f);
		}

		template <typename F>
		auto pop_back_until (F const f) {
			return __ranger::pop_back_until<I>(*this, f);
		}

		template <typename E>
		auto put (E const e) { return __ranger::put(*this, e); }
	};

	template <typename I, typename F>
	struct OrderedRange : public Range<I> {
		OrderedRange (I begin, I end) : Range<I>(begin, end) {}

		using value_type = typename Range<I>::value_type;

		auto contains (value_type const& value) const {
			return std::binary_search(this->begin(), this->end(), value, F());
		}

		auto lower_bound (value_type const& value) const {
			return std::lower_bound(this->begin(), this->end(), value, F());
		}

		auto upper_bound (value_type const& value) const {
			return std::upper_bound(this->begin(), this->end(), value, F());
		}
	};
}

namespace ranger {
	template <typename I> using range_t = __ranger::Range<I>;

	template <typename I>
	auto range (I begin, I end) {
		return range_t<I>(begin, end);
	}

	template <typename R>
	auto range (R& r) {
		using iterator = decltype(r.begin());
		return range_t<iterator>(r.begin(), r.end());
	}

	template <typename R>
	auto ptr_range (R& r) {
		using pointer = decltype(r.data());
		return range_t<pointer>(r.data(), r.data() + r.size());
	}

	inline auto zstr_range (const char* z) {
		auto r = z;
		while (*r != '\0') r++;
		return range(z, r);
	}

	template <typename R>
	auto reverse (R& r) {
		using reverse_iterator = std::reverse_iterator<decltype(r.begin())>;
		return range(reverse_iterator(r.end()), reverse_iterator(r.begin()));
	}

	template <typename F, typename R>
	auto ordered (R& r) {
		using iterator = decltype(r.begin());
		return __ranger::OrderedRange<iterator, F>(r.begin(), r.end());
	}

	template <typename R>
	auto ordered (R& r) {
		return ordered<std::less<>, R>(r);
	}

	template <typename I>
	typename std::enable_if_t<
		range_t<I>::is_input::value,
		range_t<I>
	>
	input_range (I begin) {
		return range_t<I>(begin, I{});
	}

	// rvalue references wrappers
	template <typename R> auto range (R&& r) { return range<R>(r); }
	template <typename R> auto ptr_range (R&& r) { return ptr_range<R>(r); }
	template <typename R> auto reverse (R&& r) { return reverse<R>(r); }
	template <typename R> auto ordered (R&& r) { return ordered<R>(r); }
	template <typename F, typename R> auto ordered (R&& r) { return ordered<R, F>(r); }
}
