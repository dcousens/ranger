# ranger
A header-only library for wrapping wrapping C++ STL iterators in a range-like interface.


**WARNING:** This library is a work in progress,  please report any bugs!


## Example

``` cpp
#include <ranger/ranger.hpp>

// ...

std::vector<int> numbers = {1, 2, 3};

for (auto &x : ranger::retro(numbers)) {
	std::cout << x << ' ';
}
// 3, 2, 1

auto a = ranger::range(numbers).take(2); // {1, 2}
a[1] = 8; // {1, 8}

numbers.push_back(9);

auto b = ranger::range(numbers).drop(1); // {8, 3, 9}
auto c = ranger::retro(b); // {9, 3, 8}
```


## LICENSE [MIT](LICENSE)
Inspired by ranges from [D](https://dlang.org/phobos/std_range.html) and functional programming in [Haskell](http://hackage.haskell.org/package/base-4.12.0.0/docs/Data-List.html).
