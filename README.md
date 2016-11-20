# JsonChecker_Constexpr
## An modified JSON Syntax Checker based on the original C-implementation on json.org
It is constexpr-complete, which means a highly reusability due to no-side-effects, no dynamic memory and a lot of static_assert for compile time unit testing

## Usage

````cpp
#include "JsonChecker_Constexpr.hpp"

constexpr auto V1 { ( "{}"_jsonvalidator ) };
static_assert (V1,"Fail V1");

#if __has_include(<optional>)
	constexpr auto V2 { "{}"_jsonoptionator };
	static_assert (V2,"Fail V2");
#endif

constexpr auto V3 { "{}"_jsonterminator };
static_assert (V3,"Fail V3");

ìnt main(){
  const auto real_life_snippet1 { "{}"_jsonterminator } ; // compiles
  const auto real_life_snippet2 { "{}"_jsonterminator } ; // stopps
  return 0;
}

````
