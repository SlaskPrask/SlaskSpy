#include "SlaskSpy.hpp"

#include <obs-module.h>
#include <plugin-support.h>
#include <unordered_map>
#include <vector>

#include "com_ports.h"
#include "logger.h"
#include "viewer.h"

namespace {
constexpr const char *kComPortName{"com_port"};
constexpr const char *kControllerType{"ctrl_type"};
constexpr const char *kSkinSelect{"skin"};
constexpr const char *kSkinDirectory{"skin_dir"};
constexpr uint32_t kBaudRate{115200};
static std::unordered_map<slask_spy::ViewerType, std::vector<slask_spy::SkinData>> available_skins;
}

gs_color_space
SlaskSpy::GetSpyColorSpace(void *data, size_t count,
			     const enum gs_color_space *preferred_spaces)
{
	SlaskSpy const *spy{static_cast<SlaskSpy *>(data)};
	if (spy->graphics_ != nullptr) {
		auto const *bg = spy->graphics_->GetBackground();
		if (bg != nullptr && bg->image3.image2.image.texture != nullptr) {
			return bg->space;
		}
	}

	return GS_CS_SRGB;
}

obs_source_info SlaskSpy::GetSourceInfo() {
	static obs_source_info info{0};
	if (info.create == nullptr) {
		info.id = "SlaskSpy";
		info.type = obs_source_type::OBS_SOURCE_TYPE_INPUT;
		info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB;
		info.get_name = GetSpyName;
		info.create = CreateSpy;
		info.destroy = DestroySpy;
		info.get_width = GetSpyWidth;
		info.get_height = GetSpyHeight;
		info.get_properties = GetSpyProperties;
		info.update = UpdateSpy;
		info.video_render = RenderSpy;
		info.video_get_color_space = GetSpyColorSpace;
	}

	return info;
}

bool SlaskSpy::OnSkinDirectoryChanged(obs_properties_t *properties,
				      obs_property_t *skin_dir_prop,
				      obs_data_t *settings) {
	std::string skin_dir{obs_data_get_string(settings, kSkinDirectory)};
	obs_property_t *type{obs_properties_get(properties, kControllerType)};
	if (type == nullptr) {
		Logger::Error("Could not get property with name %s",
			      kControllerType);
		return true;
	}
	obs_property_t *skin{obs_properties_get(properties, kSkinSelect)};
	if (skin == nullptr) {
		Logger::Error("Could not get property with name %s",
			      kSkinDirectory);
		return true;
	}

	if (skin_dir.empty()) {
		obs_property_set_enabled(type, false);
		return true;
	}

	Logger::Info("Skin dir: %s", skin_dir.c_str());

	obs_property_set_enabled(type, true);
	obs_property_list_clear(type);
	available_skins = slask_spy::SkinSettings::FetchSkins(skin_dir);
	obs_property_list_add_int(type, "None", 0);
	for (auto const &it : available_skins) {
		obs_property_list_add_int(
			type,
			slask_spy::Viewer::StringFromType(it.first).c_str(),
					  static_cast<int64_t>(it.first));
	}
	return true;
}

bool SlaskSpy::OnControllerTypeChanged(obs_properties_t *properties,
				       obs_property_t *type,
				       obs_data_t *settings) {
	slask_spy::ViewerType const kType{static_cast<slask_spy::ViewerType>(
		obs_data_get_int(settings, kControllerType))};
	
	obs_property_t *skins{obs_properties_get(properties, kSkinSelect)}; 
	obs_property_set_enabled(skins, true);
	obs_property_list_clear(skins);
	if (kType == slask_spy::ViewerType::kNull) {
		return true;
	}

	if (available_skins.empty()) {
		std::string const skin_dir{obs_data_get_string(settings, kSkinDirectory)};

		if (skin_dir.empty()) {
			obs_property_set_enabled(type, false);
			return true;
		}
		available_skins = slask_spy::SkinSettings::FetchSkins(skin_dir);
	}

	auto const &skin_data = available_skins.find(kType);
	if (skin_data == available_skins.end()) {
		return true;
	}

	for (auto const &it : skin_data->second) {
		obs_property_list_add_string(skins, it.name.c_str(), it.path.c_str());
	}
	return true;
}

obs_properties_t* SlaskSpy::GetSpyProperties(void* data) {
	obs_properties_t *properties{obs_properties_create()};
	
	obs_property_t *inputs{obs_properties_add_list(
		properties, 
		kComPortName, 
		"Select COM device",
		OBS_COMBO_TYPE_LIST, 
		OBS_COMBO_FORMAT_INT)};
	
	// add all names:
	auto const ports{com_ports::FetchCOMPorts()};
	for (int32_t i{0}; i < ports.size(); ++i) {
		obs_property_list_add_int(
			inputs, 
			ports.at(i).friendly_name.c_str(), 
			static_cast<int64_t>(ports.at(i).index)
		);
	}

	obs_property_t *skin{obs_properties_add_path(
		properties, kSkinDirectory, "Skin Directory",
		OBS_PATH_DIRECTORY, "", "")};
	obs_property_set_modified_callback(skin, OnSkinDirectoryChanged);

	obs_property_t *controller_type{obs_properties_add_list(
		properties, 
		kControllerType, 
		"Select controller type",
		OBS_COMBO_TYPE_RADIO,
		OBS_COMBO_FORMAT_INT
	)};
	obs_property_set_modified_callback(controller_type, OnControllerTypeChanged);

	obs_properties_add_list(
		properties,
		kSkinSelect, 
		"Select skin",
		OBS_COMBO_TYPE_LIST,
		OBS_COMBO_FORMAT_STRING
	);

	Logger::Info("properties created");

	return properties;
}
void SlaskSpy::TickSpy() {
	run_ = true;
	while (run_) {
		device_->Tick();
	}
}
 
void SlaskSpy::Reset() {
	if (tick_thread_ != nullptr) {
		run_ = false;
		tick_thread_->join();
		tick_thread_ = nullptr;
	}

	if (skin_settings_ != nullptr)
	{
		delete skin_settings_;
		skin_settings_ = nullptr;
	}

	if (viewer_ != nullptr) {
		delete viewer_;
		viewer_ = nullptr;
	}

	if (device_ != nullptr) {
		delete device_;
		device_ = nullptr;
	}

	if (graphics_ != nullptr) {
		delete graphics_;
		graphics_ = nullptr;
	}
}

void SlaskSpy::UpdateSpy(void* data, obs_data_t* settings) {
	SlaskSpy *spy{static_cast<SlaskSpy *>(data)};
	spy->Reset();

	spy->com_port_ = static_cast<int32_t>(obs_data_get_int(settings, kComPortName));
	spy->skin_path_ = obs_data_get_string(settings, kSkinSelect);
	spy->skin_path_ += "/";
	slask_spy::ViewerType const type{static_cast<slask_spy::ViewerType>(
		obs_data_get_int(settings, kControllerType))};

	if (type == slask_spy::ViewerType::kNull) {
		return;
	}

	spy->skin_settings_ =
		slask_spy::SkinSettings::LoadSkinSettings(spy->skin_path_, type);
	
	if (spy->skin_settings_ == nullptr) {
		obs_log(LOG_INFO, "SlaskSpy: Skin failed to load at path: %s", spy->skin_path_.c_str());
		return;
	}

	spy->viewer_ = slask_spy::Viewer::CreateViewer(type);
	spy->device_ = new com_ports::COMDevice(
		spy->com_port_, kBaudRate, spy->viewer_->GetDataBytesSize(),
		[spy](char *data) { spy->viewer_->SetIncommingData(data); },
		[](){});
			
	spy->graphics_ = new slask_spy::OBSGraphicsWrapper();
	spy->graphics_->SetupScene(spy->skin_settings_, spy->viewer_);
	spy->tick_thread_ =
		new std::thread([spy]() { spy->TickSpy(); });
}

void SlaskSpy::RenderSpy(void* data, gs_effect_t* effect) {
	SlaskSpy *spy{static_cast<SlaskSpy *>(data)};
	if (spy->graphics_ == nullptr) {
		return;
	}
	spy->graphics_->Render(effect);
}


const char* SlaskSpy::GetSpyName(void* type_data) {
	return "SlaskSpy";
}

void* SlaskSpy::CreateSpy(obs_data_t* settings, obs_source* source) {
	SlaskSpy *spy{new SlaskSpy(source)};
	UpdateSpy(spy, settings);
	return spy;
}

void SlaskSpy::DestroySpy(void* data) {
	SlaskSpy *spy{static_cast<SlaskSpy *>(data)};
	delete spy;
}

uint32_t SlaskSpy::GetSpyWidth(void* data) {
	SlaskSpy *spy{static_cast<SlaskSpy *>(data)};
	if (spy->graphics_ == nullptr) {
		Logger::Info("Returning 1");
		return 1;
	}
	return spy->graphics_->GetWidth();
}

uint32_t SlaskSpy::GetSpyHeight(void *data) {
	SlaskSpy *spy{static_cast<SlaskSpy *>(data)};
	if (spy->graphics_ == nullptr) {
		return 1;
	}
	return spy->graphics_->GetHeight();
}

SlaskSpy::SlaskSpy(obs_source_t *source) : 
	source_{source}, 
	com_port_{-1},
    skin_path_{""},
	skin_settings_{nullptr},
	graphics_{nullptr},
	viewer_{nullptr},
	device_{nullptr}, 
	tick_thread_{nullptr},
	run_{false}
{

}

SlaskSpy::~SlaskSpy() {
	Reset();
}