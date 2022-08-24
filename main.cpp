#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"
#include <fmt/core.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/exporters/ostream/span_exporter.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>

namespace sdktrace = opentelemetry::sdk::trace;

/*
 * An exporter is responsible for sending the telemetry data to a particular backend.
 * OpenTelemetry offers six tracing exporters out of the box:
 * - In-Memory Exporter: keeps the data in memory, useful for debugging.
 * - Jaeger Exporter: prepares and sends the collected telemetry data to a Jaeger backend via UDP and HTTP.
 * - Zipkin Exporter: prepares and sends the collected telemetry data to a Zipkin backend via the Zipkin APIs.
 * - Logging Exporter: saves the telemetry data into log streams.
 * - OpenTelemetry(otlp) Exporter: sends the data to the OpenTelemetry Collector using protobuf/gRPC or protobuf/HTTP.
 * - ETW Exporter: sends the telemetry data to Event Tracing for Windows (ETW).
 */
auto ostream_exporter = std::unique_ptr<sdktrace::SpanExporter>(
        new opentelemetry::exporter::trace::OStreamSpanExporter(std::cerr));

/*
 * Span Processor is initialised with an Exporter. Different Span Processors are offered by OpenTelemetry C++ SDK:
 * - SimpleSpanProcessor: immediately forwards ended spans to the exporter.
 * - BatchSpanProcessor: batches the ended spans and send them to exporter in bulk.
 * - MultiSpanProcessor: Allows multiple span processors to be active and configured at the same time.
 */
auto simple_processor = std::unique_ptr<sdktrace::SpanProcessor>(
        new sdktrace::SimpleSpanProcessor(std::move(ostream_exporter)));

/*
 * A Resource is an immutable representation of the entity producing telemetry as key-value pair.
 * The OpenTelemetry C++ SDK allow for creation of Resources and for associating them with telemetry.
 */
auto resource_attributes = opentelemetry::sdk::resource::ResourceAttributes
        {
                {"service.name",        "product_name"},     // Suggest this to be "product name"
                {"service.instance.id", "application_name"}  // Suggest this to be "application name"
        };
auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attributes);
auto received_attributes = resource.GetAttributes();

/*
 * Sampling is mechanism to control/reducing the number of samples of traces collected and sent to the backend.
 * OpenTelemetry C++ SDK offers four samplers out of the box:
 * - AlwaysOnSampler which samples every trace regardless of upstream sampling decisions.
 * - AlwaysOffSampler which doesn’t sample any trace, regardless of upstream sampling decisions.
 * - ParentBased which uses the parent span to make sampling decisions, if present.
 * - TraceIdRatioBased which samples a configurable percentage of traces.
 */
auto always_on_sampler = std::unique_ptr<sdktrace::AlwaysOnSampler>(new sdktrace::AlwaysOnSampler);

/*
 * SDK configuration are shared between TracerProvider and all it’s Tracer instances through TracerContext.
 */
//auto tracer_context = std::make_shared<sdktrace::TracerContext>
//        (std::move(simple_processor), resource, std::move(always_on_sampler));
// FIXME: std::make_shared not resolving in CLion

/*
 * TracerProvider instance holds the SDK configurations ( Span Processors, Samplers, Resource).
 * There is single global TracerProvider instance for an application, and it is created at the start of application.
 * There are two different mechanisms to create TraceProvider instance
 * - Using constructor which takes already created TracerContext shared object as parameter.
 * - Using constructor which takes SDK configurations as parameter.
 */
auto tracer_provider = std::make_shared<sdktrace::TracerProvider>
        (std::move(simple_processor), resource, std::move(always_on_sampler));
// FIXME: because tracer_context above isn't working, I can't use the first form.
//opentelemetry::trace::Provider::SetTracerProvider(tracer_provider);


int main(int argc, char **argv) {
//    auto provider = opentelemetry::trace::Provider::GetTracerProvider();
//    auto provider = opentelemetry::nostd::shared_ptr<sdktrace::TracerProvider>
//            (std::move(simple_processor), resource, std::move(always_on_sampler));
    auto tracer = tracer_provider->GetTracer("foo_library", "1.0.0");
    auto span = tracer->StartSpan("ApplicationLifetime");
    auto scope = tracer->WithActiveSpan(span);

    Poco::MD5Engine md5;

    {
        auto span2 = tracer->StartSpan("MD5");
        auto scope2 = tracer->WithActiveSpan(span2);
        Poco::DigestOutputStream ds(md5);
        ds << "abcdefghijklmnopqrstuvwxyz";
        ds.close();
        span2->End();
    }

    fmt::print("{}\n", Poco::DigestEngine::digestToHex(md5.digest()));
    span->End();

    return 0;
}
