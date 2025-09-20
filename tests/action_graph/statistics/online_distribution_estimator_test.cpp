#include <action_graph/statistics/online_distribution_estimator.h>

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

namespace action_graph {
namespace statistics {
namespace {

TEST(OnlineDistributionEstimatorTest, ThrowsWithoutValues) {
  OnlineDistributionEstimator<double> estimator;
  EXPECT_THROW(static_cast<void>(estimator.GetNormalDistribution()),
               std::logic_error);
  EXPECT_THROW(static_cast<void>(estimator.GetMax()), std::logic_error);
}

TEST(OnlineDistributionEstimatorTest, HandlesSingleValue) {
  OnlineDistributionEstimator<double> estimator;
  estimator.AddValue(5.0);

  const auto distribution = estimator.GetNormalDistribution();
  EXPECT_EQ(distribution.sample_size, 1u);
  EXPECT_DOUBLE_EQ(distribution.mean, 5.0);
  EXPECT_DOUBLE_EQ(distribution.standard_deviation, 0.0);
  EXPECT_DOUBLE_EQ(estimator.GetMax(), 5.0);
}

TEST(OnlineDistributionEstimatorTest, ComputesStatisticsForIntegralValues) {
  OnlineDistributionEstimator<int> estimator;
  estimator.AddValue(1);
  estimator.AddValue(2);
  estimator.AddValue(3);
  estimator.AddValue(4);

  const auto distribution = estimator.GetNormalDistribution();
  EXPECT_EQ(distribution.sample_size, 4u);
  EXPECT_DOUBLE_EQ(distribution.mean, 2.5);
  const double expected_std_dev = std::sqrt(5.0 / 3.0);
  EXPECT_NEAR(distribution.standard_deviation, expected_std_dev, 1e-12);
  EXPECT_EQ(estimator.GetMax(), 4);
}

TEST(OnlineDistributionEstimatorTest, SupportsNegativeAndFloatingValues) {
  OnlineDistributionEstimator<double> estimator;
  estimator.AddValue(-1.5);
  estimator.AddValue(0.5);
  estimator.AddValue(2.0);

  const auto distribution = estimator.GetNormalDistribution();
  EXPECT_EQ(distribution.sample_size, 3u);
  EXPECT_NEAR(distribution.mean, 0.333333333333, 1e-9);
  const double expected_variance = ((-1.5 - distribution.mean) *
                                    (-1.5 - distribution.mean) +
                                    (0.5 - distribution.mean) *
                                        (0.5 - distribution.mean) +
                                    (2.0 - distribution.mean) *
                                        (2.0 - distribution.mean)) /
                                   2.0;
  EXPECT_NEAR(distribution.standard_deviation,
              std::sqrt(expected_variance), 1e-12);
  EXPECT_DOUBLE_EQ(estimator.GetMax(), 2.0);
}

TEST(OnlineDistributionEstimatorTest, MaintainsNumericalStabilityForLargeMagnitudes) {
  OnlineDistributionEstimator<double> estimator;
  constexpr long double kBaseValue = 1'000'000'000'000.0L;
  const std::vector<long double> offsets = {0.0L, 0.25L, -0.5L, 1.0L,
                                            -0.75L, 0.125L, -0.375L, 0.625L};

  std::vector<long double> values;
  values.reserve(offsets.size());
  for (const auto offset : offsets) {
    const long double value = kBaseValue + offset;
    estimator.AddValue(static_cast<double>(value));
    values.push_back(value);
  }

  const auto distribution = estimator.GetNormalDistribution();

  long double sum = 0.0L;
  for (const auto value : values) {
    sum += value;
  }
  const long double expected_mean = sum / static_cast<long double>(values.size());
  EXPECT_NEAR(distribution.mean, static_cast<double>(expected_mean), 1e-9);

  long double variance_sum = 0.0L;
  for (const auto value : values) {
    const long double diff = value - expected_mean;
    variance_sum += diff * diff;
  }
  const long double expected_variance =
      variance_sum / static_cast<long double>(values.size() - 1);
  EXPECT_NEAR(distribution.standard_deviation,
              std::sqrt(static_cast<double>(expected_variance)), 1e-9);
}

} // namespace
} // namespace statistics
} // namespace action_graph

