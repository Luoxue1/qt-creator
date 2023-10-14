// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmldesignertracing.h"

namespace QmlDesigner {
namespace Tracing {

namespace {
using TraceFile = NanotraceHR::TraceFile<tracingIsEnabled()>;

TraceFile traceFile{"qml_designer.json"};

thread_local auto strinViewEventQueueData = NanotraceHR::makeEventQueueData<NanotraceHR::StringViewTraceEvent,
                                                                            10000>(traceFile);
thread_local NanotraceHR::EventQueue stringViewEventQueue_ = strinViewEventQueueData.createEventQueue();

thread_local auto stringViewWithStringArgumentsEventQueueData = NanotraceHR::
    makeEventQueueData<NanotraceHR::StringViewWithStringArgumentsTraceEvent, 1000>(traceFile);
thread_local NanotraceHR::EventQueue stringViewEventWithStringArgumentsQueue_ = stringViewWithStringArgumentsEventQueueData
                                                                                    .createEventQueue();
} // namespace

EventQueue &eventQueue()
{
    return stringViewEventQueue_;
}

} // namespace Tracing

namespace ModelTracing {
namespace {
using namespace NanotraceHR::Literals;

thread_local Category category_{"model"_t, Tracing::stringViewEventWithStringArgumentsQueue_};

} // namespace

Category &category()
{
    return category_;
}

} // namespace ModelTracing
} // namespace QmlDesigner
