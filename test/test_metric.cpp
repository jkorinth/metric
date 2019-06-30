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
