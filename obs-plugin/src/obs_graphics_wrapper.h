#ifndef OBS_GRAPHICS_WRAPPER_H
#define OBS_GRAPHICS_WRAPPER_H

#include <graphics/image-file.h>
#include <graphics/matrix4.h>
#include <graphics/vec3.h>
#include <atomic>
#include <string>
#include <unordered_map>

#include "graphics_wrapper.h"
#include "input_items.h"
#include "skin_settings.h"
#include "viewer.h"

namespace slask_spy {

enum class InputType {
	kButton,
	kStick
};

class GraphicsObject {
public:

	struct DrawParams {
		uint32_t x;
		uint32_t y;
		uint32_t width;
		uint32_t height;
	};

	GraphicsObject(CommonSetting const* common, gs_image_file4_t const* image, bool flipX = false, bool flipY = false)
		: kTranslation
		{static_cast<float>(
				       common->x +
				       (flipX ? common->width
					      : 0)),
			       static_cast<float>(common->y) + (flipY ? common->height : 0), 
					0.0f
		},
		kScaling{(flipX ? -1 : 1) * static_cast<float>(common->width) /
				image->image3.image2.image.cx,
			   (flipY ? -1 : 1) *
				   static_cast<float>(common->height) /
				image->image3.image2.image.cy,
			1.0f
		},
		kDrawRegion{ 0, 0,
			image->image3.image2.image.cx,
		    image->image3.image2.image.cy}
		{
		
	}

	virtual ~GraphicsObject() {}

	virtual uint32_t GetFlip() const { return 0;
	}

	virtual vec3 const* GetTranslation() const { 
		return &kTranslation;
	}
	virtual vec3 const *GetScaling() const { 
		return &kScaling;
	}

	virtual DrawParams const* GetDrawRegion() const { 
		return &kDrawRegion;
	}

	virtual bool IsHidden() const { 
		return false;
	}

protected:
	vec3 const kTranslation;
	vec3 const kScaling;
	DrawParams const kDrawRegion;
};

class OBSInputAnalog : public InputAnalog, public GraphicsObject {
public:
	OBSInputAnalog(AnalogSetting const *settings,
		       gs_image_file4_t const *image)
		: InputAnalog(settings),
		  GraphicsObject(settings, image, 
			  settings->direction == AnalogDirection::kLeft,
			  settings->direction == AnalogDirection::kUp),
		  draw_params_{kDrawRegion},
		  direction_{settings->direction},
		  reversed_{settings->reverse ? 1U : 0U},
		  flip_{static_cast<uint32_t>((settings->direction == AnalogDirection::kLeft
				 ? GS_FLIP_U
				 : 0) |
			(settings->direction == AnalogDirection::kUp ? GS_FLIP_V
								     : 0))}
	{}

	~OBSInputAnalog() = default;

	void Update(uint8_t analog) override{
		const float percentage = abs(reversed_ - analog / 255.f);

		switch (direction_) {
			case AnalogDirection::kLeft:
			draw_params_.x = (kDrawRegion.width -
						(static_cast<uint32_t>(
							kDrawRegion.width *
							percentage)));
				[[fallthrough]];
			case AnalogDirection::kRight: // Fallthrough
				draw_params_.width = (static_cast<uint32_t>(
					kDrawRegion.width * percentage));
				break;
			case AnalogDirection::kUp:
				draw_params_.y =
					(kDrawRegion.height -
					(static_cast<uint32_t>(
						kDrawRegion.height *
						percentage)));
				[[fallthrough]];
			case AnalogDirection::kDown:
				draw_params_.height =
					(static_cast<uint32_t>(
						kDrawRegion.height *
						percentage));
				break;
		} 
	}

	uint32_t GetFlip() const override { 
		return flip_;
	}

	DrawParams const* GetDrawRegion() const override {
		return &draw_params_;
	}

private:
	DrawParams draw_params_;
	AnalogDirection const direction_;
	const uint8_t reversed_;
	const uint32_t flip_;
};

class OBSInputStick : public InputStick, public GraphicsObject {
public:
	OBSInputStick(StickSetting const *settings,
		      gs_image_file4_t const *image, 
			  float x_divisor,
		      float y_divisor)
		: InputStick(settings, x_divisor, y_divisor),
		  GraphicsObject(settings, image),
		  translated_position_{kTranslation}
	{}

	~OBSInputStick() = default;

	void Update(int8_t x, int8_t y) override
	{
		translated_position_.x = kTranslation.x + x * kRangeX;
		translated_position_.y = kTranslation.y - y * kRangeY;
	}
	virtual vec3 const *GetTranslation() const { 
		return &translated_position_; 
	}

private:
	vec3 translated_position_;
};

class OBSInputButton : public InputButton, public GraphicsObject {
public:
	OBSInputButton(ButtonSetting const *settings,
		      gs_image_file4_t const *image)
		: InputButton(settings),
		  GraphicsObject(settings, image),
		  hidden_{true}
	{
	}

	~OBSInputButton() = default;

	void Update(bool pressed) override { 
		hidden_ = !pressed;
	}

	
	bool IsHidden() const override { return hidden_; }

private:
	std::atomic<bool> hidden_;
};



class OBSGraphicsWrapper : public GraphicsWrapper {
public:
	OBSGraphicsWrapper();
	~OBSGraphicsWrapper();

	void StartDispatchThread(std::function<void()> const &tick_callback) override;
	void Update() override;
	bool SetupScene(slask_spy::SkinSettings const *settings, Viewer *viewer,
			std::string const &background) override;
	int32_t GetWidth() const override;
	int32_t GetHeight() const override;
	void Render(gs_effect_t *effect);
	gs_image_file4_t const *GetBackground() const;


private:

	gs_image_file4_t const* GetImage(CommonSetting const *common,
				std::string_view skin_path);

	void LoadGraphicsStick(StickSetting const* settings, std::string_view skin_path);
	void LoadGraphicsButton(ButtonSetting const *settings,
				std::string_view skin_path);
	void LoadGraphicsAnalog(AnalogSetting const *settings,
				std::string_view skin_path);

	std::unordered_map<std::string, gs_image_file4_t*> graphics_;
	std::unordered_map<std::string, std::vector<GraphicsObject*>> objects_;

	std::string background_identifier_;
	Viewer *viewer_;
};
} // namespace slask_spy
#endif // OBS_GRAPHICS_WRAPPER_H
