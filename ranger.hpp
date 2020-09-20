#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <type_traits>

namespace __ranger {
	template <
		typename R,
		typename U=typename R::iterator,
		typename T=typename std::iterator_traits<U>::iterator_category
	> typename std::enable_if_t<
		std::is_same<std::random_access_iterator_tag, T>::value,
		R
	> drop (R r, const size_t n) {
		auto begin = r.begin();
		std::advance(begin, n);
		if (begin > r.end()) return R(r.end(), r.end());
		return R(begin, r.end());
	}

	template <
		typename R,
		typename U=typename R::iterator,
		typename T=typename std::iterator_traits<U>::iterator_category
	> typename std::enable_if_t<
		not std::is_same<std::random_access_iterator_tag, T>::value,
		R
	> drop (R r, const size_t n) {
		for (size_t i = 0; i < n; i++) {
			if (r.empty()) break;
			r.pop_front();
		}

		return r;
	}

	template <typename R>
	auto take (R r, const size_t n) {
		return R(r.begin(), r.drop(n).begin());
	}

	template <typename R>
	void put (R& r, typename R::value_type e) {
		r.front() = e;
		r.pop_front();
	}

	template <
		typename R,
		typename E,
		typename V = typename std::enable_if_t<
			std::is_same<typename R::value_type, typename E::value_type>::value,
			E
		>
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
		using iterator = I;
		using value_type = typename std::remove_const_t<
			typename std::remove_reference_t<decltype(*I())>
		>;

		Range (I begin, I end) : _begin(begin), _end(end) {}

		auto begin () const { return this->_begin; }
		auto drop (size_t n) const { return __ranger::drop(*this, n); }
		auto empty () const { return this->_begin == this->_end; }
		auto end () const { return this->_end; }

		auto take (size_t n) const { return __ranger::take(*this, n); }
		auto& back () {
			assert(not this->empty());
			return *(this->_end - 1);
		}

		auto& back () const {
			assert(not this->empty());
			return *(this->_end - 1);
		}

		auto& front () {
			assert(not this->empty());
			return *this->_begin;
		}

		auto& front () const {
			assert(not this->empty());
			return *this->_begin;
		}

		template <typename U=I>
		typename std::enable_if_t<std::is_pointer<U>::value, I> data () {
			return this->_begin;
		}

		template <typename U=I, typename T=typename std::iterator_traits<U>::iterator_category>
		typename std::enable_if_t<
			std::is_same<std::random_access_iterator_tag, T>::value,
			size_t
		> size () const {
			const auto diff = std::distance(this->_begin, this->_end);
			assert(diff >= 0);
			return static_cast<size_t>(diff);
		}

		template <typename U=I, typename T=typename std::iterator_traits<U>::iterator_category>
		typename std::enable_if_t<
			std::is_same<std::random_access_iterator_tag, T>::value,
			value_type&
		> operator[] (const size_t i) {
			return this->drop(i).front();
		}

		template <typename U=I, typename T=typename std::iterator_traits<U>::iterator_category>
		typename std::enable_if_t<
			std::is_same<std::random_access_iterator_tag, T>::value,
			value_type
		> operator[] (const size_t i) const {
			return this->drop(i).front();
		}

		template <typename E, typename U=I, typename T=typename std::iterator_traits<U>::iterator_category>
		typename std::enable_if_t<
			std::is_base_of<std::forward_iterator_tag, T>::value,
			bool
		> operator< (const E& rhs) const {
			return std::lexicographical_compare(this->begin(), this->end(), rhs.begin(), rhs.end());
		}

		template <typename E, typename U=I, typename T=typename std::iterator_traits<U>::iterator_category>
		typename std::enable_if_t<
			std::is_base_of<std::forward_iterator_tag, T>::value,
			bool
		> operator== (const E& rhs) const {
			return std::equal(this->begin(), this->end(), rhs.begin(), rhs.end());
		}

		// mutators
		void pop_back () {
			assert(not this->empty());
			std::advance(this->_end, -1);
		}

		void pop_front () {
			assert(not this->empty());
			std::advance(this->_begin, 1);
		}

		template <typename E>
		void put (E e) { __ranger::put(*this, e); }
	};

	template <typename I, typename F>
	struct OrderedRange : public Range<I> {
	public:
		OrderedRange (I begin, I end) : Range<I>(begin, end) {}

		using value_type = typename Range<I>::value_type;

		auto contains (const value_type& value) const {
			return std::binary_search(this->begin(), this->end(), value, F());
		}

		auto lower_bound (const value_type& value) const {
			return std::lower_bound(this->begin(), this->end(), value, F());
		}

		auto upper_bound (const value_type& value) const {
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

	// rvalue references wrappers
	template <typename R> auto range (R&& r) { return range<R>(r); }
	template <typename R> auto ptr_range (R&& r) { return ptr_range<R>(r); }
	template <typename R> auto reverse (R&& r) { return reverse<R>(r); }
	template <typename R> auto ordered (R&& r) { return ordered<R>(r); }
	template <typename F, typename R> auto ordered (R&& r) { return ordered<R, F>(r); }
}
