#include "skin_settings.h"

#include <sys/stat.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "logger.h"
#include "viewer.h"

namespace slask_spy {

bool SkinSettings::FetchSkins(
	std::string const &skins_directory,
	std::unordered_map<ViewerType, std::map<std::string, SkinData *>> &skins)
{
	// Clean up old skins
	{
		std::unordered_set<SkinData *> ptrs{};
		for (auto const &entry : skins) {
			for (auto const &pair : entry.second) {
				SkinData *skin_ptr{pair.second};
				if (ptrs.find(skin_ptr) == ptrs.end()) {
					delete skin_ptr;
					ptrs.insert(skin_ptr);
				}
			}
		}
	}
	skins.clear();

	{
		struct stat sb {
			0
		};
		if (stat(skins_directory.c_str(), &sb) != 0) {
			Logger::Info("skin_settings: Invalid path: %s",
				     skins_directory);
			return false;
		}
	}

	std::filesystem::path skin_dir{skins_directory};

	for (auto &path :
	     std::filesystem::recursive_directory_iterator(skin_dir)) {
		if (path.is_directory()) {
			bool added{false};
			auto const data = GetSkinData(path.path().string());
			auto const types = std::get<0>(data);

			for (auto type : types) {
				if (type != ViewerType::kNull) {
					auto it = skins.find(type);
					if (it == skins.end()) {
						skins[type] =
							std::map<std::string, SkinData*>{};
					}
					std::string const &name{
						std::get<1>(data)};
					
					if (skins[type].find(name) !=
					    skins[type].end()) {
						Logger::Warn(
							"Duplicate skin found: %s, discarding",
							name);
						continue;
					}
					skins[type][name] = std::get<2>(data);
					added = true;
				}
			}

			if (!added) {
				delete std::get<2>(data);
			}
		}
	}
	return skins.size() > 0;
}

std::tuple<std::vector<ViewerType>, std::string, SkinData *>
SkinSettings::GetSkinData(std::string_view skin_path)
{
	SkinData *skin{new SkinData()};
	std::string path{std::string(skin_path) + "/"};
	std::vector<ViewerType> type{};

	std::string const skin_xml_path{path + "skin.xml"};
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
				Logger::Error(
					"skin_settings: Skin %s is not a valid skin.xml",
					skin_xml_path.c_str());	
				delete skin;
				return std::make_tuple(
					std::vector<ViewerType>{}, path,
					nullptr);
			} 

			break;
		}

		std::string type_string{};

		try {
			type_string =
				GetAttributeValue(line, "type");
			skin->name = GetAttributeValue(line, "name");
		} catch (std::exception const& error) {
			Logger::Error("skin_settings: Attribute failure: %s",
				      error.what());
			delete skin;
			return std::make_tuple(std::vector<ViewerType>{}, path,
					       nullptr);
		}


		std::string single_type;
		ViewerType type_value;
		size_t pos_start{0};
		size_t pos_end{0};

		while ((pos_end = type_string.find(';', pos_start)) !=
		       std::string::npos) {
			single_type = type_string.substr(pos_start,
							 pos_end - pos_start);

			pos_start = pos_end + sizeof(';');
			type_value = Viewer::TypeFromString(single_type);

			if (type_value == ViewerType::kNull) {
				Logger::Warn(
					"skin_settings: Skin type %s is not implemented. Skin: %s",
					single_type.c_str(),
					skin_xml_path.c_str());
			} else {
				type.push_back(type_value);
			}
		}

		single_type = type_string.substr(pos_start);
		type_value = Viewer::TypeFromString(single_type);

		if (type_value == ViewerType::kNull) {
			Logger::Warn(
				"skin_settings: Skin type %s is not implemented. Skin: %s",
				single_type.c_str(), skin_xml_path.c_str());
		} else {
			type.push_back(type_value);
		}

		if (type.size() > 0) {
			while (std::getline(xml_file, line)) {
				// Discard empty lines or the xml tag
				if (line.empty()) {
					continue;
				}
				
				if (line.find("<background") != std::string::npos) {
					try {
						skin->backgrounds.push_back(							
							BackgroundData{
								GetAttributeValue(line, "name"),
								GetAttributeValue(line, "image"),
							}
						);
					} catch (std::exception const &error) {
						Logger::Error(
							"skin_settings: Attribute failure: %s",
							error.what());
					}
				} else if (line.find("<background") == std::string::npos) {
					if (skin->backgrounds.size() == 0) {
						delete skin;
						return std::make_tuple(
							std::vector<ViewerType>{},
							path,
							nullptr);
					} 
					break;
				}
			}
		}

	} else {
		delete skin;
		skin = nullptr;
		Logger::Warn("skin_settings: Could not open file %s",
			     skin_xml_path.c_str());
	}

	return std::make_tuple(type, path, skin);
}

SkinSettings *SkinSettings::LoadSkinSettings(std::string_view skin_directory,
					     ViewerType type)
{
	SkinSettings *skin{new SkinSettings(skin_directory, type)};
	if (!skin->valid_) {
		delete skin;
		skin = nullptr;
	}

	return skin;
}

std::vector<ButtonSetting> const &SkinSettings::GetButtonSettings() const
{
	return buttons_;
}

std::vector<StickSetting> const &SkinSettings::GetStickSettings() const
{
	return sticks_;
}

std::vector<AnalogSetting> const &SkinSettings::GetAnalogSettings() const
{
	return analogs_;
}

std::string_view SkinSettings::GetSkinPath() const
{
	return skin_path_;
}

SkinSettings::SkinSettings(std::string_view skin_directory, ViewerType type)
	: valid_{false},
	  skin_path_{skin_directory},
	  type_{type},
	  buttons_{},
	  sticks_{}
{
	std::ifstream xml_file{skin_path_ + "skin.xml"};

	if (xml_file.is_open()) {
		std::string line{};

		// First get to after <skin element
		while (std::getline(xml_file, line)) {
			// Quickly pass the past lines
			if (line.find("<background") != std::string::npos) {
				break;
			}
		}

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
			} else if (line.find("/>") != std::string::npos) {
				if (line.find("<") != std::string::npos) {
					break;
				}
				element_line += line;
			}
			

			if (element_line.find("<button") != std::string::npos) {
				if (!CreateButtonSetting(element_line)) {
					break;
				}
			} else if (element_line.find("<stick") !=
				   std::string::npos) {
				if (!CreateStickSetting(element_line)) {
					break;
				}
			} else if (element_line.find("<analog") !=
				   std::string::npos) {
				if (!CreateAnalogSetting(element_line)) {
					break;
				}
			} 
			element_line = "";
		}
	}
}

bool SkinSettings::CreateAnalogSetting(std::string const &line)
{
	try {
		CommonSetting const common{
			std::stoi(GetAttributeValue(line, "x")),
			std::stoi(GetAttributeValue(line, "y")),
			std::stoi(GetAttributeValue(line, "width")) + 1,
			std::stoi(GetAttributeValue(line, "height")) + 1,
			GetAttributeValue(line, "image")};

		int32_t const index{Viewer::GetMappingIndex(
			GetAttributeValue(line, "name"), type_)};

		if (index == -1) {
			return false;
		}

		static std::unordered_map<std::string_view, AnalogDirection> const
			kAnalogDirection{{"right", AnalogDirection::kRight},
					 {"left", AnalogDirection::kLeft},
					 {"down", AnalogDirection::kDown},
					 {"up", AnalogDirection::kUp}};

		auto const &analog_dir = GetAttributeValue(line, "direction");
		auto const &analog_it = kAnalogDirection.find(analog_dir);
		if (analog_it == kAnalogDirection.end()) {
			Logger::Error(
				"skin_settings: Invalid analog direction: %s",
				analog_dir.c_str());
			return false;
		};
		static std::unordered_map<std::string_view, bool> const
			kReverse{{"true", true}, {"false", false}};
		auto const &reverse = GetAttributeValue(line, "reverse");
		auto const &reverse_it = kReverse.find(reverse);
		if (reverse_it == kReverse.end()) {
			Logger::Error(
				"skin_settings: Invalid reverse value: %s",
				reverse.c_str());
			return false;
		}

		analogs_.push_back(AnalogSetting{
			common, index, analog_it->second, reverse_it->second});
		return true;
	} catch (std::exception const & error) {
		Logger::Error("skin_settings: Analog error: %s", error.what());
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
			std::stoi(GetAttributeValue(line, "height")) + 1,
			GetAttributeValue(line, "image")};

		int32_t const index{Viewer::GetMappingIndex(
			GetAttributeValue(line, "name"), type_)};

		if (index == -1) {
			return false;
		}

		buttons_.push_back(ButtonSetting{common, index});
		return true;
	} catch (std::exception const & error) {
		Logger::Error("skin_settings: Button error: %s", error.what());
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

		int32_t const x_index{Viewer::GetMappingIndex(
			GetAttributeValue(line, "xname"), type_)};
		int32_t const y_index{Viewer::GetMappingIndex(
			GetAttributeValue(line, "yname"), type_)};

		if (x_index == -1 || y_index == -1) {
			return false;
		}

		sticks_.push_back(StickSetting{
			common, std::stoi(GetAttributeValue(line, "xrange")),
			std::stoi(GetAttributeValue(line, "yrange")), x_index,
			y_index});
		return true;
	} catch (std::exception const & error) {
		Logger::Error("skin_settings: Stick error: %s", error.what());
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
