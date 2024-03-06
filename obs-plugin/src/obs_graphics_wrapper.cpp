#include "obs_graphics_wrapper.h"

#include <graphics/image-file.h>
#include <graphics/matrix4.h>
#include <obs-module.h>

#include "logger.h"

namespace slask_spy {

OBSGraphicsWrapper::OBSGraphicsWrapper() : 
	graphics_{}, 
	objects_ {},
	background_identifier_{},
	  viewer_{nullptr}
{
}

OBSGraphicsWrapper::~OBSGraphicsWrapper() {
	obs_enter_graphics();
	for (auto &it : graphics_) {
		gs_image_file4_free(it.second);
		delete it.second;
	}
	obs_leave_graphics();
	for (auto &it : objects_) {
		for (auto &obj : it.second) {
			delete obj;
		}
	}
}

void OBSGraphicsWrapper::StartDispatchThread(
	std::function<void()> const &tick_callback)
{
}

void OBSGraphicsWrapper::Render(gs_effect_t* effect) {
	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(true);
	
	gs_blend_state_push();
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);

	gs_image_file *const image =
		&graphics_.at(background_identifier_)->image3.image2.image;
	gs_texture_t *const texture{image->texture};

	gs_eparam_t *const param = gs_effect_get_param_by_name(effect, "image");
	gs_effect_set_texture_srgb(param, texture);
	gs_draw_sprite(texture, 0, image->cx, image->cy);

	for (auto const& it : objects_) {
		gs_image_file *const image =
			&graphics_.at(it.first)
				 ->image3.image2.image;
		gs_texture_t *const texture{image->texture};
		gs_effect_set_texture_srgb(param, texture);
		
		for (auto const &obj : it.second) {
			if (obj->IsHidden()) {
				continue;
			}
			GraphicsObject::DrawParams const* draw_region = obj->GetDrawRegion();

			gs_matrix_push();
			gs_matrix_translate(obj->GetTranslation());
			gs_matrix_scale(obj->GetScaling());
			gs_draw_sprite_subregion(texture, 
				obj->GetFlip(), 
				draw_region->x, 
				draw_region->y,
				draw_region->width, 
				draw_region->height
			);
			gs_matrix_pop();
		}
	}	

	gs_blend_state_pop();
	gs_enable_framebuffer_srgb(previous);
}

gs_image_file4_t const* OBSGraphicsWrapper::GetBackground() const {
	auto const &it = graphics_.find(background_identifier_);
	if (it != graphics_.end()) {
		return graphics_.at(background_identifier_);
	}
	return nullptr;
}

void OBSGraphicsWrapper::Update() {
	
}

bool OBSGraphicsWrapper::SetupScene(slask_spy::SkinSettings const *settings,
				    Viewer *viewer, std::string const& background)
{
	viewer_ = viewer;
	background_identifier_ = background;
	graphics_[background] =
		new gs_image_file4_t();
	std::string const path{};
	gs_image_file4_init(
		graphics_[background],
			    (std::string(settings->GetSkinPath()) +
			     background)
			.c_str(),
		GS_IMAGE_ALPHA_PREMULTIPLY_SRGB);

	auto const &analogs = settings->GetAnalogSettings();
	for (auto const &it : analogs) {
		LoadGraphicsAnalog(&it, settings->GetSkinPath());
	}

	auto const &sticks = settings->GetStickSettings();
	for (auto const &it : sticks) {
		LoadGraphicsStick(&it,
				   settings->GetSkinPath());
	}

	auto const &buttons = settings->GetButtonSettings();
	for (auto const &it : buttons) {
		LoadGraphicsButton(&it, settings->GetSkinPath());
	}


	bool result{true};
	obs_enter_graphics();
	for (auto &it : graphics_) {
		gs_image_file4_init_texture(it.second);
		// Validate textures
		if (!it.second->image3.image2.image.loaded) {
			result = false;
			Logger::Warn(
				"obs_graphics_wrapper: Couldn't load texture: %s, full path: %s",
				it.first.c_str(),
				(std::string(std::string(settings->GetSkinPath()) +
					it.first).c_str()));
			break;
		}
	}
	obs_leave_graphics();


	return result;
}

void OBSGraphicsWrapper::LoadGraphicsStick(StickSetting const *settings,
					   std::string_view skin_path)
{
	auto stick = new OBSInputStick(
		settings, 
		GetImage(settings, skin_path),
		128.f,
		128.f
	);

	viewer_->AssignStick(stick);
	objects_[settings->image].push_back(stick);
}

void OBSGraphicsWrapper::LoadGraphicsButton(ButtonSetting const *settings,
					    std::string_view skin_path)
{
	auto button = new OBSInputButton(
		settings, GetImage(settings, skin_path));
	
	viewer_->AssignButton(button);
	objects_[settings->image].push_back(button);
}

void OBSGraphicsWrapper::LoadGraphicsAnalog(AnalogSetting const *settings,
					    std::string_view skin_path)
{
	auto analog =
		new OBSInputAnalog(settings, GetImage(settings, skin_path));

	viewer_->AssignAnalog(analog);
	objects_[settings->image].push_back(analog);
}

gs_image_file4_t const *
OBSGraphicsWrapper::GetImage(
					    CommonSetting const *common,
					    std::string_view skin_path)
{
	gs_image_file4_t *image{nullptr};

	if (graphics_.find(common->image) == graphics_.end())
	{
		std::string const path{std::string(skin_path) + common->image};
		image = new gs_image_file4_t();
		gs_image_file4_init(image, path.c_str(),
				    GS_IMAGE_ALPHA_PREMULTIPLY_SRGB);
		graphics_[common->image] = image;
		objects_[common->image] = std::vector<GraphicsObject*>();
	} else {
		image = graphics_[common->image];
	}

	return image;
}

int32_t OBSGraphicsWrapper::GetWidth() const {
	auto const &it = graphics_.find(background_identifier_);
	if (it != graphics_.end()) {
		return graphics_.at(background_identifier_)->image3.image2.image.cx;
	}
	return 1;

}

int32_t OBSGraphicsWrapper::GetHeight() const {
	auto const &it = graphics_.find(background_identifier_);
	if (it != graphics_.end()) {
		return graphics_.at(background_identifier_)
			->image3.image2.image.cy;
	}
	return 1;
}

} // namespace slask_spy
