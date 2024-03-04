#ifndef SKIN_SETTINGS_H
#define SKIN_SETTINGS_H

#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace slask_spy {

	enum class AnalogDirection : uint8_t {
		kLeft,
		kRight,
		kUp,
		kDown
	};

	struct CommonSetting {
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
		std::string image;
	};

	struct ButtonSetting : public CommonSetting {
		int32_t index;
	};

	struct AnalogSetting : public ButtonSetting {
		AnalogDirection direction;
		bool reverse;
	};

	struct StickSetting : public CommonSetting {
		int32_t x_range;
		int32_t y_range;
		int32_t x_index;
		int32_t y_index;
	};

	struct BackgroundSetting {
		std::string image;
	};

	struct SkinData {
		std::string name;
		std::string author;
		std::string path;
	};

	enum class ViewerType;
	class SkinSettings {
	public:
		static SkinSettings* LoadSkinSettings(std::string_view skin_directory, ViewerType type);

		static std::unordered_map<ViewerType, std::vector<SkinData>> FetchSkins(std::string const& skins_directory);

		std::vector<ButtonSetting> const& GetButtonSettings() const;
		std::vector<StickSetting> const &GetStickSettings() const;
		std::vector<AnalogSetting> const &GetAnalogSettings() const;
		BackgroundSetting const &GetBackgroundSettings() const;
		std::string_view GetSkinPath() const;

	private:
		SkinSettings(std::string_view skin_directory, ViewerType type);

		bool CreateButtonSetting(std::string const &line);
		bool CreateStickSetting(std::string const &line);
		bool CreateBackgroundSetting(std::string const &line);
		bool CreateAnalogSetting(std::string const& line);
		static std::string GetAttributeValue(std::string const &line,
					      std::string const &name);
		static std::tuple<ViewerType, SkinData> GetSkinData(std::string_view skin_path);


		bool valid_;
		std::string const skin_path_;
		ViewerType const type_;
		std::vector<ButtonSetting> buttons_;
		std::vector<StickSetting> sticks_;
		std::vector<AnalogSetting> analogs_;
		BackgroundSetting background_;
	};

} // namespace slask_spy

#endif SKIN_SETTINGS_H
