#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <type_traits>

namespace __ranger {
	template <typename R, typename F>
	auto drop_until (R r, const F f) {
		while (not r.empty()) {
			if (f(r.front())) break;
			r.pop_front();
		}

		return r;
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
			std::is_same_v<typename R::value_type, typename E::value_type>,
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
		using distance_type = decltype(std::distance(I(), I()));

		Range (I begin, I end) : _begin(begin), _end(end) {}

		auto begin () const { return this->_begin; }
		auto empty () const { return this->_begin == this->_end; }
		auto end () const { return this->_end; }

		auto drop (const size_t un) const {
			const auto n = static_cast<distance_type>(un);
			if constexpr(std::is_signed<distance_type>::value) {
				assert(n >= 0);
			}

			auto it = this->_begin;
			std::advance(it, n);
			if constexpr(std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<I>::iterator_category>) {
				if (it > this->_end) return Range(this->_end, this->_end);
			}

			return Range(it, this->_end);
		}

		auto drop_back (const size_t un) const {
			const auto n = static_cast<distance_type>(un);
			if constexpr(std::is_signed<distance_type>::value) {
				assert(n >= 0);
			}

			auto it = this->_end;
			std::advance(it, -n);
			if constexpr(std::is_same<std::random_access_iterator_tag, typename std::iterator_traits<I>::iterator_category>::value) {
				if (it < this->_begin) return Range(this->_begin, this->_begin);
			}

			return Range(this->_begin, it);
		}

		template <typename F>
		auto drop_until (const F f) const {
			return __ranger::drop_until(*this, f);
		}

		auto take (const size_t n) const {
			return Range(this->_begin, this->drop(n).begin());
		}

		template <typename F>
		auto take_until (const F f) const {
			auto save  = this->drop_until(f);
			return Range(this->_begin, save.begin());
		}

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
		typename std::enable_if_t<std::is_pointer_v<U>, I> data () {
			return this->_begin;
		}

		template <typename T=typename std::iterator_traits<I>::iterator_category>
		typename std::enable_if_t<
			std::is_same_v<std::random_access_iterator_tag, T>,
			size_t
		> size () const {
			assert(this->_end >= this->_begin);
			return static_cast<size_t>(std::distance(this->_begin, this->_end));
		}

		template <typename T=typename std::iterator_traits<I>::iterator_category>
		typename std::enable_if_t<
			std::is_same_v<std::random_access_iterator_tag, T>,
			value_type&
		> operator[] (const size_t i) {
			return this->drop(i).front();
		}

		template <typename T=typename std::iterator_traits<I>::iterator_category>
		typename std::enable_if_t<
			std::is_same_v<std::random_access_iterator_tag, T>,
			value_type
		> operator[] (const size_t i) const {
			return this->drop(i).front();
		}

		template <typename E, typename T=typename std::iterator_traits<I>::iterator_category>
		typename std::enable_if_t<
			std::is_base_of_v<std::forward_iterator_tag, T>,
			bool
		> operator< (const E& rhs) const {
			return std::lexicographical_compare(this->begin(), this->end(), rhs.begin(), rhs.end());
		}

		template <typename E, typename T=typename std::iterator_traits<I>::iterator_category>
		typename std::enable_if_t<
			std::is_base_of_v<std::forward_iterator_tag, T>,
			bool
		> operator== (const E& rhs) const {
			return std::equal(this->begin(), this->end(), rhs.begin(), rhs.end());
		}

		// mutators
		void pop_back () {
			if (this->empty()) return;
			std::advance(this->_end, -1);
		}

		auto pop_back (const size_t n) {
			auto save = this->_end;
			*this = this->drop_back(n);
			return Range(this->_end, save);
		}

		void pop_front () {
			if (this->empty()) return;
			std::advance(this->_begin, 1);
		}

		auto pop_front (const size_t n) {
			auto save = this->_begin;
			*this = this->drop(n);
			return Range(save, this->_begin);
		}

		template <typename F>
		auto pop_until (const F f) {
			auto save = this->_begin;
			*this = this->drop_until(f);
			return Range(save, this->_begin);
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
