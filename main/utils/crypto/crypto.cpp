#include "crypto.hpp"

#include <esp_log.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>
#include <mbedtls/x509_csr.h>

#include <cstring>

namespace {
struct PkContext {
    mbedtls_pk_context ctx;
    PkContext() { mbedtls_pk_init(&ctx); }
    ~PkContext() { mbedtls_pk_free(&ctx); }
};

struct EntropyContext {
    mbedtls_entropy_context ctx;
    EntropyContext() { mbedtls_entropy_init(&ctx); }
    ~EntropyContext() { mbedtls_entropy_free(&ctx); }
};

struct DrbgContext {
    mbedtls_ctr_drbg_context ctx;
    DrbgContext() { mbedtls_ctr_drbg_init(&ctx); }
    ~DrbgContext() { mbedtls_ctr_drbg_free(&ctx); }
};

struct CsrWriter {
    mbedtls_x509write_csr req;
    CsrWriter() { mbedtls_x509write_csr_init(&req); }
    ~CsrWriter() { mbedtls_x509write_csr_free(&req); }
};

constexpr const char* TAG = "Crypto";
}  // namespace

bool Crypto::generateRsaKeyAndCsr(const std::string& deviceId,
                                  std::string& outPrivateKeyPem,
                                  std::string& outCsrPem) {
    ESP_LOGI(TAG, "Generating RSA key and CSR...");

    const char* pers = "csr_gen";

    PkContext key;
    EntropyContext entropy;
    DrbgContext drbg;
    CsrWriter csr;

    ESP_LOGI(TAG, "Seeding DRBG...");
    int ret =
        mbedtls_ctr_drbg_seed(&drbg.ctx, mbedtls_entropy_func, &entropy.ctx,
                              (const unsigned char*)pers, strlen(pers));
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to seed DRBG: -0x%04X", -ret);
        return false;
    }

    ESP_LOGI(TAG, "Setting up PK context...");
    ret = mbedtls_pk_setup(&key.ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to setup PK context: -0x%04X", -ret);
        return false;
    }

    ESP_LOGI(TAG, "Generating RSA key...");
    ret = mbedtls_rsa_gen_key(mbedtls_pk_rsa(key.ctx), mbedtls_ctr_drbg_random,
                              &drbg.ctx, 2048, 65537);
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to generate RSA key: -0x%04X", -ret);
        return false;
    }

    ESP_LOGI(TAG, "Configuring CSR writer...");
    mbedtls_x509write_csr_set_md_alg(&csr.req, MBEDTLS_MD_SHA256);
    mbedtls_x509write_csr_set_key(&csr.req, &key.ctx);

    std::string subject = "CN=" + deviceId;
    ESP_LOGI(TAG, "Setting CSR subject: %s", subject.c_str());
    ret = mbedtls_x509write_csr_set_subject_name(&csr.req, subject.c_str());
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to set subject name: -0x%04X", -ret);
        return false;
    }

    ESP_LOGI(TAG, "Generating CSR PEM...");
    unsigned char csr_buf[2048];
    ret = mbedtls_x509write_csr_pem(&csr.req, csr_buf, sizeof(csr_buf),
                                    mbedtls_ctr_drbg_random, &drbg.ctx);
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to write CSR PEM: -0x%04X", -ret);
        return false;
    }
    outCsrPem = std::string((char*)csr_buf);
    ESP_LOGI(TAG, "CSR PEM generated successfully.");

    ESP_LOGI(TAG, "Writing private key PEM...");
    unsigned char key_buf[2048];
    ret = mbedtls_pk_write_key_pem(&key.ctx, key_buf, sizeof(key_buf));
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to write private key PEM: -0x%04X", -ret);
        return false;
    }
    outPrivateKeyPem = std::string((char*)key_buf);
    ESP_LOGI(TAG, "Private key PEM generated successfully.");

    ESP_LOGI(TAG, "RSA key and CSR generation complete.");
    return true;
}