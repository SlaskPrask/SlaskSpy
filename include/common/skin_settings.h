#ifndef SKIN_SETTINGS_H
#define SKIN_SETTINGS_H

#include <string>
#include <string_view>
#include <vector>


namespace slask_spy {

	struct CommonSetting {
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
		std::string image;
	};

	struct ButtonSetting : public CommonSetting {
		std::string name;
	};

	struct StickSetting : public CommonSetting {
		int32_t x_range;
		int32_t y_range;
		std::string x_name;
		std::string y_name;
	};

	struct BackgroundSetting {
		std::string image;
	};


	class SkinSettings {
	public:
		static SkinSettings* LoadSkinSettings(std::string_view skin_directory);

		std::vector<ButtonSetting> const& GetButtonSettings() const;
		std::vector<StickSetting> const &GetStickSettings() const;
		BackgroundSetting const &GetBackgroundSettings() const;
		std::string_view GetSkinPath() const;

	private:
		SkinSettings(std::string_view skin_directory);

		bool CreateButtonSetting(std::string const &line);
		bool CreateStickSetting(std::string const &line);
		bool CreateBackgroundSetting(std::string const &line);
		std::string GetAttributeValue(std::string const &line,
					      std::string const &name);


		bool valid_;
		// std::string skin_name_;
		// std::string skin_author_;
		// std::string skin_type_;

		std::string const skin_path_;
		std::vector<ButtonSetting> buttons_;
		std::vector<StickSetting> sticks_;
		BackgroundSetting background_;
	};

} // namespace slask_spy

#endif SKIN_SETTINGS_H
