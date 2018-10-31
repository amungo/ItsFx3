/****************************************************************************
** Meta object code from reading C++ file 'gpscorrform.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../gpscorrform.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gpscorrform.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_GPSCorrForm_t {
    QByteArrayData data[18];
    char stringdata[175];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GPSCorrForm_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GPSCorrForm_t qt_meta_stringdata_GPSCorrForm = {
    {
QT_MOC_LITERAL(0, 0, 11), // "GPSCorrForm"
QT_MOC_LITERAL(1, 12, 7), // "satInfo"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 3), // "prn"
QT_MOC_LITERAL(4, 25, 4), // "corr"
QT_MOC_LITERAL(5, 30, 5), // "shift"
QT_MOC_LITERAL(6, 36, 4), // "freq"
QT_MOC_LITERAL(7, 41, 10), // "is_visible"
QT_MOC_LITERAL(8, 52, 10), // "satChanged"
QT_MOC_LITERAL(9, 63, 12), // "cellSelected"
QT_MOC_LITERAL(10, 76, 7), // "RecFile"
QT_MOC_LITERAL(11, 84, 10), // "ChooseFile"
QT_MOC_LITERAL(12, 95, 14), // "RefreshPressed"
QT_MOC_LITERAL(13, 110, 15), // "gnssTypeChanged"
QT_MOC_LITERAL(14, 126, 15), // "prnCheckUncheck"
QT_MOC_LITERAL(15, 142, 8), // "checkAll"
QT_MOC_LITERAL(16, 151, 10), // "uncheckAll"
QT_MOC_LITERAL(17, 162, 12) // "uncheckInVis"

    },
    "GPSCorrForm\0satInfo\0\0prn\0corr\0shift\0"
    "freq\0is_visible\0satChanged\0cellSelected\0"
    "RecFile\0ChooseFile\0RefreshPressed\0"
    "gnssTypeChanged\0prnCheckUncheck\0"
    "checkAll\0uncheckAll\0uncheckInVis"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GPSCorrForm[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    5,   69,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    5,   80,    2, 0x08 /* Private */,
       9,    2,   91,    2, 0x08 /* Private */,
      10,    1,   96,    2, 0x08 /* Private */,
      11,    1,   99,    2, 0x08 /* Private */,
      12,    1,  102,    2, 0x08 /* Private */,
      13,    1,  105,    2, 0x08 /* Private */,
      14,    1,  108,    2, 0x08 /* Private */,
      15,    1,  111,    2, 0x08 /* Private */,
      16,    1,  114,    2, 0x08 /* Private */,
      17,    1,  117,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Float, QMetaType::Int, QMetaType::Double, QMetaType::Bool,    3,    4,    5,    6,    7,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Float, QMetaType::Int, QMetaType::Double, QMetaType::Bool,    3,    4,    5,    6,    7,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,

       0        // eod
};

void GPSCorrForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        GPSCorrForm *_t = static_cast<GPSCorrForm *>(_o);
        switch (_id) {
        case 0: _t->satInfo((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 1: _t->satChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 2: _t->cellSelected((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->RecFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->ChooseFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->RefreshPressed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->gnssTypeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->prnCheckUncheck((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->checkAll((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->uncheckAll((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->uncheckInVis((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (GPSCorrForm::*_t)(int , float , int , double , bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&GPSCorrForm::satInfo)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject GPSCorrForm::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_GPSCorrForm.data,
      qt_meta_data_GPSCorrForm,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *GPSCorrForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GPSCorrForm::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_GPSCorrForm.stringdata))
        return static_cast<void*>(const_cast< GPSCorrForm*>(this));
    if (!strcmp(_clname, "StreamDataHandler"))
        return static_cast< StreamDataHandler*>(const_cast< GPSCorrForm*>(this));
    if (!strcmp(_clname, "ChunkDumpCallbackIfce"))
        return static_cast< ChunkDumpCallbackIfce*>(const_cast< GPSCorrForm*>(this));
    return QWidget::qt_metacast(_clname);
}

int GPSCorrForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void GPSCorrForm::satInfo(int _t1, float _t2, int _t3, double _t4, bool _t5)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
