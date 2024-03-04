#ifndef N64_VIEWER_H
#define N64_VIEWER_H

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "viewer.h"
namespace slask_spy {
class N64Viewer : public Viewer {
public:
  N64Viewer() = default;

  size_t GetDataBytesSize() const override { return kDataBytes; }

  static std::unordered_map<std::string_view, int32_t> const &GetMapping() {
    static std::unordered_map<std::string_view, int32_t> const kMapping{
        {"a", 0},        {"b", 1},       {"z", 2},    {"start", 3},
        {"up", 4},       {"down", 5},    {"left", 6}, {"right", 7},
        {"l", 10},       {"r", 11},      {"cup", 12}, {"cdown", 13},
        {"cleft", 14},   {"cright", 15},

        {"stick_x", 16}, {"stick_y", 24}};
    return kMapping;
  }

private:
  static constexpr size_t kDataBytes{33};
};
} // namespace slask_spy

#endif // N64_VIEWER_H
