/****************************************************************************
** Meta object code from reading C++ file 'imgstringlistmodel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "imgstringlistmodel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'imgstringlistmodel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ImgStringListModel_t {
    QByteArrayData data[1];
    char stringdata0[19];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ImgStringListModel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ImgStringListModel_t qt_meta_stringdata_ImgStringListModel = {
    {
QT_MOC_LITERAL(0, 0, 18) // "ImgStringListModel"

    },
    "ImgStringListModel"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ImgStringListModel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void ImgStringListModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject ImgStringListModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QStringListModel::staticMetaObject>(),
    qt_meta_stringdata_ImgStringListModel.data,
    qt_meta_data_ImgStringListModel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ImgStringListModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ImgStringListModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ImgStringListModel.stringdata0))
        return static_cast<void*>(this);
    return QStringListModel::qt_metacast(_clname);
}

int ImgStringListModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStringListModel::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
