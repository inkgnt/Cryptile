#pragma once

#include "utils/secure_buffer.h"

#include <variant>
#include <array>
#include <vector>

//TODO delete
#include <QString>
#include <QByteArray>

/*db_id(primary key, plain) | created_at(plain) | updated_at(plain) | last_access_at(plain) |
 * | object_id(plain, random, 32 byte) | type (plain, 1 byte) | ns_salt(plain, random, 32 bytes) | s_salt(plain, random, 32 bytes) |
 * | iv + aes256gcm_Knsd(NCEK) | iv + aes256gcm_Ksd(SCEK) | iv + aes256gcm_NCEK(nonsecure data) | iv + aes256gcm_SCEK(secure data)
 *
 * iv(nonce) - random, 12 bytes, iv is always stored in db before encrypted object
 * object_id (plain) - random, 32 bytes, generates on writing DataRecord to db
 * NCEK - random, 32 bytes, generates on writing DataRecord to db, encrypted using Knsd
 * SCEK - random, 32 bytes, generates on writing DataRecord to db, encrypted using Ksd
 * nonsecure data - json, encrypted using NCEK
 * secure data - json, encrypted using SCEK
 *
 * Knsd =
 *  = HKDF-SHA256 (
 * IKM = HMAC-SHA256-challenge(yubikey_key, SHA-256("vault-nonsecret-challenge-v1" + object_id))
 * salt = ns_salt
 * info = "nonsecret-kek-v1"
 * output length = 32 )
 *
 * Ksd =
 *  = HKDF-SHA256 (
 * IKM = HMAC-SHA256-challenge(yubikey_key, SHA-256("vault-secret-challenge-v1" + object_id))
 * salt = s_salt
 * info = "secret-kek-v1"
 * output length = 32 )
 */

struct DataRecord { //TODO: migrate to DataRecord_v1
    int id;
    QString url;
    QByteArray login;
    QByteArray password;
};

enum class TlvTag : uint8_t {
    //nonsecure base:
    title = 0x00,
    tags = 0x01,
    favorite = 0x02,

    //nonsecure credentials
    url = 0x03,

    //nonsecure bankcard:
    last_four_digits = 0x10,

    //nonsecure billing address & nonsecure bankcard:
    billing_address_id = 0x11,

    //secure credentials:
    login = 0x20,
    password = 0x21,
    notes = 0x22,

    //secure bankcard:
    number = 0x30,
    expiration = 0x31,
    cvv = 0x32,
    cardholder_name = 0x33,

    //secure billing address
    full_name = 0x40,
    country = 0x41,
    city = 0x42,
    address_line1 = 0x43,
    address_line2 = 0x44,
    state = 0x45,
    zip = 0x46,
    phone = 0x47,

    extended_tag = 0xFF
};

//actual db record
enum class RecordType : uint8_t {
    Credentials,
    BankCard,
    BillingAddress,
    Empty
};

struct DbRecordNonSecure {
    uint64_t db_id;
    SecureBuffer object_id;
    RecordType type;

    uint64_t created_at;
    uint64_t updated_at;
    uint64_t last_access_at;

    std::array<uint8_t, 32>  ns_salt;

    SecureBuffer iv_ncek;
    SecureBuffer encrypted_ncek;

    SecureBuffer iv_nonsecure;
    SecureBuffer encrypted_nonsecure;
};

struct DbRecordSecure {
    uint64_t db_id;

    SecureBuffer s_salt;

    SecureBuffer iv_scek;
    SecureBuffer encrypted_scek;

    SecureBuffer iv_secure;
    SecureBuffer encrypted_secure;
};

//service structs for interface view
namespace detail {

//nonsecure
struct NonSecureBase {
    SecureBuffer title;

    std::vector<SecureBuffer> tags;
    bool favorite = false;
};

struct NonSecureCredentials : NonSecureBase {
    SecureBuffer url;
};

struct NonSecureBankCard : NonSecureBase {
    SecureBuffer last_four_digits;
    SecureBuffer optional_billing_address_id;
};

struct NonSecureBillingAddress : NonSecureBase {
    SecureBuffer billing_address_id;
};

//secure
struct SecureCredentials {
    SecureBuffer login;
    SecureBuffer password;
    SecureBuffer notes;
};


struct SecureBankCard {
    SecureBuffer number;
    SecureBuffer expiration;
    SecureBuffer cvv;
    SecureBuffer cardholder_name;
};

struct SecureBillingAddress {
    SecureBuffer full_name;
    SecureBuffer country;
    SecureBuffer city;
    SecureBuffer address_line1;
    SecureBuffer address_line2;
    SecureBuffer state;
    SecureBuffer zip;
    SecureBuffer phone;
};

}

//general structs
struct Credentials {
    detail::NonSecureCredentials nonsecure_data;
    detail::SecureCredentials secure_data;
};

struct BankCard {
    detail::NonSecureBankCard nonsecure_data;
    detail::SecureBankCard secure_data;
};

struct BillingAddress {
    detail::NonSecureBillingAddress nonsecure_data;
    detail::SecureBillingAddress secure_data;
};

using DataRecordVariant = std::variant<
    Credentials,
    BankCard,
    BillingAddress,
    std::monostate
>;

//interface view
struct DataRecordView {
    uint64_t db_id = 0;
    SecureBuffer object_id;
    RecordType type = RecordType::Empty;

    uint64_t created_at = 0;
    uint64_t updated_at = 0;
    uint64_t last_access_at = 0;

    DataRecordVariant data = std::monostate();
};
