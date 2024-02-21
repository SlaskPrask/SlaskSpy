#ifndef QT_GRAPHICS_WRAPPER_H
#define QT_GRAPHICS_WRAPPER_H

#include "graphics_wrapper.h"

#include <functional>
#include <memory>
#include <QGraphicsView>
#include <QObject>
#include <QTimer>
#include <QXmlStreamAttributes>

#include "qt_input_items.h"
#include "viewer.h"

class QTGraphicsWrapper : public GraphicsWrapper, public QObject {
public:
    QTGraphicsWrapper(QGraphicsView* graphics_view);
    ~QTGraphicsWrapper();
    void StartDispatchThread(std::function<void()> const& tick_callback) override;
    void Update() override;
    bool SetupScene(std::string_view skin_directory, Viewer* viewer) override;
    int32_t GetHeight() const override;
    int32_t GetWidth() const override;

private:

    QPixmap CreateImage(QXmlStreamAttributes const& attributes);
    bool SetBackground(QXmlStreamAttributes const& attributes);
    QtInputButton* CreateButton(QXmlStreamAttributes const& attributes);
    QtInputStick* CreateStick(QXmlStreamAttributes const& attributes);

    QGraphicsView* graphics_view_;
    QGraphicsScene* scene_;
    QTimer* dispatch_;
    QString skin_directory_;
    int32_t bg_width_;
    int32_t bg_height_;
};

#endif // QT_GRAPHICS_WRAPPER_H
