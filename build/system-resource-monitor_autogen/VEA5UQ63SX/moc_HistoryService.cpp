/****************************************************************************
** Meta object code from reading C++ file 'HistoryService.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/services/HistoryService.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HistoryService.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN3srm14HistoryServiceE_t {};
} // unnamed namespace

template <> constexpr inline auto srm::HistoryService::qt_create_metaobjectdata<qt_meta_tag_ZN3srm14HistoryServiceE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "srm::HistoryService",
        "samplesReady",
        "",
        "QList<MetricSample>",
        "samples",
        "statsReady",
        "MetricKind",
        "kind",
        "MetricStats",
        "stats",
        "exportFinished",
        "ok",
        "path",
        "error",
        "errorOccurred"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'samplesReady'
        QtMocHelpers::SignalData<void(const QVector<MetricSample> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'statsReady'
        QtMocHelpers::SignalData<void(MetricKind, const MetricStats &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 }, { 0x80000000 | 8, 9 },
        }}),
        // Signal 'exportFinished'
        QtMocHelpers::SignalData<void(bool, const QString &, const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 11 }, { QMetaType::QString, 12 }, { QMetaType::QString, 13 },
        }}),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<HistoryService, qt_meta_tag_ZN3srm14HistoryServiceE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject srm::HistoryService::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3srm14HistoryServiceE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3srm14HistoryServiceE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3srm14HistoryServiceE_t>.metaTypes,
    nullptr
} };

void srm::HistoryService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<HistoryService *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->samplesReady((*reinterpret_cast<std::add_pointer_t<QList<MetricSample>>>(_a[1]))); break;
        case 1: _t->statsReady((*reinterpret_cast<std::add_pointer_t<MetricKind>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<MetricStats>>(_a[2]))); break;
        case 2: _t->exportFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 3: _t->errorOccurred((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (HistoryService::*)(const QVector<MetricSample> & )>(_a, &HistoryService::samplesReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (HistoryService::*)(MetricKind , const MetricStats & )>(_a, &HistoryService::statsReady, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (HistoryService::*)(bool , const QString & , const QString & )>(_a, &HistoryService::exportFinished, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (HistoryService::*)(const QString & )>(_a, &HistoryService::errorOccurred, 3))
            return;
    }
}

const QMetaObject *srm::HistoryService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *srm::HistoryService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3srm14HistoryServiceE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int srm::HistoryService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void srm::HistoryService::samplesReady(const QVector<MetricSample> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void srm::HistoryService::statsReady(MetricKind _t1, const MetricStats & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void srm::HistoryService::exportFinished(bool _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3);
}

// SIGNAL 3
void srm::HistoryService::errorOccurred(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
