# ranger
An attempt at a header-only range library for wrapping C++ STL iterators in a range-like interface.

**WARNING:** There is no assured bounds protection.

The library does have *some* `assert`s internally for debugging purposes,  but you still need to watch out for any undefined behaviour from your iterators,  e.g dereferencing past `end`.

**WARNING:** This library is a work in progress,  please report any bugs!  If you need something production safe,  this isn't the library for you.


## Example

``` cpp
#include <ranger/ranger.hpp>

// ...

std::vector<int> numbers = {1, 2, 3};

for (auto &x : ranger::reverse(numbers)) {
	std::cout << x << ' ';
}
// 3, 2, 1

auto a = ranger::range(numbers).take(2); // {1, 2}
a[1] = 8; // {1, 8}

numbers.push_back(9);

auto b = ranger::range(numbers).drop(1); // {8, 3, 9}
auto c = ranger::reverse(b); // {9, 3, 8}
```


## LICENSE [MIT](LICENSE)
Parts of this work are inspired by the concepts used in ranges as seen in the [D](https://dlang.org/) programming language.
