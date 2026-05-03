#pragma once

#include "utils/secure_buffer.h"

#include <variant>
#include <vector>
#include <QString>
#include <QByteArray>

/*db_id(primary key, plain) | created_at(plain) | updated_at(plain) | last_access_at(plain) | object_id(plain, random, 32 byte) | type (plain) | ns_salt(plain) | s_salt(plain) |
 * | iv + aes256gcm_Knsd(NCEK) | iv + aes256gcm_Ksd(SCEK) | iv + aes256gcm_NCEK(nonsecure data) | iv + aes256gcm_SCEK(secure data)
 *
 * iv(nonce) - random, always stored in db before encrypted object
 * object_id (plain) - random, generates on writing DataRecord to db
 * NCEK - random, generates on writing DataRecord to db, encrypted using Knsd
 * SCEK - random, generates on writing DataRecord to db, encrypted using Ksd
 * nonsecure data - json, encrypted using NCEK
 * secure data - json, encrypted using SCEK
 *
 */

struct DataRecord { //TODO: migrate to DataRecord_v1
    int id;
    QString url;
    QByteArray login;
    QByteArray password;
};

//new code
enum class RecordType : uint8_t {
    Credentials,
    BankCard,
    BillingAddress,
    Empty
};

struct DbRecord {
    uint64_t db_id;
    SecureBuffer object_id;
    RecordType type;

    uint64_t created_at;
    uint64_t updated_at;
    uint64_t last_access_at;

    SecureBuffer ns_salt;
    SecureBuffer s_salt;

    SecureBuffer iv_ncek;
    SecureBuffer encrypted_ncek;

    SecureBuffer iv_scek;
    SecureBuffer encrypted_scek;

    SecureBuffer iv_nonsecure;
    SecureBuffer encrypted_nonsecure;

    SecureBuffer iv_secure;
    SecureBuffer encrypted_secure;
};

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

struct DataRecord_v1 {
    uint64_t db_id = 0;
    SecureBuffer object_id;
    RecordType type = RecordType::Empty;

    uint64_t created_at = 0;
    uint64_t updated_at = 0;
    uint64_t last_access_at = 0;

    DataRecordVariant data = std::monostate();
};
