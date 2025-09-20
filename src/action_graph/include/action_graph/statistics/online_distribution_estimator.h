#pragma once

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

namespace action_graph {
namespace statistics {

template <typename Value> class OnlineDistributionEstimator {
  static_assert(std::is_arithmetic_v<Value>,
                "OnlineDistributionEstimator requires arithmetic Value type.");

public:
  struct NormalDistributionParameters {
    double mean = 0.0;
    double standard_deviation = 0.0;
    std::size_t sample_size = 0;
  };

  void AddValue(Value value) {
    const long double value_ld = static_cast<long double>(value);

    if (sample_size_ == 0) {
      reference_value_ = value_ld;
      has_reference_value_ = true;
      sample_size_ = 1;
      mean_displacement_ = 0.0L;
      m2_displacement_ = 0.0L;
      max_value_ = value;
      has_max_value_ = true;
      return;
    }

    ++sample_size_;
    const long double displacement = value_ld - reference_value_;
    const long double delta = displacement - mean_displacement_;
    mean_displacement_ +=
        delta / static_cast<long double>(sample_size_);
    const long double delta2 = displacement - mean_displacement_;
    m2_displacement_ += delta * delta2;

    if (!has_max_value_ || value > max_value_) {
      max_value_ = value;
      has_max_value_ = true;
    }
  }

  [[nodiscard]] NormalDistributionParameters GetNormalDistribution() const {
    if (sample_size_ == 0) {
      throw std::logic_error(
          "Unable to retrieve distribution parameters without samples.");
    }
    if (!has_reference_value_) {
      throw std::logic_error(
          "Internal error: reference value not initialized despite samples.");
    }

    NormalDistributionParameters distribution{};
    distribution.mean =
        static_cast<double>(reference_value_ + mean_displacement_);
    distribution.sample_size = sample_size_;
    if (sample_size_ > 1) {
      const long double variance =
          m2_displacement_ /
          static_cast<long double>(sample_size_ - 1);
      distribution.standard_deviation =
          std::sqrt(static_cast<double>(variance));
    }
    return distribution;
  }

  [[nodiscard]] Value GetMax() const {
    if (!has_max_value_) {
      throw std::logic_error("Unable to retrieve maximum without samples.");
    }
    return max_value_;
  }

  [[nodiscard]] std::size_t GetSampleSize() const { return sample_size_; }

private:
  std::size_t sample_size_ = 0;
  long double reference_value_ = 0.0L;
  bool has_reference_value_ = false;
  long double mean_displacement_ = 0.0L;
  long double m2_displacement_ = 0.0L;
  Value max_value_{};
  bool has_max_value_ = false;
};

} // namespace statistics
} // namespace action_graph

