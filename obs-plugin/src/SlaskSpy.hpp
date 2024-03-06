#ifndef SLASK_SPY_HPP
#define SLASK_SPY_HPP

#include <obs-module.h>

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>

#include "com_ports.h"
#include "obs_graphics_wrapper.h"
#include "skin_settings.h"
#include "viewer.h"

class SlaskSpy {
public:
	static obs_source_info GetSourceInfo();
	static const char *GetSpyName(void *type_data);
	static void* CreateSpy(obs_data_t* settings, obs_source* source);
	static void DestroySpy(void *data);
	static uint32_t GetSpyWidth(void *data);
	static uint32_t GetSpyHeight(void *data);
	static obs_properties_t *GetSpyProperties(void *data);
	static void UpdateSpy(void *data, obs_data_t *settings);
	static void RenderSpy(void *data, gs_effect_t *effect);
	static gs_color_space GetSpyColorSpace(void *data, size_t count,
			 const enum gs_color_space *preferred_spaces);

	static bool OnSkinDirectoryChanged(obs_properties_t *properties,
					   obs_property_t *skin_dir_prop,
					   obs_data_t *settings);
	static bool OnControllerTypeChanged(obs_properties_t *properties,
					   obs_property_t *type,
					   obs_data_t *settings);
	static bool OnSkinChanged(obs_properties_t *properties,
					    obs_property_t *skin,
					    obs_data_t *settings);

	void TickSpy();
	~SlaskSpy();

private:
	SlaskSpy(obs_source_t *source);
	void Reset();
	
	obs_source_t *source_;

	int32_t com_port_;
	std::string skin_path_;
	std::string background_;
	slask_spy::SkinSettings *skin_settings_;
	slask_spy::OBSGraphicsWrapper *graphics_;
	slask_spy::Viewer *viewer_;
	com_ports::COMDevice *device_;
	std::thread *tick_thread_;
	std::atomic<bool> run_;
};

#endif // SLASK_SPY_HPP