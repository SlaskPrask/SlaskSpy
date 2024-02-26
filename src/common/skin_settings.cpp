#include "skin_settings.h"

#include <fstream>
#include <string_view>

namespace slask_spy {
	SkinSettings* SkinSettings::LoadSkinSettings(std::string_view skin_directory) {
		SkinSettings *skin{new SkinSettings(skin_directory)};
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

BackgroundSetting const &SkinSettings::GetBackgroundSettings() const {
	return background_;
}

std::string_view SkinSettings::GetSkinPath() const {
	return skin_path_;
}


	SkinSettings::SkinSettings(std::string_view skin_directory)
		: valid_{false},
		skin_path_{skin_directory},
		buttons_{},
		sticks_{},
		background_{}
	{
		std::ifstream xml_file{skin_path_ + "skin.xml"};
		
		if (xml_file.is_open()) {
			std::string line{};

			// First find skin element
			while (std::getline(xml_file, line)) {
				// Discard empty lines or the xml tag
				if (line.empty() ||
				    line.find("<?xml") != std::string::npos) {
					continue;
				}

				if (line.find("<skin") == std::string::npos) {
					return;
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

			buttons_.push_back(ButtonSetting{
				common,
				GetAttributeValue(line, "name"),
			});	
			return true;
        } catch (std::exception const & /*unused*/) {
			return false;
		}

		// TODO: Validate attributes

		
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
		
		sticks_.push_back(StickSetting{
			common,
			std::stoi(GetAttributeValue(line, "xrange")),
			std::stoi(GetAttributeValue(line, "yrange")),
			GetAttributeValue(line, "xname"),
			GetAttributeValue(line, "yname"),
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
