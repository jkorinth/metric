#ifndef METRIC_METRIC_H_
#define METRIC_METRIC_H_

#include <ratio>

namespace metric {

template <typename Repr, typename Ratio = std::ratio<1>>
struct distance {
  explicit constexpr distance(Repr c) noexcept : count(c) {}
  const Repr count;
  operator Repr() const noexcept { return count; }
};

template <typename Repr> using nanometers = distance<Repr, std::nano>;
template <typename Repr> using micrometers = distance<Repr, std::micro>;
template <typename Repr> using millimeters = distance<Repr, std::milli>;
template <typename Repr> using centimeters = distance<Repr, std::centi>;
template <typename Repr> using decimeters = distance<Repr, std::deci>;
template <typename Repr> using meters = distance<Repr>;
template <typename Repr> using kilometers = distance<Repr, std::kilo>;
template <typename Repr> using megameters = distance<Repr, std::mega>;

namespace literals {
using ull = unsigned long long;
using ld = long double;

inline constexpr metric::nanometers<ull> operator""_nm(ull v) {
  return nanometers<ull>(v);
}

inline constexpr metric::nanometers<ld> operator""_nm(ld v) {
  return nanometers<ld>(v);
}

inline constexpr metric::micrometers<ull> operator""_um(ull v) {
  return micrometers<ull>(v);
}

inline constexpr metric::micrometers<ld> operator""_um(ld v) {
  return micrometers<ld>(v);
}

inline constexpr metric::millimeters<ull> operator""_mm(ull v) {
  return millimeters<ull>(v);
}

inline constexpr metric::millimeters<ld> operator""_mm(ld v) {
  return millimeters<ld>(v);
}

inline constexpr metric::meters<ull> operator""_m(ull v) {
  return meters<ull>(v);
}

inline constexpr metric::meters<ld> operator""_m(ld v) {
  return meters<ld>(v);
}

inline constexpr metric::kilometers<ull> operator""_km(ull v) {
  return kilometers<ull>(v);
}

inline constexpr metric::kilometers<ld> operator""_km(ld v) {
  return kilometers<ld>(v);
}

inline constexpr metric::megameters<ull> operator""_Mm(ull v) {
  return megameters<ull>(v);
}

inline constexpr metric::megameters<ld> operator""_Mm(ld v) {
  return megameters<ld>(v);
}

}  // namespace literals

}  // namespace metric

#endif  // METRIC_METRIC_H_
