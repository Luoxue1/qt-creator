// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <qmldesignercorelib_exports.h>

#include <nanotrace/nanotracehr.h>

#pragma once

namespace QmlDesigner {
namespace Tracing {
#ifdef ENABLE_QMLDESIGNER_TRACING
using Enabled = std::true_type;
#else
using Enabled = std::false_type;
#endif

constexpr bool tracingIsEnabled()
{
#ifdef ENABLE_QMLDESIGNER_TRACING
    return NanotraceHR::isTracerActive();
#else
    return false;
#endif
}

using EventQueue = NanotraceHR::EventQueue<NanotraceHR::StringViewTraceEvent, Enabled>;
QMLDESIGNERCORE_EXPORT EventQueue &eventQueue();

} // namespace Tracing

namespace ModelTracing {
constexpr bool tracingIsEnabled()
{
#ifdef ENABLE_MODEL_TRACING
    return NanotraceHR::isTracerActive();
#else
    return false;
#endif
}

using Category = NanotraceHR::StringViewWithStringArgumentsCategory<tracingIsEnabled()>;
using ObjectTraceToken = Category::ObjectTokenType;
QMLDESIGNERCORE_EXPORT Category &category();

} // namespace ModelTracing
} // namespace QmlDesigner
