#ifndef GAMECUBE_VIEWER_H
#define GAMECUBE_VIEWER_H

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "viewer.h"
namespace slask_spy {
class GamecubeViewer : public Viewer {
public:
  GamecubeViewer() = default;

  size_t GetDataBytesSize() const override { return kDataBytes; }

  static std::unordered_map<std::string_view, int32_t> const &GetMapping() {
    static std::unordered_map<std::string_view, int32_t> const kMapping{
        {"start", 3},     {"y", 4},         {"x", 5},         {"b", 6},
        {"a", 7},         {"l", 9},         {"r", 10},        {"z", 11},
        {"up", 12},       {"down", 13},     {"right", 14},    {"left", 15},

        {"lstick_x", 16}, {"lstick_y", 24}, {"cstick_x", 32}, {"cstick_y", 40},
        {"trig_l", 48},   {"trig_r", 56}};
    return kMapping;
  }

protected:
  void SetStickData(char *data, InputStick *stick) override {
    uint8_t x{0};
    uint8_t y{0};
    const int32_t x_index{stick->IndexX()};
    const int32_t y_index{stick->IndexY()};
    for (int32_t i{0}; i < 8; ++i) {
      x |= data[x_index + i] ? (0x80 >> i) : 0;
      y |= data[y_index + i] ? (0x80 >> i) : 0;
    }
    stick->Update(x - 128, y - 128);
  }

private:
  static constexpr size_t kDataBytes{65};
};
} // namespace slask_spy

#endif // GAMECUBE_VIEWER_H_VIEWER_H
