#ifndef INPUTVIEWER_H
#define INPUTVIEWER_H

#include <cstdint>
#include <functional>
#include <optional>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QXmlStreamReader>

#include "com_ports.h"
#include "viewer.h"

class InputViewer
{
public:
    InputViewer(QGraphicsView* graphics_view, std::function<void(int32_t, int32_t)> size_callback, int32_t com_port, ViewerType type);
    ~InputViewer();
    bool Valid();

private:

    void SetupScene(std::function<void(int32_t, int32_t)> const& size_callback);
    bool SetBackground(QXmlStreamAttributes const& attributes, std::function<void(int32_t, int32_t)> const& size_callback);
    bool CreateButton(QXmlStreamAttributes const& attributes);
    bool CreateStick(QXmlStreamAttributes const& attributes);

    QPixmap CreateImage(QXmlStreamAttributes const& attributes);

    QGraphicsView* graphics_view_;
    QGraphicsScene* scene{nullptr};
    std::unique_ptr<Viewer> viewer_;
    com_ports::COMDevice input_;
};

#endif // INPUTVIEWER_H
