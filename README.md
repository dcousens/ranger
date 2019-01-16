# ranger
A modern, header-only, range library for C++.


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
a[1] = 8;

numbers.push_back(9);

auto b = ranger::range(numbers).drop(1); // {2, 8, 9}
auto c = ranger::retro(b); // {9, 8, 2}
```


## LICENSE [MIT](LICENSE)
Inspired by ranges from [D](https://dlang.org/phobos/std_range.html) and functional programming in [Haskell](http://hackage.haskell.org/package/base-4.12.0.0/docs/Data-List.html).
