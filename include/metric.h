#ifndef METRIC_METRIC_H_
#define METRIC_METRIC_H_

#include <ratio>
#include <type_traits>

namespace metric {

/* @{ Base Types */
template <typename T>
struct is_distance : std::false_type {};

template <typename Repr, typename Ratio = std::ratio<1>>
struct distance {
  static_assert(!is_distance<Repr>::value, "A distance representation can not be distance");
  static_assert(std::__is_ratio<Ratio>::value, "Second template parameter of distance must be std::ratio");
  static_assert(Ratio::num > 0, "distance ratio must be positive");

  using repr = Repr;
  using ratio = Ratio;

  constexpr distance() noexcept = default;

  distance(const distance&) = default;

  template <class Repr2, typename = typename std::enable_if<
    std::is_convertible<Repr2, Repr>::value &&
    (std::is_floating_point<Repr>::value || !std::is_floating_point<Repr2>::value)
  >::type>
  inline constexpr explicit distance(const Repr2& r) : count_(r) {};

  explicit constexpr distance(Repr c) noexcept : count_(c) {}

  // operator Repr() const noexcept { return count; }
  inline Repr count() const noexcept { return count_; }

  const Repr count_;
};

template <typename Repr, typename Ratio>
struct is_distance<distance<Repr, Ratio>> : std::true_type {};

template <typename Repr, typename Ratio>
struct is_distance<const distance<Repr, Ratio>> : std::true_type {};

template <typename Repr, typename Ratio>
struct is_distance<volatile distance<Repr, Ratio>> : std::true_type {};

template <typename Repr, typename Ratio>
struct is_distance<const volatile distance<Repr, Ratio>> : std::true_type {};

template <typename Repr> using nanometers  = distance<Repr, std::nano>;
template <typename Repr> using micrometers = distance<Repr, std::micro>;
template <typename Repr> using millimeters = distance<Repr, std::milli>;
template <typename Repr> using centimeters = distance<Repr, std::centi>;
template <typename Repr> using decimeters  = distance<Repr, std::deci>;
template <typename Repr> using meters      = distance<Repr>;
template <typename Repr> using kilometers  = distance<Repr, std::kilo>;
template <typename Repr> using megameters  = distance<Repr, std::mega>;
/* Base Types @} */


/* @{ Operators */
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
constexpr typename std::enable_if<is_distance<ToDistance>::value, ToDistance>::type
distance_cast(const distance<Repr, Ratio>& d) {
  return __distance_cast<distance<Repr, Ratio>, ToDistance>()(d);
}
/* Operators @} */

/* @{ Relational Operators */

// distance ==

template <typename LhsDistance, typename RhsDistance>
struct __distance_eq {
  inline constexpr bool operator()(const LhsDistance& lhs, const RhsDistance& rhs) const {
    using CT = typename std::common_type<LhsDistance, RhsDistance>::type;
    return distance_cast<CT>(lhs).count() == distance_cast<CT>(rhs).count();
  }
};

template <typename Distance>
struct __distance_eq<Distance, Distance> {
  inline constexpr bool operator()(const Distance& lhs, const Distance& rhs) const {
    return lhs.count() == rhs.count();
  }
};

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator==(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return __distance_eq<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>()(lhs, rhs);
}

// distance !=

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
bool operator!=(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return !__distance_eq<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>()(lhs, rhs);
}

// distance <

template <typename LhsDistance, typename RhsDistance>
struct __distance_lt {
  inline constexpr bool operator()(const LhsDistance& lhs, const RhsDistance& rhs) const {
    using CT = typename std::common_type<LhsDistance, RhsDistance>::type;
    return distance_cast<CT>(lhs) < distance_cast<CT>(rhs);
  }
};

template <typename Distance>
struct __distance_lt<Distance, Distance> {
  inline constexpr bool operator()(const Distance& lhs, const Distance& rhs) const {
    return lhs.count() < rhs.count();
  }
};

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator<(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return __distance_lt<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>()(lhs, rhs);
}

// distance >

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator>(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return rhs < lhs;
}


// distance <=

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator<=(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return !(rhs < lhs);
}

// distance >=

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator>=(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return !(lhs < rhs);
}

/* Relational Operators @} */

/* @{ Arithmetic Operators */

// distance +

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type
operator +(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  using CD = typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type;
  return CD(distance_cast<CD>(lhs).count() + distance_cast<CD>(rhs).count());
}

// distance -

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type
operator -(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  using CD = typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type;
  return CD(distance_cast<CD>(lhs).count() - distance_cast<CD>(rhs).count());
}

// distance * (scalar)

template <typename Repr1, typename Ratio1, typename Repr2>
inline constexpr
typename std::enable_if<
  std::is_convertible<Repr2, typename std::common_type<Repr1, Repr2>::type>::value,
  distance<typename std::common_type<Repr1, Repr2>::type, Ratio1>
>::type
operator *(const distance<Repr1, Ratio1>& d, const Repr2& s) {
  using CR = typename std::common_type<Repr1, Repr2>::type;
  using CD = distance<CR, Ratio1>;
  return CD(distance_cast<CD>(d).count() * static_cast<CR>(s));
}

template <typename Repr1, typename Ratio1, typename Repr2>
inline constexpr
typename std::enable_if<
  std::is_convertible<Repr2, typename std::common_type<Repr1, Repr2>::type>::value,
  distance<typename std::common_type<Repr1, Repr2>::type, Ratio1>
>::type
operator *(const Repr2& s, const distance<Repr1, Ratio1>& d) {
  using CR = typename std::common_type<Repr1, Repr2>::type;
  using CD = distance<CR, Ratio1>;
  return CD(distance_cast<CD>(d).count() * static_cast<CR>(s));
}

// distance /

template <typename Distance, typename Repr, bool = is_distance<Repr>::value>
struct __distance_divide_result {};

template <class Distance, class Repr2,
          bool = std::is_convertible<Repr2,
                                     typename std::common_type<typename Distance::repr, Repr2>::type
                                    >::value
>
struct __distance_divide_imp {};

template <typename Repr1, typename Ratio, typename Repr2>
struct __distance_divide_imp<distance<Repr1, Ratio>, Repr2, true> {
  using type = distance<typename std::common_type<Repr1, Repr2>::type, Ratio>;
};

template <typename Repr1, typename Ratio, typename Repr2>
struct __distance_divide_result<distance<Repr1, Ratio>, Repr2, false>
  : __distance_divide_imp<distance<Repr1, Ratio>, Repr2> {};


template <typename Repr1, typename Ratio, typename Repr2>
inline constexpr
typename __distance_divide_result<distance<Repr1, Ratio>, Repr2>::type
operator /(const distance<Repr1, Ratio>& d, const Repr2 s) {
  using CR = typename std::common_type<Repr1, Repr2>::type;
  using CD = distance<CR, Ratio>;
  return CD(distance_cast<CD>(d).count() / static_cast<CR>(s));
}

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
typename std::common_type<Repr1, Repr2>::type
operator /(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  using CT = typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type;
  return distance_cast<CT>(lhs).count() / distance_cast<CT>(rhs).count();
}

// distance %

template <typename Repr1, typename Ratio, typename Repr2>
inline constexpr
typename __distance_divide_result<distance<Repr1, Ratio>, Repr2>::type
operator %(const distance<Repr1, Ratio>& lhs, const Repr2& rhs) {
  using CR = typename std::common_type<Repr1, Repr2>::type;
  using CD = distance<CR, Ratio>;
  return CD(distance_cast<CD>(lhs).count() % static_cast<CR>(rhs));
}

template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type
operator %(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  using CR = typename std::common_type<Repr1, Repr2>::type;
  using CD = typename std::common_type<distance<Repr1, Ratio1>,
                                       distance<Repr2, Ratio2>>::type;
  return CD(static_cast<CR>(distance_cast<CD>(lhs).count()) %
            static_cast<CR>(distance_cast<CD>(rhs).count()));
}

/* Arithmetic Operators @} */

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

inline constexpr metric::decimeters<ull> operator""_dm(ull v) {
  return decimeters<ull>(v);
}

inline constexpr metric::decimeters<ld> operator""_dm(ld v) {
  return decimeters<ld>(v);
}

inline constexpr metric::centimeters<ull> operator""_cm(ull v) {
  return centimeters<ull>(v);
}

inline constexpr metric::centimeters<ld> operator""_cm(ld v) {
  return centimeters<ld>(v);
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
