#include "qt_graphics_wrapper.h"

#include <cstdint>
#include <iostream>
#include <memory>

#include <QFile>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QString>
#include <QTimer>
#include <QXmlStreamReader>

#include "qt_input_items.h"
#include "viewer.h"

QTGraphicsWrapper::QTGraphicsWrapper(QGraphicsView *graphics_view)
	: graphics_view_{graphics_view},
	  scene_{nullptr},
	  dispatch_{nullptr},
	  skin_directory_{},
	  bg_width_{0},
	  bg_height_{0}
{
}

QTGraphicsWrapper::~QTGraphicsWrapper()
{
	if (dispatch_ != nullptr) {
		dispatch_->stop();
		delete dispatch_;
	}

	if (scene_ != nullptr) {
		for (int32_t i{0}; i < scene_->items().count(); ++i) {
			delete scene_->items().at(i);
		}
		delete scene_;
	}
}

void QTGraphicsWrapper::StartDispatchThread(
	std::function<void()> const &tick_callback)
{
	dispatch_ = new QTimer(this);
	connect(dispatch_, &QTimer::timeout, this,
		[tick_callback]() { tick_callback(); });
	dispatch_->start();
}

void QTGraphicsWrapper::Update()
{
	graphics_view_->update();
}

bool QTGraphicsWrapper::SetupScene(std::string_view skin_directory,
				   Viewer *viewer)
{
	skin_directory_ = skin_directory.data();
	QFile xml_file{skin_directory_ + "skin.xml"};

	bool background_init{false};

	if (xml_file.open(QIODevice::ReadOnly)) {
		QXmlStreamReader xml(&xml_file);

		while (!xml.isEndDocument()) {

			if (xml.isStartElement()) {
				if (xml.name().compare("background") == 0) {
					background_init =
						SetBackground(xml.attributes());
					if (!background_init) {
						return false;
					}
				} else if (background_init) {
					if (xml.name().compare("button") == 0) {
						QtInputButton *button =
							CreateButton(
								xml.attributes());
						if (button == nullptr) {
							return false;
						}

						if (!viewer->AssignButton(
							    xml.attributes()
								    .value("name")
								    .toString()
								    .toStdString(),
							    button)) {
							delete button;
							return false;
						}

						scene_->addItem(button);

					} else if (xml.name().compare(
							   "stick") == 0) {
						QtInputStick *stick = CreateStick(
							xml.attributes());
						if (stick == nullptr) {
							return false;
						}

						if (!viewer->AssignStick(
							    xml.attributes()
								    .value("xname")
								    .toString()
								    .toStdString(),
							    xml.attributes()
								    .value("yname")
								    .toString()
								    .toStdString(),
							    stick)) {
							delete stick;
							return false;
						}

						scene_->addItem(stick);
					}
				}
			}

			xml.readNext();
		}
	} else {
		std::cerr << "Error opening " << skin_directory_.toStdString()
			  << "skin.xml" << std::endl;
		return false;
	}

	graphics_view_->setScene(scene_);
	graphics_view_->setCacheMode(QGraphicsView::CacheBackground);
	return true;
}

int32_t QTGraphicsWrapper::GetHeight() const
{
	return bg_height_;
}

int32_t QTGraphicsWrapper::GetWidth() const
{
	return bg_width_;
}

QtInputButton *
QTGraphicsWrapper::CreateButton(QXmlStreamAttributes const &attributes)
{

	QPixmap image{CreateImage(attributes)};
	if (image.isNull()) {
		return nullptr;
	}

	if (!attributes.hasAttribute("name")) {
		std::cerr << "Missing 'name' attribute" << std::endl;
		return nullptr;
	}

	return new QtInputButton(image, attributes.value("x").toInt(),
				 attributes.value("y").toInt());
}

QtInputStick *
QTGraphicsWrapper::CreateStick(QXmlStreamAttributes const &attributes)
{
	QPixmap image{CreateImage(attributes)};

	if (image.isNull()) {
		return nullptr;
	}

	if (!attributes.hasAttribute("xrange") ||
	    !attributes.hasAttribute("yrange")) {
		std::cerr << "Missing 'xrange' and/or 'yrange' attributes"
			  << std::endl;
		return nullptr;
	}

	if (!attributes.hasAttribute("xname") ||
	    !attributes.hasAttribute("yname")) {
		std::cerr << "Missing 'xname' and/or 'yname' attributes"
			  << std::endl;
		return nullptr;
	}

	return new QtInputStick(image, attributes.value("x").toInt(),
				attributes.value("y").toInt(),
				attributes.value("xrange").toInt(),
				attributes.value("yrange").toInt(), 128.f,
				128.f);
}

QPixmap QTGraphicsWrapper::CreateImage(QXmlStreamAttributes const &attributes)
{
	QPixmap image{skin_directory_ + attributes.value("image").toString()};
	if (image.isNull()) {
		std::cerr << "No image found" << std::endl;
		return QPixmap{};
	}

	if (!attributes.hasAttribute("x") || !attributes.hasAttribute("y")) {
		std::cerr << "Missing 'x' and/or 'y' attributes" << std::endl;
		return QPixmap{};
	}

	const int32_t width{attributes.hasAttribute("width")
				    ? attributes.value("width").toInt() + 1
				    : image.width()};
	const int32_t height{attributes.hasAttribute("height")
				     ? attributes.value("height").toInt() + 1
				     : image.height()};
	return image.scaled(width, height);
}

bool QTGraphicsWrapper::SetBackground(QXmlStreamAttributes const &attributes)
{
	QImage img{skin_directory_ + attributes.value("image").toString()};
	if (img.isNull()) {
		std::cerr << "No background found" << std::endl;
		return false;
	}

	bg_width_ = img.width();
	bg_height_ = img.height();

	QBrush brush{img};
	scene_ = new QGraphicsScene();

	scene_->setSceneRect(0, 0, img.width(), img.height());
	scene_->setBackgroundBrush(brush);
	return true;
}
