#include "lineformat.h"
#include <QStringList>
#include <QDebug>

LineFormat::LineFormat()
{

}

void LineFormat::operator=(const LineFormat&obj)
{
    this->separator_ = obj.separator_;
    this->normalized_coords_ = obj.normalized_coords_;
    this->line_fmt_ = obj.line_fmt_;
    this->Build();
}

LineFormat::LineFormat(const LineFormat &obj)
{
    *this = obj;
}


QString LineFormat::GetX1()
{
    auto r = (this->normalized_coords_) ? this->bbox_.x()/this->image_size_.width() : this->bbox_.x();
    return  QString::number(r);
}

QString LineFormat::GetX2()
{
    auto r = (this->normalized_coords_) ? this->bbox_.right()/this->image_size_.width() : this->bbox_.right();
    return  QString::number(r);
}
QString LineFormat::GetY1()
{
    auto r = (this->normalized_coords_) ? this->bbox_.y()/this->image_size_.height() : this->bbox_.y();
    return  QString::number(r);
}

QString LineFormat::GetY2()
{
    auto r = (this->normalized_coords_) ? this->bbox_.bottom()/this->image_size_.height() : this->bbox_.bottom();
    return  QString::number(r);
}

QString LineFormat::GetCX()
{
    auto r = (this->normalized_coords_) ? this->bbox_.center().x()/this->image_size_.width() : this->bbox_.center().x();
    return  QString::number(r);
}

QString LineFormat::GetCY()
{
    auto r = (this->normalized_coords_) ? this->bbox_.center().y()/this->image_size_.height() : this->bbox_.center().y();
    return  QString::number(r);
}

QString LineFormat::GetW()
{
    auto r = (this->normalized_coords_) ? this->bbox_.width()/this->image_size_.width() : this->bbox_.width();
    return  QString::number(r);
}

QString LineFormat::GetH()
{
    auto r = (this->normalized_coords_) ? this->bbox_.height()/this->image_size_.height() : this->bbox_.height();
    return  QString::number(r);
}

QString LineFormat::GetImgW()
{
    return  QString::number(this->image_size_.width());
}

QString LineFormat::GetImgH()
{
    return  QString::number(this->image_size_.height());
}

QString LineFormat::GetLabel()
{
     return  this->label_;
}
QString LineFormat::GetImageId()
{
    return this->image_id_;
}


void LineFormat::Reset(const QString fmt,
                       const bool normalized_coords,
                       const QString &sep)
{
    const QStringList elements = fmt.split(" ");
    normalized_coords_ = normalized_coords;
    separator_ = sep;
    line_fmt_ = fmt;
    Build();
}

void LineFormat::Build()
{
    line_parts_.clear();
    const auto elements = line_fmt_.split(" ");
    for (const auto &e : elements){
        const auto ee = e.trimmed();
        std::function<QString(void)> f=nullptr;
        if ( ee == "x1" )
            f = std::bind(&LineFormat::GetX1, this);
        else if ( ee == "x2" )
            f = std::bind(&LineFormat::GetX2, this);
        else if (ee=="y1")
            f = std::bind(&LineFormat::GetY1, this);
        else if (ee=="y2")
            f = std::bind(&LineFormat::GetY2, this);
        else if (ee=="w")
           f = std::bind(&LineFormat::GetW, this);
        else if (ee=="h")
            f = std::bind(&LineFormat::GetH, this);
        else if (ee=="img_w")
           f = std::bind(&LineFormat::GetImgW, this);
        else if (ee=="img_h")
           f = std::bind(&LineFormat::GetImgH, this);
        else if (ee=="cx")
            f = std::bind(&LineFormat::GetCX, this);
        else if (ee=="cy")
            f = std::bind(&LineFormat::GetCY, this);
        else if (ee=="id")
            f = std::bind(&LineFormat::GetImageId, this);
        else if (ee=="cls")
            f = std::bind(&LineFormat::GetLabel, this);
        if (f) line_parts_.push_back(f);
    }
}

QString LineFormat::GenLine(const QString &image_id,
                            const QSize &img_size,
                            const QString &label,
                            const QRectF &bbox)
{
    image_id_ = image_id;
    image_size_ = img_size;
    label_ = label;
    bbox_ = bbox;
    QString r="";
    auto iter = line_parts_.begin();
    while (iter != line_parts_.end()-1){
        r += (*iter)() + separator_;
        ++iter;
    }
    r+=(*iter)();
    return r;
}
