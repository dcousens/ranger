#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <type_traits>

namespace __ranger {
	template <typename R>
	auto drop (R r, const size_t n) {
		auto begin = r.begin();
		std::advance(begin, n);
		return R(begin, r.end());
	}

	template <typename R>
	auto take (R r, const size_t n) {
		auto end = r.begin();
		std::advance(end, n);
		return R(r.begin(), end);
	}

	template <typename R>
	void put (R& r, typename R::value_type e) {
		r.front() = e;
		r.pop_front();
	}

	template <
		typename R,
		typename E,
		typename V = typename std::enable_if<
			std::is_same<typename R::value_type, typename E::value_type>::value,
			E
		>::type
	>
	void put (R& r, E e) {
		while (not e.empty()) {
			r.front() = e.front();
			r.pop_front();
			e.pop_front();
		}
	}

	template <typename I>
	struct Range {
	private:
		I _begin;
		I _end;

	public:
		using value_type = typename std::remove_const<
			typename std::remove_reference<decltype(*I())>::type
		>::type;

		Range (I begin, I end) : _begin(begin), _end(end) {}

		auto begin () const { return this->_begin; }
		auto drop (size_t n) const { return __ranger::drop(*this, n); }
		auto empty () const { return this->_begin == this->_end; }
		auto end () const { return this->_end; }
		auto size () const {
			const auto diff = std::distance(this->_begin, this->_end);
			assert(diff >= 0);
			return static_cast<size_t>(diff);
		}
		auto take (size_t n) const { return __ranger::take(*this, n); }
		auto& back () { return *(this->_end - 1); }
		auto& back () const { return *(this->_end - 1); }
		auto& front () { return *this->_begin; }
		auto& front () const { return *this->_begin; }

		template <typename U=I>
		typename std::enable_if<std::is_pointer<U>::value, I>::type data () {
			return this->_begin;
		}

		auto& operator[] (const size_t i) {
			assert(i < this->size());
			auto it = this->_begin;
			std::advance(it, i);
			return *it;
		}

		auto operator[] (const size_t i) const {
			assert(i < this->size());
			auto it = this->_begin;
			std::advance(it, i);
			return *it;
		}

		template <typename E>
		auto operator< (const E& rhs) const {
			return std::lexicographical_compare(this->begin(), this->end(), rhs.begin(), rhs.end());
		}

		template <typename E>
		auto operator== (const E& rhs) const {
			return std::equal(this->begin(), this->end(), rhs.begin(), rhs.end());
		}

		// mutators
		void pop_back () {
			assert(std::distance(this->_begin, this->_end) > 0);
			std::advance(this->_end, -1);
		}
		void pop_front () {
			assert(std::distance(this->_begin, this->_end) > 0);
			std::advance(this->_begin, 1);
		}

		template <typename E>
		void put (E e) { __ranger::put(*this, e); }
	};

	template <typename I, typename F>
	struct OrderedRange : public Range<I> {
	private:
		F f;

	public:
		OrderedRange (I begin, I end, const F& f) : Range<I>(begin, end), f(f) {}

		template <typename V>
		auto contains (const V& value) const {
			return std::binary_search(this->begin(), this->end(), value, this->f);
		}

		template <typename V>
		auto lower_bound (const V& value) const {
			return std::lower_bound(this->begin(), this->end(), value, this->f);
		}

		template <typename V>
		auto upper_bound (const V& value) const {
			return std::upper_bound(this->begin(), this->end(), value, this->f);
		}
	};
}

namespace ranger {
	template <typename R>
	auto range (R& r) {
		using iterator = decltype(r.begin());

		return __ranger::Range<iterator>(r.begin(), r.end());
	}

	template <typename R>
	auto ptr_range (R& r) {
		using pointer = decltype(r.data());

		return __ranger::Range<pointer>(r.data(), r.data() + r.size());
	}

	inline auto zstr_range (const char* z) {
		using pointer = decltype(z);

		auto r = z;
		while (*r != '\0') r++;

		return __ranger::Range<pointer>(z, r);
	}

	template <typename R>
	auto retro (R& r) {
		using reverse_iterator = std::reverse_iterator<decltype(r.begin())>;

		return __ranger::Range<reverse_iterator>(reverse_iterator(r.end()), reverse_iterator(r.begin()));
	}

	template <typename R, typename F>
	auto ordered (R& r, const F& f) {
		using iterator = decltype(r.begin());

		return __ranger::OrderedRange<iterator, F>(r.begin(), r.end(), f);
	}

	template <typename R>
	auto ordered (R& r) {
		auto compare = std::less<>();
		return ordered(r, compare);
	}

	// rvalue references wrappers
	template <typename R> auto range (R&& r) { return range<R>(r); }
	template <typename R> auto ptr_range (R&& r) { return ptr_range<R>(r); }
	template <typename R> auto retro (R&& r) { return retro<R>(r); }
	template <typename R> auto ordered (R&& r) { return ordered<R>(r); }
	template <typename R, typename F> auto ordered (R&& r, const F& f) { return ordered<R, F>(r, f); }
}
