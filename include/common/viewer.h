#ifndef VIEWER_H
#define VIEWER_H

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "input_items.h"

namespace slask_spy {
enum class ViewerType { kNull = 0, kN64, kGC };

class Viewer {
public:
  static Viewer *CreateViewer(ViewerType type);
  static std::string StringFromType(ViewerType type);
  static ViewerType TypeFromString(std::string_view type_string);
  static int32_t GetMappingIndex(std::string_view name, ViewerType type);

  virtual size_t GetDataBytesSize() const = 0;

  void SetIncommingData(char *data);
  void AssignButton(InputButton *button_item);
  void AssignStick(InputStick *stick_item);
  void AssignAnalog(InputAnalog *stick_item);

  virtual ~Viewer() = default;

protected:
  virtual void SetStickData(char *data, InputStick *stick);

  std::vector<InputButton *> assigned_buttons_{};
  std::vector<InputStick *> assigned_sticks_{};
  std::vector<InputAnalog *> assigned_analogs_{};
};
} // namespace slask_spy

#endif // VIEWER_H
