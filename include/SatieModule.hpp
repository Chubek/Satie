#pragma once

#include <string>
#include <string_view>

namespace satie
{

class TheoryModule
{
public:
  virtual ~TheoryModule () = default;
  virtual std::string_view name () const noexcept = 0;
};

} // namespace satie
