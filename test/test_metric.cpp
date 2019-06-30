#include <gtest/gtest.h>
#include "metric.h"

using namespace metric;
using namespace metric::literals;

TEST(MetricTest, nm_operator_ull) {
  auto d { 5_nm };
  EXPECT_EQ(5, d);
}
