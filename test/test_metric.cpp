#include <gtest/gtest.h>
#include "metric.h"

using namespace metric;
using namespace metric::literals;

TEST(MetricTest, distance_cast_conversions) {
  auto d { 5.0_m };
  auto nm { distance_cast<nanometers<decltype(d)::repr>>(d) };
  auto um { distance_cast<micrometers<decltype(d)::repr>>(d) };
  auto mm { distance_cast<millimeters<decltype(d)::repr>>(d) };
  auto m { distance_cast<meters<decltype(d)::repr>>(d) };
  auto km { distance_cast<kilometers<decltype(d)::repr>>(d) };
  auto Mm { distance_cast<megameters<decltype(d)::repr>>(d) };
  EXPECT_EQ(nm.count(), d.count() * 1000000000);
  EXPECT_EQ(um.count(), d.count() * 1000000);
  EXPECT_EQ(mm.count(), d.count() * 1000);
  EXPECT_EQ(m.count(), d.count());
  EXPECT_EQ(km.count() * 1000, d.count());
  EXPECT_EQ(Mm.count() * 1000000, d.count());
}

TEST(MetricTest, equality) {
  auto d1 { 5_cm };
  auto d2 { 50.0_mm };
  auto d3 { 0.5_dm };
  auto d4 { 0.05_m };
  auto d5 { 0.00005_km };
  auto d6 { 4_cm };
  EXPECT_TRUE(d1 == d2);
  EXPECT_TRUE(d2 == d1);
  EXPECT_TRUE(d1 == d3);
  EXPECT_TRUE(d1 == d4);
  EXPECT_TRUE(d1 == d5);
  EXPECT_TRUE(d1 != d6);
}

TEST(MetricTest, relational) {
  auto d1 { 5_cm };
  auto d2 { 4_cm };
  EXPECT_TRUE(d2 < d1);
  EXPECT_FALSE(d2 < d2);
  EXPECT_TRUE(d1 > d2);
  EXPECT_FALSE(d1 > d1);
  EXPECT_TRUE(d2 <= d2);
  EXPECT_TRUE(d2 >= d2);
  EXPECT_TRUE(d1 >= d2);
}

TEST(MetricTest, arithmetic_ext) {
  auto d1 { 1_cm };
  auto d2 { 0.02_m };
  auto d3 { centimeters<unsigned long long>(-1) };
  auto d4 { 40_cm };
  EXPECT_EQ(d1 + d1, d2);
  EXPECT_EQ(d1 + d3, centimeters<unsigned long long>(0));
  EXPECT_EQ(d2 - d1, d1);
  EXPECT_EQ(d2 + d2, d2 * 2);
  EXPECT_EQ(d2 + d2, 2 * d2);
  EXPECT_EQ(d1 / 1ULL, d1);
  EXPECT_EQ(d2 / d1, 2);
  EXPECT_EQ(d4 % 30_cm, 10_cm);
  EXPECT_EQ(d4 % 30, 10_cm);
}

TEST(MetricTest, arithmetic_int) {
  auto d1 { 1_cm  };
  auto d2 { 0.02_m };
  auto d4 { 40_cm };
  EXPECT_EQ(d4++, 40_cm);
  EXPECT_EQ(++d4, 42_cm);
  EXPECT_EQ(--d4, 41_cm);
  EXPECT_EQ(d4--, 41_cm);
  EXPECT_EQ(d4, 40_cm);
  d4 += 3_cm;
  EXPECT_EQ(d4, 43_cm);
  d4 -= 3_cm;
  EXPECT_EQ(d4, 40_cm);
  d4 *= 3;
  EXPECT_EQ(d4, 120_cm);
  d4 /= 3;
  EXPECT_EQ(d4, 40_cm);
  d4 %= 30;
  EXPECT_EQ(d4, 10_cm);
  d4 %= 3_cm;
  EXPECT_EQ(d4, 1_cm);
}

template <typename Repr>
using yards = metric::distance<Repr, std::ratio<1143, 1250>>;

TEST(MetricTest, custom_units) {
  auto d { yards<long double>(10.0) };  // the whole ten yards
  auto m { distance_cast<meters<decltype(d)::repr>>(d) };
  EXPECT_DOUBLE_EQ(m.count(), 9.144);
  EXPECT_EQ(distance_cast<yards<long double>>(m), d);
  EXPECT_EQ(d, d);

  auto e { yards<float>(10.0) };
  auto l { distance_cast<meters<decltype(e)::repr>>(e) };
  EXPECT_EQ(e, e);
  EXPECT_EQ(distance_cast<yards<decltype(e)::repr>>(l), e);
  static_assert( distance_cast<yards<float>>(
        distance_cast<meters<float>>(yards<float>(10.0))) ==
      yards<float>(10.0), "distance_cast is invertible");
}
