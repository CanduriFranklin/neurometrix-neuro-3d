// ============================================================================
// neuro-3d - Composition root
// ============================================================================
#include "application/use_cases/process_volume_use_case.hpp"
#include "infrastructure/adapters/config/config_loader.hpp"
#include "infrastructure/adapters/dicom/dicom_volume_loader_adapter.hpp"
#include "infrastructure/adapters/kafka/kafka_consumer_adapter.hpp"
#include "infrastructure/adapters/kafka/kafka_producer_adapter.hpp"
#include "infrastructure/adapters/logging/logger.hpp"
#include "infrastructure/adapters/metrics/metrics_server.hpp"
#include "infrastructure/adapters/postgres/postgres_repository_adapter.hpp"
#include "infrastructure/adapters/security/tls_context.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace {
std::atomic<bool> g_shutdown{false};
void on_signal(int) { g_shutdown.store(true, std::memory_order_release); }
} // anonymous namespace

int main() {
    using namespace neurometrix::core3d;

    auto config = infrastructure::config::load_config();
    if (!config) {
        std::fprintf(stderr, "[core-3d] FATAL: config error\n");
        return 1;
    }

    infrastructure::logging::init_logger(config->logging);
    auto log = infrastructure::logging::get();
    log->info("Core-3D starting up");

    auto tls = infrastructure::security::TLSContext::load(
        config->kafka.ssl_ca_location,
        config->kafka.ssl_cert_location,
        config->kafka.ssl_key_location);
    if (!tls) {
        log->critical("Failed to load TLS context");
        return 1;
    }

    infrastructure::metrics::MetricsServer metrics{config->metrics};
    if (!metrics.start()) {
        log->critical("Failed to start metrics server on port {}", config->metrics.port);
        return 1;
    }
    log->info("Metrics server started on port {}", config->metrics.port);

    infrastructure::postgres::PostgresRepositoryAdapter repo{config->postgres, *tls};
    if (!repo.start()) {
        log->critical("Postgres adapter failed");
        return 1;
    }
    log->info("Postgres adapter started");

    infrastructure::dicom::DicomVolumeLoaderAdapter dicom{};
    log->info("DICOM volume loader ready");

    infrastructure::kafka::KafkaProducerAdapter producer{config->kafka, *tls};
    if (!producer.start()) {
        log->critical("Kafka producer failed");
        return 1;
    }
    log->info("Kafka producer started");

    application::ProcessVolumeUseCase use_case{dicom, repo, producer};
    log->info("Use case wired");

    infrastructure::kafka::KafkaConsumerAdapter consumer{
        config->kafka, *tls, use_case};

    std::atomic<bool> consumer_finished{false};
    std::thread consumer_thread([&consumer, &consumer_finished]() {
        (void)consumer.run();
        consumer_finished.store(true, std::memory_order_release);
    });

    log->info("Core-3D is now consuming from Kafka");

    while (!g_shutdown.load(std::memory_order_acquire) &&
           !consumer_finished.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    log->info("Shutting down");
    consumer.stop();
    if (consumer_thread.joinable()) consumer_thread.join();

    producer.stop();
    repo.stop();
    metrics.stop();

    log->info("Core-3D shutdown complete");
    return 0;
}