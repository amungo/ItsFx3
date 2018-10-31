/****************************************************************************
** Meta object code from reading C++ file 'hwmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../hwmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hwmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_HWManager_t {
    QByteArrayData data[25];
    char stringdata[344];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_HWManager_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_HWManager_t qt_meta_stringdata_HWManager = {
    {
QT_MOC_LITERAL(0, 0, 9), // "HWManager"
QT_MOC_LITERAL(1, 10, 16), // "newDevicePointer"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 11), // "FX3DevIfce*"
QT_MOC_LITERAL(4, 40, 18), // "informInitHWStatus"
QT_MOC_LITERAL(5, 59, 19), // "informCloseHWStatus"
QT_MOC_LITERAL(6, 79, 19), // "informStartHWStatus"
QT_MOC_LITERAL(7, 99, 18), // "informStopHWStatus"
QT_MOC_LITERAL(8, 118, 15), // "informDebugInfo"
QT_MOC_LITERAL(9, 134, 20), // "fx3_dev_debug_info_t"
QT_MOC_LITERAL(10, 155, 12), // "initHardware"
QT_MOC_LITERAL(11, 168, 12), // "DriverType_t"
QT_MOC_LITERAL(12, 181, 7), // "drvType"
QT_MOC_LITERAL(13, 189, 11), // "const char*"
QT_MOC_LITERAL(14, 201, 13), // "imageFileName"
QT_MOC_LITERAL(15, 215, 23), // "additionalImageFileName"
QT_MOC_LITERAL(16, 239, 12), // "algoFileName"
QT_MOC_LITERAL(17, 252, 12), // "dataFileName"
QT_MOC_LITERAL(18, 265, 13), // "closeHardware"
QT_MOC_LITERAL(19, 279, 12), // "startStreams"
QT_MOC_LITERAL(20, 292, 11), // "stopStreams"
QT_MOC_LITERAL(21, 304, 9), // "setAttReg"
QT_MOC_LITERAL(22, 314, 8), // "uint32_t"
QT_MOC_LITERAL(23, 323, 7), // "reg_val"
QT_MOC_LITERAL(24, 331, 12) // "getDebugInfo"

    },
    "HWManager\0newDevicePointer\0\0FX3DevIfce*\0"
    "informInitHWStatus\0informCloseHWStatus\0"
    "informStartHWStatus\0informStopHWStatus\0"
    "informDebugInfo\0fx3_dev_debug_info_t\0"
    "initHardware\0DriverType_t\0drvType\0"
    "const char*\0imageFileName\0"
    "additionalImageFileName\0algoFileName\0"
    "dataFileName\0closeHardware\0startStreams\0"
    "stopStreams\0setAttReg\0uint32_t\0reg_val\0"
    "getDebugInfo"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HWManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x06 /* Public */,
       4,    2,   77,    2, 0x06 /* Public */,
       5,    2,   82,    2, 0x06 /* Public */,
       6,    2,   87,    2, 0x06 /* Public */,
       7,    2,   92,    2, 0x06 /* Public */,
       8,    2,   97,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    5,  102,    2, 0x0a /* Public */,
      18,    0,  113,    2, 0x0a /* Public */,
      19,    0,  114,    2, 0x0a /* Public */,
      20,    0,  115,    2, 0x0a /* Public */,
      21,    1,  116,    2, 0x0a /* Public */,
      24,    0,  119,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::Bool, 0x80000000 | 9,    2,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 11, 0x80000000 | 13, 0x80000000 | 13, 0x80000000 | 13, 0x80000000 | 13,   12,   14,   15,   16,   17,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 22,   23,
    QMetaType::Void,

       0        // eod
};

void HWManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        HWManager *_t = static_cast<HWManager *>(_o);
        switch (_id) {
        case 0: _t->newDevicePointer((*reinterpret_cast< FX3DevIfce*(*)>(_a[1]))); break;
        case 1: _t->informInitHWStatus((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->informCloseHWStatus((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 3: _t->informStartHWStatus((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 4: _t->informStopHWStatus((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: _t->informDebugInfo((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< fx3_dev_debug_info_t(*)>(_a[2]))); break;
        case 6: _t->initHardware((*reinterpret_cast< DriverType_t(*)>(_a[1])),(*reinterpret_cast< const char*(*)>(_a[2])),(*reinterpret_cast< const char*(*)>(_a[3])),(*reinterpret_cast< const char*(*)>(_a[4])),(*reinterpret_cast< const char*(*)>(_a[5]))); break;
        case 7: _t->closeHardware(); break;
        case 8: _t->startStreams(); break;
        case 9: _t->stopStreams(); break;
        case 10: _t->setAttReg((*reinterpret_cast< uint32_t(*)>(_a[1]))); break;
        case 11: _t->getDebugInfo(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (HWManager::*_t)(FX3DevIfce * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HWManager::newDevicePointer)) {
                *result = 0;
            }
        }
        {
            typedef void (HWManager::*_t)(bool , QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HWManager::informInitHWStatus)) {
                *result = 1;
            }
        }
        {
            typedef void (HWManager::*_t)(bool , QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HWManager::informCloseHWStatus)) {
                *result = 2;
            }
        }
        {
            typedef void (HWManager::*_t)(bool , QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HWManager::informStartHWStatus)) {
                *result = 3;
            }
        }
        {
            typedef void (HWManager::*_t)(bool , QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HWManager::informStopHWStatus)) {
                *result = 4;
            }
        }
        {
            typedef void (HWManager::*_t)(bool , fx3_dev_debug_info_t );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HWManager::informDebugInfo)) {
                *result = 5;
            }
        }
    }
}

const QMetaObject HWManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_HWManager.data,
      qt_meta_data_HWManager,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *HWManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HWManager::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_HWManager.stringdata))
        return static_cast<void*>(const_cast< HWManager*>(this));
    if (!strcmp(_clname, "DeviceDataHandlerIfce"))
        return static_cast< DeviceDataHandlerIfce*>(const_cast< HWManager*>(this));
    return QObject::qt_metacast(_clname);
}

int HWManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void HWManager::newDevicePointer(FX3DevIfce * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void HWManager::informInitHWStatus(bool _t1, QString _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void HWManager::informCloseHWStatus(bool _t1, QString _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void HWManager::informStartHWStatus(bool _t1, QString _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void HWManager::informStopHWStatus(bool _t1, QString _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void HWManager::informDebugInfo(bool _t1, fx3_dev_debug_info_t _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE
