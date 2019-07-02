/* Copyright (C) 2019 J. Korinth
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

/**
 * \file   metric.h
 * \brief  Header-only library for metric distances.
 * \author Jens Korinth <jkorinth@gmx.net>
 * \copyright GNU Lesser General Public License.
**/

#ifndef METRIC_METRIC_H_
#define METRIC_METRIC_H_

#include <cstdlib>
#include <limits>
#include <ostream>
#include <ratio>
#include <type_traits>

/**
 * \brief The `metric` namespace contains distance types based on meters.
 *
 * The base type `distance` stores distance values, where the unit is encoded in
 * the type using a compile-time rational. Units are not stored at runtime; they
 * are only used in conversions at compile-time.
 *
 * ## Literals
 *
 * The inline namespace `metric::literals` defines several convenient shorthands
 * for SI metric distances:
 *
 * ~~~{.cpp}
 * // Representative defaults to unsigned long long:
 * auto d1 { 1_nm };    // 1 nanometer
 * auto d2 { 1_um };    // 1 micrometer
 * auto d3 { 1_mm };    // 1 millimeter
 * auto d4 { 1_dm };    // 1 decimeter
 * auto d5 { 1_cm };    // 1 centimeter
 * auto d6 { 1_m };     // 1 meter
 * auto d7 { 1_km };    // 1 kilometer
 * auto d8 { 1_Mm };    // 1 megameter
 *
 * // Representative defaults to long double:
 * auto d9 { 1.0_nm };  // 1 nanometer
 * ...
 * ~~~
 *
 * ## Custom units
 *
 * Arbitrary distance units can be defined using the `std::ratio` class
 * template argument to express its ratio w.r.t. a meter:
 *
 * ~~~{.cpp}
 * // a type representation for 1 foot:
 * template <typename Repr>
 * using foot = metric::distance<Repr, std::ratio<381, 1250>>;
 * ~~~
 *
 * ## Casting between units 
 *
 * Distance units can be converted to each other via `distance_cast`:
 * ~~~{.cpp}
 * template <typename Repr>
 * using yards = metric::distance<Repr, std::ratio<1143, 1250>>;
 *
 * auto d { yards(10) };  // the whole ten yards
 * auto m { metric::distance_cast<metric::meters>(d) };
 *
 * // conversions are invertible:
 * static_assert(
 *   distance_cast<yards<float>>(distance_cast<meters<float>>(yards<float>(10.0))) == yards<float>(10.0),
 *   "distance_cast is invertible"
 * );
 * ~~~
 *
 * ## Relational Operators
 * Comparisons work between different units as expected:
 * ~~~{.cpp}
 * static_assert(5_cm == 0.5_dm);
 * static_assert(5_cm > 15_mm);
 * static_assert(5_cm < 1_m);
 * static_assert(5_cm <= 50_mm);
 * static_assert(5_cm >= 50_mm);
 * static_assert(5_cm != 4_cm);
 * ~~~
 *
 * ## Arithmetic Operators
 * Arithmetic operations include both distance and scalar operations:
 * ~~~{.cpp}
 * auto d1 { 5_cm };
 * auto d2 { d1 * 3 };      // == 15_cm
 * auto d3 { 3 * d1 };      // == d2
 * auto d4 { 5_cm / 5 };    // == 3_cm
 * auto d5 { 5_cm % 3_cm};  // == 2_cm
 * auto s6 { 6_cm / 2_cm};  // == 3
 * ...
 * ~~~
 *
 * Arithmetic operations automatically change units appropriately:
 * ~~~{.cpp}
 * std::cout << 1_cm + 5_m;   // -> "501 cm"
 * std::cout << 1_cm - 2_mm;  // -> "8 mm"
 * ~~~
 *
 * ## Stream Operators
 * As seen in the examples above, `std::ostream` operator implementations are provided, as well, to
 * provide nicely formatted output. Also works with custom types.
 **/
namespace metric {

/* @{ Ugly macro: constexpr rules are relaxed in C++14 and later. **/
#if __cplusplus > 201400
#define _METRIC_CONSTEXPR_AFTER_CXX14 constexpr
#else
#define _METRIC_CONSTEXPR_AFTER_CXX14
#endif
/* @} */

/* @{ base types */

/** \brief *Primary template:* Types are not distances by default. **/
template <typename T>
struct is_distance : std::false_type {};

/**
 * \brief Distance type for unit which have a linear relation with meters.
 *
 * The base type `distance` stores distance values, where the unit is encoded in
 * the type using a compile-time rational. Units are not stored at runtime; they
 * are only used in conversions at compile-time.
 *
 * \tparam Repr Representation type of units.
 * \tparam Ratio Instance of `std::ratio` describing the relation to meters, i.e.,
 *               `Repr` x `Ratio` === m.         
 **/
template <typename Repr, typename Ratio = std::ratio<1>>
struct distance {
  static_assert(!is_distance<Repr>::value, "A distance representation can not be distance");
  static_assert(std::__is_ratio<Ratio>::value, "Second template parameter of distance must be std::ratio");
  static_assert(Ratio::num > 0, "distance ratio must be positive");

  using repr = Repr;        ///< \brief Representation type for unit values.
  using ratio = Ratio;      ///< \brief Ratio expressing relation to meters.

  /*! \brief Default constructor. **/
  constexpr distance() noexcept = default;

  /*! \brief Default copy constructor. **/
  distance(const distance&) = default;

  /*!
   * \brief Construct value from representation of unit values.
   * \tparam Repr2 Type of unit representation (can be same as `Repr`).
   * \param r Unit values.
   */
  template <class Repr2, typename = typename std::enable_if<
    std::is_convertible<Repr2, Repr>::value &&
    (std::is_floating_point<Repr>::value || !std::is_floating_point<Repr2>::value)
  >::type>
  inline constexpr explicit distance(const Repr2& r) : count_(r) {};

  /*!
   * \brief Construct distance from representation of unit values.
   * \param c Unit values.
   **/
  explicit constexpr distance(Repr c) noexcept : count_(c) {}

  /*! \brief Increase this distance by one unit value. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance& operator++()    { ++count_; return *this; };

  /*! \brief Increase this distance by one unit value. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance operator++(int) { return distance(count_++); }

  /*! \brief Decrease this distance by one unit value. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance& operator--()    { --count_; return *this; }

  /*! \brief Decrease this distance by one unit value. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance operator--(int) { return distance(count_--); }

  /*! \brief Increase this distance by `rhs.count()` unit values. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance& operator +=(const distance& rhs) { count_ += rhs.count_; return *this; }

  /*! \brief Decrease this distance by `rhs.count()` unit values. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance& operator -=(const distance& rhs) { count_ -= rhs.count_; return *this; }

  /*! \brief Multiply this distance by `s`. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance& operator *=(const repr& s) { count_ *= s; return *this; }

  /*! \brief Divide this distance by `s`. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance& operator /=(const repr& s) { count_ /= s; return *this; }

  /*! \brief Return remainder of division of this distance by `s`. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance& operator %=(const repr& s) { count_ %= s; return *this; }

  /*! \brief Return remainder of division of this distance by `rhs.count()` unit values. **/
  inline _METRIC_CONSTEXPR_AFTER_CXX14
  distance& operator %=(const distance& rhs) { count_ %= rhs.count_; return *this; }

  /*! \brief Return number of unit values. **/
  inline constexpr Repr count() const noexcept { return count_; }

 private:
  Repr count_;
};

/** \brief *Specialization:* instances of `distance` are distances. **/
template <typename Repr, typename Ratio>
struct is_distance<distance<Repr, Ratio>> : std::true_type {};

/** \brief *Specialization:* `const` instances of `distance` are distances. **/
template <typename Repr, typename Ratio>
struct is_distance<const distance<Repr, Ratio>> : std::true_type {};

/** \brief *Specialization:* `volatile` instances of `distance` are distances. **/
template <typename Repr, typename Ratio>
struct is_distance<volatile distance<Repr, Ratio>> : std::true_type {};

/** \brief *Specialization:* `const volatile` instances of `distance` are distances. **/
template <typename Repr, typename Ratio>
struct is_distance<const volatile distance<Repr, Ratio>> : std::true_type {};

/* base types @} */

/* @{ type shorthands */
template <typename Repr> using nanometers  = distance<Repr, std::nano>;
template <typename Repr> using micrometers = distance<Repr, std::micro>;
template <typename Repr> using millimeters = distance<Repr, std::milli>;
template <typename Repr> using centimeters = distance<Repr, std::centi>;
template <typename Repr> using decimeters  = distance<Repr, std::deci>;
template <typename Repr> using meters      = distance<Repr>;
template <typename Repr> using kilometers  = distance<Repr, std::kilo>;
template <typename Repr> using megameters  = distance<Repr, std::mega>;
/* type shorthands @} */

}  // namespace metric

/*! @{ Specialization of `std::common_type` for `distance`. **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
struct std::common_type<metric::distance<Repr1, Ratio1>, metric::distance<Repr2, Ratio2>> {
  /// Common type is built with greatest common divisor (GCD).
  using type = metric::distance<typename std::common_type<Repr1, Repr2>::type,
                                typename __ratio_gcd<Ratio1, Ratio2>::type>;
};
/*! @} */

namespace metric {

/* @{ distance_cast */

namespace {   // anonymous namespace for distance_cast helpers

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

template <class FromDistance, class ToDistance, class Ratio>
struct __distance_cast<FromDistance, ToDistance, Ratio, false, false> {
  inline constexpr ToDistance operator()(const FromDistance& fd) const {
    using CT = typename std::common_type<typename ToDistance::repr,
                                         typename FromDistance::repr,
                                         intmax_t>::type;
    return ToDistance(static_cast<typename ToDistance::repr>(
          static_cast<CT>(fd.count()) * static_cast<CT>(Ratio::num)) / static_cast<CT>(Ratio::den));
  }
};

}  // namespace

/**
 * \brief Cast given `distance` instance to `ToDistance` type.
 * \tparam ToDistance `distance` type to cast to.
 * \tparam Repr Unit value representative of `d`.
 * \tparam Ratio Ratio of `d`.
 * \param d The `distance` instance to convert.
 * \return d In units of `ToDistance`.
 **/
template <class ToDistance, class Repr, class Ratio>
constexpr typename std::enable_if<is_distance<ToDistance>::value, ToDistance>::type
distance_cast(const distance<Repr, Ratio>& d) {
  return __distance_cast<distance<Repr, Ratio>, ToDistance>()(d);
}

/* distance_cast @} */

/* @{ relational_operators */

namespace {  // anonymous namespace for equality helpers

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
    return std::is_floating_point<typename Distance::repr>::value ?
      (lhs.count() > rhs.count() ?
        lhs.count() - rhs.count() <= std::numeric_limits<float>::epsilon() :
        rhs.count() - lhs.count() <= std::numeric_limits<float>::epsilon()) :
      lhs.count() == rhs.count();
  }
};

}  // namespace

/**
 * \brief Provides generalized equality relation across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of comparison.
 * \param rhs Right-hand side of comparison.
 * \return true, iff. distances are equal.
 **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator==(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return __distance_eq<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>()(lhs, rhs);
}

/**
 * \brief Provides generalized inequality relation across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of comparison.
 * \param rhs Right-hand side of comparison.
 * \return false, iff. distances are equal.
 **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
bool operator!=(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return !__distance_eq<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>()(lhs, rhs);
}

namespace {  // anonymous namespace for < operator helpers

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

}  // namespace

/**
 * \brief Provides generalized less-than relation across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of comparison.
 * \param rhs Right-hand side of comparison.
 * \return true, iff. lhs < rhs.
 **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator<(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return __distance_lt<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>()(lhs, rhs);
}

/**
 * \brief Provides generalized greater-than relation across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of comparison.
 * \param rhs Right-hand side of comparison.
 * \return true, iff. lhs > rhs.
 **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator>(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return rhs < lhs;
}


/**
 * \brief Provides generalized less-or-equal relation across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of comparison.
 * \param rhs Right-hand side of comparison.
 * \return true, iff. lhs <= rhs.
 **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator<=(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return !(rhs < lhs);
}

/**
 * \brief Provides generalized greater-or-equal relation across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of comparison.
 * \param rhs Right-hand side of comparison.
 * \return true, iff. lhs >= rhs.
 **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
bool operator>=(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  return !(lhs < rhs);
}

/* relational operators @} */

/* arithmetic operators @{ */

/**
 * \brief Provides generalized addition across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of comparison.
 * \param rhs Right-hand side of comparison.
 * \return New `distance` where length is LHS + RHS.
 **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type
operator +(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  using CD = typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type;
  return CD(distance_cast<CD>(lhs).count() + distance_cast<CD>(rhs).count());
}

/**
 * \brief Provides generalized subtraction across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of comparison.
 * \param rhs Right-hand side of comparison.
 * \return New `distance` of length LHS - RHS.
 **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type
operator -(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  using CD = typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type;
  return CD(distance_cast<CD>(lhs).count() - distance_cast<CD>(rhs).count());
}

/**
 * \brief Provides generalized scalar multiplication across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \param d Distance to multiply.
 * \param s Scalar value to multiply by.
 * \return New `distance` of length `d` * `s`.
 **/
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

/**
 * \brief Provides generalized multiplication across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param s Scalar value to multiply by.
 * \param d Distance to multiply.
 * \return New `distance` of length `d` * `s`.
 **/
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

namespace {  // anonymous namespace for / operator helpers

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

}  // namespace

/**
 * \brief Provides generalized scalar division across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param d Distance to divide.
 * \param s Scalar value to divide `d` by.
 * \return `distance` instance representing the divided lhs.
 **/
template <typename Repr1, typename Ratio, typename Repr2>
inline constexpr
typename __distance_divide_result<distance<Repr1, Ratio>, Repr2>::type
operator /(const distance<Repr1, Ratio>& d, const Repr2 s) {
  using CR = typename std::common_type<Repr1, Repr2>::type;
  using CD = distance<CR, Ratio>;
  return CD(distance_cast<CD>(d).count() / static_cast<CR>(s));
}

/**
 * \brief Provides generalized division across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of division.
 * \param rhs Right-hand side of division.
 * \return `distance` instance representing the divided lhs.
 **/
template <typename Repr1, typename Ratio1, typename Repr2, typename Ratio2>
inline constexpr
typename std::common_type<Repr1, Repr2>::type
operator /(const distance<Repr1, Ratio1>& lhs, const distance<Repr2, Ratio2>& rhs) {
  using CT = typename std::common_type<distance<Repr1, Ratio1>, distance<Repr2, Ratio2>>::type;
  return distance_cast<CT>(lhs).count() / distance_cast<CT>(rhs).count();
}

/**
 * \brief Provides generalized scalar modulo across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of comparison.
 * \param rhs Right-hand side of comparison.
 * \return `distance` instance representing the divison remainder.
 **/
template <typename Repr1, typename Ratio, typename Repr2>
inline constexpr
typename __distance_divide_result<distance<Repr1, Ratio>, Repr2>::type
operator %(const distance<Repr1, Ratio>& lhs, const Repr2& rhs) {
  using CR = typename std::common_type<Repr1, Repr2>::type;
  using CD = distance<CR, Ratio>;
  return CD(distance_cast<CD>(lhs).count() % static_cast<CR>(rhs));
}

/**
 * \brief Provides generalized scalar modulo across `distance` instances.
 * \tparam Repr1 Unit value representative type of LHS.
 * \tparam Ratio1 Unit value ratio type of LHS.
 * \tparam Repr2 Unit value representative type of RHS.
 * \tparam Ratio2 Unit value ratio type of RHS.
 * \param lhs Left-hand side of modulo.
 * \param rhs Right-hand side of modulo.
 * \return `distance` instance representing the divison remainder.
 **/
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

/* arithmetic operators @} */

/*  literal suffixes */

/** \brief Provides literal suffixes for metric distance literals. **/
namespace literals {

using ull = unsigned long long;  ///< short-hand for `unsigned long long`
using ld = long double;          ///< short-hand for `long double`

/**
 * \brief Nanometers suffix with `unsigned long long` representative.
 * \return v nanometers.
 **/
inline constexpr metric::nanometers<ull> operator""_nm(ull v) {
  return nanometers<ull>(v);
}

/**
 * \brief Nanometers suffix with `long double` representative.
 * \return v nanometers.
 **/
inline constexpr metric::nanometers<ld> operator""_nm(ld v) {
  return nanometers<ld>(v);
}

/**
 * \brief Micrometers suffix with `unsigned long long` representative.
 * \return v micrometers.
 **/
inline constexpr metric::micrometers<ull> operator""_um(ull v) {
  return micrometers<ull>(v);
}

/**
 * \brief Micrometers suffix with `long double` representative.
 * \return v micrometers.
 **/
inline constexpr metric::micrometers<ld> operator""_um(ld v) {
  return micrometers<ld>(v);
}

/**
 * \brief Millimeters suffix with `unsigned long long` representative.
 * \return v millimeters.
 **/
inline constexpr metric::millimeters<ull> operator""_mm(ull v) {
  return millimeters<ull>(v);
}

/**
 * \brief Millimeters suffix with `long double` representative.
 * \return v millimeters.
 **/
inline constexpr metric::millimeters<ld> operator""_mm(ld v) {
  return millimeters<ld>(v);
}

/**
 * \brief Centimeters suffix with `unsigned long long` representative.
 * \return v centimeters.
 **/
inline constexpr metric::centimeters<ull> operator""_cm(ull v) {
  return centimeters<ull>(v);
}

/**
 * \brief Centimeters suffix with `long double` representative.
 * \return v centimeters.
 **/
inline constexpr metric::centimeters<ld> operator""_cm(ld v) {
  return centimeters<ld>(v);
}

/**
 * \brief Decimeters suffix with `unsigned long long` representative.
 * \return v decimeters.
 **/
inline constexpr metric::decimeters<ull> operator""_dm(ull v) {
  return decimeters<ull>(v);
}

/**
 * \brief Decimeters suffix with `long double` representative.
 * \return v decimeters.
 **/
inline constexpr metric::decimeters<ld> operator""_dm(ld v) {
  return decimeters<ld>(v);
}

/**
 * \brief Meters suffix with `unsigned long long` representative.
 * \return v meters.
 **/
inline constexpr metric::meters<ull> operator""_m(ull v) {
  return meters<ull>(v);
}

/**
 * \brief Meters suffix with `long double` representative.
 * \return v meters.
 **/
inline constexpr metric::meters<ld> operator""_m(ld v) {
  return meters<ld>(v);
}

/**
 * \brief Kilometers suffix with `unsigned long long` representative.
 * \return v kilometers.
 **/
inline constexpr metric::kilometers<ull> operator""_km(ull v) {
  return kilometers<ull>(v);
}

/**
 * \brief Kilometers suffix with `long double` representative.
 * \return v kilometers.
 **/
inline constexpr metric::kilometers<ld> operator""_km(ld v) {
  return kilometers<ld>(v);
}

/**
 * \brief Megameters suffix with `unsigned long long` representative.
 * \return v megameters.
 **/
inline constexpr metric::megameters<ull> operator""_Mm(ull v) {
  return megameters<ull>(v);
}

/**
 * \brief Megameters suffix with `long double` representative.
 * \return v megameters.
 **/
inline constexpr metric::megameters<ld> operator""_Mm(ld v) {
  return megameters<ld>(v);
}

}  // namespace literals

/* literal suffixes  */

/* @{ stream operators */

/**
 * \brief *Primary template:* stream operator for generic `distance` instances.
 *
 * Outputs format: "<UNITS> <NUM>/<DEN> m".
 **/
template <typename Repr, typename Ratio>
std::ostream& operator <<(std::ostream& o, const distance<Repr, Ratio>& d) {
  o << d.count() << " " << Ratio::num << "/" << Ratio::den << " m";
  return o;
}

/**
 * \brief *Specialization:* stream operator for nanometers.
 *
 * Outputs format: "<UNITS> nm".
 **/
template <typename Repr>
std::ostream& operator <<(std::ostream& o, const distance<Repr, std::nano>& d) {
  o << d.count() << " nm";
  return o;
}

/**
 * \brief *Specialization:* stream operator for micrometers.
 *
 * Outputs format: "<UNITS> um".
 **/
template <typename Repr>
std::ostream& operator <<(std::ostream& o, const distance<Repr, std::micro>& d) {
  o << d.count() << " um";
  return o;
}

/**
 * \brief *Specialization:* stream operator for millimeters.
 *
 * Outputs format: "<UNITS> mm".
 **/
template <typename Repr>
std::ostream& operator <<(std::ostream& o, const distance<Repr, std::milli>& d) {
  o << d.count() << " mm";
  return o;
}

/**
 * \brief *Specialization:* stream operator for centimeters.
 *
 * Outputs format: "<UNITS> cm".
 **/
template <typename Repr>
std::ostream& operator <<(std::ostream& o, const distance<Repr, std::centi>& d) {
  o << d.count() << " cm";
  return o;
}

/**
 * \brief *Specialization:* stream operator for decimeters.
 *
 * Outputs format: "<UNITS> dm".
 **/
template <typename Repr>
std::ostream& operator <<(std::ostream& o, const distance<Repr, std::deci>& d) {
  o << d.count() << " dm";
  return o;
}

/**
 * \brief *Specialization:* stream operator for meters.
 *
 * Outputs format: "<UNITS> m".
 **/
template <typename Repr>
std::ostream& operator <<(std::ostream& o, const distance<Repr, std::ratio<1>>& d) {
  o << d.count() << " m";
  return o;
}

/**
 * \brief *Specialization:* stream operator for kilometers.
 *
 * Outputs format: "<UNITS> km".
 **/
template <typename Repr>
std::ostream& operator <<(std::ostream& o, const distance<Repr, std::kilo>& d) {
  o << d.count() << " km";
  return o;
}

/**
 * \brief *Specialization:* stream operator for megameters.
 *
 * Outputs format: "<UNITS> Mm".
 **/
template <typename Repr>
std::ostream& operator <<(std::ostream& o, const distance<Repr, std::mega>& d) {
  o << d.count() << " Mm";
  return o;
}

/* stream operators @} */

}  // namespace metric

#endif  // METRIC_METRIC_H_
