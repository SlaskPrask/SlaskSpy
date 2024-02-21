#include "../include/inputviewer.h"
#include "qevent.h"


#include <functional>
#include <iostream>
#include <QFile>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QString>
#include <QXmlStreamReader>

#include "com_ports.h"

#include "input_items.h"
#include "viewer.h"
#include "n64/n64_viewer.h"

namespace {
const QString kSkinPath{"../InputSkins/Tron-Style/"};
constexpr uint32_t kBaudRate{115200};
std::unique_ptr<Viewer> GetViewer(ViewerType type) {
    switch (type) {
    case ViewerType::kN64:
        return std::make_unique<N64Viewer>();
    default:
        return nullptr;
    }
}
}

InputViewer::InputViewer(QGraphicsView* graphics_view, std::function<void(int32_t, int32_t)> size_callback, int32_t com_port, ViewerType type) :
    graphics_view_{graphics_view},
    viewer_{GetViewer(type)},
    input_{com_ports::COMDevice(com_port, kBaudRate, viewer_.get(), graphics_view_)}
{

    if (!input_.Valid()) {
        return;
    }

    SetupScene(size_callback);

    graphics_view_->setScene(scene);
    graphics_view_->setCacheMode(QGraphicsView::CacheBackground);

    input_.StartReading();
}

InputViewer::~InputViewer() {
    if (scene != nullptr) {
        for (int32_t i {0}; i < scene->items().count(); ++i) {
            delete scene->items().at(i);
        }
        scene->clear();
        delete scene;
    }
}

bool InputViewer::Valid() {
    return input_.Valid();
}

void InputViewer::SetupScene(std::function<void(int32_t, int32_t)> const& size_callback) {
    QFile xml_file{kSkinPath + "skin.xml"};
    bool background_init{false};

    if (xml_file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader xml(&xml_file);

        while (!xml.isEndDocument()) {

            if (xml.isStartElement())
            {
                if (xml.name().compare("background") == 0) {
                    background_init = SetBackground(xml.attributes(), size_callback);
                    if (!background_init) {
                        break;
                    }
                }
                else if (background_init) {
                    if (xml.name().compare("button") == 0) {
                        CreateButton(xml.attributes());
                    } else if (xml.name().compare("stick") == 0) {
                        CreateStick(xml.attributes());
                    }
                }
            }

            xml.readNext();
        }
    }
}

bool InputViewer::CreateButton(QXmlStreamAttributes const& attributes) {

    QPixmap image{CreateImage(attributes)};
    if (image.isNull()) {
        return false;
    }

    if (!attributes.hasAttribute("name")) {
        std::cerr << "Missing 'name' attribute" << std::endl;
        return false;
    }

    InputButton* button{new InputButton(image, attributes.value("x").toInt(), attributes.value("y").toInt())};

    if (!viewer_->AssignButton(attributes.value("name").toString().toStdString(), button)) {
        delete button;
        return false;
    }

    scene->addItem(button);

    return true;
    // "name" is unused
}

bool InputViewer::CreateStick(QXmlStreamAttributes const& attributes) {
    QPixmap image{CreateImage(attributes)};

    if (image.isNull()) {
        return false;
    }

    if (!attributes.hasAttribute("xrange") || !attributes.hasAttribute("yrange")) {
        std::cerr << "Missing 'xrange' and/or 'yrange' attributes" << std::endl;
        return false;
    }

    if (!attributes.hasAttribute("xname") || !attributes.hasAttribute("yname")) {
        std::cerr << "Missing 'xname' and/or 'yname' attributes" << std::endl;
        return false;
    }

    InputStick* stick{new InputStick(image, attributes.value("x").toInt(), attributes.value("y").toInt(), attributes.value("xrange").toInt(), attributes.value("yrange").toInt())};
    if (!viewer_->AssignStick(attributes.value("xname").toString().toStdString(), attributes.value("yname").toString().toStdString(), stick)) {
        delete stick;
        return false;
    }

    scene->addItem(stick);

    return true;
}

QPixmap InputViewer::CreateImage(QXmlStreamAttributes const& attributes) {
    QPixmap image{kSkinPath + attributes.value("image").toString()};
    if (image.isNull()) {
        std::cerr << "No image found" << std::endl;
        return QPixmap{};
    }

    if (!attributes.hasAttribute("x") || !attributes.hasAttribute("y")) {
        std::cerr << "Missing 'x' and/or 'y' attributes" << std::endl;
        return QPixmap{};
    }

    const int32_t width{attributes.hasAttribute("width") ? attributes.value("width").toInt() + 1 : image.width()};
    const int32_t height{attributes.hasAttribute("height") ? attributes.value("height").toInt() + 1 : image.height()};
    return image.scaled(width, height);
}

bool InputViewer::SetBackground(QXmlStreamAttributes const& attributes, std::function<void(int32_t, int32_t)> const& size_callback) {
    QImage img{kSkinPath + attributes.value("image").toString()};
    if (img.isNull()) {
        std::cerr << "No background found" << std::endl;
        return false;
    }

    size_callback(img.width(), img.height());

    QBrush brush{img};
    scene = new QGraphicsScene();

    scene->setSceneRect(0,0, img.width(), img.height());
    scene->setBackgroundBrush(brush);
    return true;
}
