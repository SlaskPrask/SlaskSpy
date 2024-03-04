#include "skin_settings.h"

#include <sys/stat.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "logger.h"
#include "viewer.h"

namespace slask_spy {

std::unordered_map<ViewerType, std::vector<SkinData>> SkinSettings::FetchSkins(std::string const& skins_directory) {
	std::unordered_map<ViewerType, std::vector<SkinData>> skins{};
	{
		struct stat sb { 0 };
		if (stat(skins_directory.c_str(), &sb) != 0) {
			Logger::Info("skin_settings: Invalid path: %s", skins_directory);
			return skins;
		}
	}
	
	std::filesystem::path skin_dir{ skins_directory };

	for (auto& path : std::filesystem::recursive_directory_iterator(skin_dir)) {
		if (path.is_directory()) {
			auto const data = GetSkinData(path.path().string());
			auto const type = std::get<0>(data);
			if (type != ViewerType::kNull) {
				auto it = skins.find(type);
				if (it == skins.end()) {
					skins[type] = std::vector<SkinData>{};
				}
				skins[type].push_back(std::get<1>(data));
			}
		}
	}
	return skins;
}

std::tuple<ViewerType, SkinData> SkinSettings::GetSkinData(std::string_view skin_path) {
	SkinData skin{};
	skin.path = skin_path;
	skin.path += "/";
	ViewerType type{ ViewerType::kNull };

	std::string const skin_xml_path{ skin.path + "skin.xml" };
	std::ifstream xml_file{skin_xml_path};

	if (xml_file.is_open()) {
		std::string line;
		// Find only the <skin tag, exclude xml tag, empty, return on other lines
		while (std::getline(xml_file, line)) {
			// Discard empty lines or the xml tag
			if (line.empty() ||
				line.find("<?xml") != std::string::npos) {
				continue;
			}

			if (line.find("<skin") == std::string::npos) {
				Logger::Error("skin_settings: Skin %s is not a valid skin.xml", skin_xml_path.c_str());
				return std::make_tuple(type, skin);
			}

			break;
		}
		type = Viewer::TypeFromString(GetAttributeValue(line, "type"));
		if (type != ViewerType::kNull) {
			skin.author = GetAttributeValue(line, "author");
			skin.name = GetAttributeValue(line, "name");
		}
		else {
			Logger::Warn("skin_settings: Skin type %s is not implemented. Skin: %s", GetAttributeValue(line, "type").c_str(), skin_xml_path.c_str());
		}
	}
	else {
		Logger::Warn("skin_settings: Could not open file %s", skin_xml_path.c_str());
	}

	return std::make_tuple(type, skin);
}

SkinSettings* SkinSettings::LoadSkinSettings(std::string_view skin_directory, ViewerType type) {
	SkinSettings *skin{new SkinSettings(skin_directory, type)};
	if (!skin->valid_) {
		delete skin;
		skin = nullptr;
	}

	return skin;
}

std::vector<ButtonSetting> const &SkinSettings::GetButtonSettings() const {
	return buttons_;
}

std::vector<StickSetting> const &SkinSettings::GetStickSettings() const {
	return sticks_;
}

std::vector<AnalogSetting> const& SkinSettings::GetAnalogSettings() const {
	return analogs_;
}

BackgroundSetting const &SkinSettings::GetBackgroundSettings() const {
	return background_;
}

std::string_view SkinSettings::GetSkinPath() const {
	return skin_path_;
}


	SkinSettings::SkinSettings(std::string_view skin_directory, ViewerType type)
		: valid_{false},
		skin_path_{skin_directory},
		type_{type},
		buttons_{},
		sticks_{},
		background_{}
	{
		std::ifstream xml_file{skin_path_ + "skin.xml"};
		
		if (xml_file.is_open()) {
			std::string line{};

			// First get to after <skin element
			while (std::getline(xml_file, line)) {
				// Discard empty lines or the xml tag
				if (line.empty() ||
				    line.find("<?xml") != std::string::npos) {
					continue;
				}

				break;
			}
			bool created_background{false};
			std::string element_line{};
			while (std::getline(xml_file, line)) {
				if (line.empty()) {
					continue;
				}

				if (line.find("</skin>") != std::string::npos) {
					valid_ = true;
					break;
				}

				if (line.find("<") != std::string::npos) {
					element_line = line;
					if (line.find("/>") == std::string::npos) {
						continue;
					}
				}
				else if (line.find("/>") != std::string::npos) {
					if (line.find("<") !=
					    std::string::npos) {
						break;
					}
					element_line += line;
				}


				if (element_line.find("<button") !=
				    std::string::npos) {
					if (!CreateButtonSetting(
						    element_line)) {
						break;
					}
				} else if (element_line.find("<stick") !=
					   std::string::npos) {
					if (!CreateStickSetting(element_line)) {
						break;
					}
				} else if (element_line.find("<analog") != std::string::npos) {
					if (!CreateAnalogSetting(element_line)) {
						break;
					}
				} else if (element_line.find("<background") !=
					   std::string::npos) {
					if (!CreateBackgroundSetting(
						    element_line)) {
						break;
					} else {
						created_background = true;
					}
				} 
				element_line = "";
			}
			if (!created_background) {
				valid_ = false;
			}
		} else {
			valid_ = false;
		}

	}

	bool SkinSettings::CreateAnalogSetting(std::string const& line) {
		try {
			CommonSetting const common{
				std::stoi(GetAttributeValue(line, "x")),
				std::stoi(GetAttributeValue(line, "y")),
				std::stoi(GetAttributeValue(line, "width")) + 1,
				std::stoi(GetAttributeValue(line, "height")) +
					1,
				GetAttributeValue(line, "image") };

			int32_t const index{ Viewer::GetMappingIndex(GetAttributeValue(line, "name"), type_) };

			if (index == -1) {
				return false;
			}

			static std::unordered_map<std::string_view, AnalogDirection> const kAnalogDirection{
				{"right", AnalogDirection::kRight},
				{"left", AnalogDirection::kLeft},
				{"down", AnalogDirection::kDown},
				{"up", AnalogDirection::kUp}
			};

			auto const& analog_dir = GetAttributeValue(line, "direction");
			auto const& analog_it = kAnalogDirection.find(analog_dir);
			if (analog_it == kAnalogDirection.end()) {
				Logger::Error("skin_settings: Invalid analog direction: %s", analog_dir.c_str());
				return false;
			}
;
			static std::unordered_map<std::string_view, bool> const kReverse{
				{"true", true},
				{"false", false}
			};
			auto const& reverse = GetAttributeValue(line, "reverse");
			auto const& reverse_it = kReverse.find(reverse);
			if (reverse_it == kReverse.end()) {
				Logger::Error("skin_settings: Invalid reverse value: %s", reverse.c_str());
				return false;
			}

			analogs_.push_back(AnalogSetting{
				common,
				index,
				analog_it->second,
				reverse_it->second
				});
			return true;
		}
		catch (std::exception const& /*unused*/) {
			return false;
		}
	}

	bool SkinSettings::CreateButtonSetting(std::string const &line)
	{
		try {
			CommonSetting const common{
				std::stoi(GetAttributeValue(line, "x")),
				std::stoi(GetAttributeValue(line, "y")),
				std::stoi(GetAttributeValue(line, "width")) + 1,
				std::stoi(GetAttributeValue(line, "height")) +
					1,
				GetAttributeValue(line, "image")};

			int32_t const index{ Viewer::GetMappingIndex(GetAttributeValue(line, "name"), type_) };

			if (index == -1) {
				return false;
			}

			buttons_.push_back(ButtonSetting{
				common,
				index
			});	
			return true;
        } catch (std::exception const & /*unused*/) {
			return false;
		}		
	}

	bool SkinSettings::CreateStickSetting(std::string const &line)
	{
		try {
			CommonSetting const common{
				std::stoi(GetAttributeValue(line, "x")),
				std::stoi(GetAttributeValue(line, "y")),
				std::stoi(GetAttributeValue(line, "width")) + 1,
				std::stoi(GetAttributeValue(line, "height")) + 1,
				GetAttributeValue(line, "image")};
		
		int32_t const x_index{ Viewer::GetMappingIndex(GetAttributeValue(line, "xname"), type_) };
		int32_t const y_index{ Viewer::GetMappingIndex(GetAttributeValue(line, "yname"), type_) };

		if (x_index == -1 || y_index == -1) {
			return false;
		}

		sticks_.push_back(StickSetting{
			common,
			std::stoi(GetAttributeValue(line, "xrange")),
			std::stoi(GetAttributeValue(line, "yrange")),
			x_index, 
			y_index
			});
			return true;
        } catch (std::exception const & /*unused*/) {
			return false;
		}
	}

	bool SkinSettings::CreateBackgroundSetting(std::string const &line)
	{
		try {
		background_.image = GetAttributeValue(line, "image");
			return true;
        } catch (std::exception const & /*unused*/) {
			return false;
		}
	}

	std::string SkinSettings::GetAttributeValue(std::string const &line,
						    std::string const &name)
	{
		size_t attribute_pos{line.find(name + "=")};
		if (attribute_pos == std::string::npos) {
			std::string const err{"Element " + line +
					" does not contain attribute " + name};
			throw std::exception(err.c_str());
		}

		size_t const attribute_start{line.find("\"", attribute_pos)};
		size_t const attribute_end{line.find("\"", attribute_start + 1)};

		if (attribute_start == std::string::npos ||
		    attribute_end == std::string::npos) {
			std::string const err{"Element " + line +
					      " is badly formatted"};
			throw std::exception(err.c_str());
		}

		return line.substr(attribute_start + 1,
				   attribute_end - attribute_start - 1);
	}

} // namespace slask_spy
