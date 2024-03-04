#ifndef GAMECUBE_VIEWER_H
#define GAMECUBE_VIEWER_H

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "viewer.h"
namespace slask_spy {
    class GamecubeViewer : public Viewer
    {
    public:
        GamecubeViewer() = default;

        size_t GetDataBytesSize() const override {
            return kDataBytes;
        }

        static std::unordered_map<std::string_view, int32_t> const& GetMapping()
        {
            static std::unordered_map<std::string_view, int32_t> const kMapping{
                {"start", 3},
                {"y", 4},
                {"x", 5},
                {"b", 6},
                {"a", 7},
                {"l", 9},
                {"r", 10},
                {"z", 11},
                {"up", 12},
                {"down", 13},
                {"right", 14},
                {"left", 15},

                {"lstick_x", 16},
                {"lstick_y", 24},
                {"cstick_x", 32},
                {"cstick_y", 40},
                {"trig_l", 48},
                {"trig_r", 56}
            };
            return kMapping;
        }

    private:
        static constexpr size_t kDataBytes{ 33 };

    };
} // namespace slask_spy

#endif // GAMECUBE_VIEWER_H_VIEWER_H
