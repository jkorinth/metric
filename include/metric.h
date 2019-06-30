#ifndef METRIC_METRIC_H_
#define METRIC_METRIC_H_

#include <ratio>
#include <type_traits>

namespace metric {


/* @{ Base Types */
template <typename Repr, typename Ratio = std::ratio<1>>
struct distance {
  using repr = Repr;
  using ratio = Ratio;
  constexpr distance() noexcept = default;

  distance(const distance&) = default;

  template <class Repr2, typename = std::enable_if_t<std::is_convertible_v<Repr, Repr2>>>
  constexpr explicit distance(const Repr2& r) : distance(r) {};

  template <class Repr2, class Ratio2>
  constexpr distance(const distance<Repr2, Ratio2>& d);

  explicit constexpr distance(Repr c) noexcept : count_(c) {}

  // operator Repr() const noexcept { return count; }
  inline Repr count() const noexcept { return count_; }

  const Repr count_;
};

template <typename Repr> using nanometers  = distance<Repr, std::nano>;
template <typename Repr> using micrometers = distance<Repr, std::micro>;
template <typename Repr> using millimeters = distance<Repr, std::milli>;
template <typename Repr> using centimeters = distance<Repr, std::centi>;
template <typename Repr> using decimeters  = distance<Repr, std::deci>;
template <typename Repr> using meters      = distance<Repr>;
template <typename Repr> using kilometers  = distance<Repr, std::kilo>;
template <typename Repr> using megameters  = distance<Repr, std::mega>;
/* Base Types @} */

/* @{ Arithmetic Operators */
/* Arithmetic Operators @} */

/* @{ Operators */
template <typename T>
struct is_distance : std::false_type {};

template <typename Repr, typename Ratio>
struct is_distance<distance<Repr, Ratio>> : std::true_type {};

template <typename Repr, typename Ratio>
struct is_distance<const distance<Repr, Ratio>> : std::true_type {};

template <typename Repr, typename Ratio>
struct is_distance<volatile distance<Repr, Ratio>> : std::true_type {};

template <typename Repr, typename Ratio>
struct is_distance<const volatile distance<Repr, Ratio>> : std::true_type {};

}  // namespace chrono

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
struct std::common_type<metric::distance<Repr1, Ratio1>, metric::distance<Repr2, Ratio2>> {
  using type = metric::distance<typename std::common_type<Repr1, Repr2>::type,
                                typename __ratio_gcd<Ratio1, Ratio2>::type>;
};

namespace metric {

template <typename FromDistance, typename ToDistance,
          typename Ratio = typename std::ratio_divide<
            typename FromDistance::ratio,
            typename ToDistance::ratio
          >::type,
          bool = Ratio::num == 1,
          bool = Ratio::den == 1>
struct __distance_cast;

template <class FromDistance, class ToDistance, class Ratio>
struct __distance_cast<FromDistance, ToDistance, Ratio, true, true> {
  inline constexpr ToDistance operator()(const FromDistance& fd) const {
    return ToDistance(static_cast<typename ToDistance::repr>(fd.count()));
  }
};

template <class FromDistance, class ToDistance, class Ratio>
struct __distance_cast<FromDistance, ToDistance, Ratio, true, false> {
  inline constexpr ToDistance operator()(const FromDistance& fd) const {
    using CT = typename std::common_type<typename ToDistance::repr,
                                         typename FromDistance::repr,
                                         intmax_t>::type;
    return ToDistance(static_cast<typename ToDistance::repr>(
          static_cast<CT>(fd.count()) / static_cast<CT>(Ratio::den)));
  }
};

template <class FromDistance, class ToDistance, class Ratio>
struct __distance_cast<FromDistance, ToDistance, Ratio, false, true> {
  inline constexpr ToDistance operator()(const FromDistance& fd) const {
    using CT = typename std::common_type<typename ToDistance::repr,
                                         typename FromDistance::repr,
                                         intmax_t>::type;
    return ToDistance(static_cast<typename ToDistance::repr>(
          static_cast<CT>(fd.count()) * static_cast<CT>(Ratio::num)));
  }
};

template <class ToDistance, class Repr, class Ratio>
constexpr std::enable_if_t<is_distance<ToDistance>::value, ToDistance>
distance_cast(const distance<Repr, Ratio>& d) {
  return __distance_cast<distance<Repr, Ratio>, ToDistance>()(d);
}
/* Operators @} */

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
