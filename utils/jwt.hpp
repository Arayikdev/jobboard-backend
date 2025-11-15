#pragma once
#include <string>
#include <chrono>
#include "../third_party/jwt-cpp/include/jwt-cpp/jwt.h"

class JWT
{
public:
    static std::string createToken(const std::string &userId, const std::string &role)
    {
        auto token = jwt::create()
                         .set_issuer("cpp-backend")
                         .set_type("JWS")
                         .set_audience("users")
                         .set_payload_claim("userId", jwt::claim(userId))
                         .set_payload_claim("role", jwt::claim(role))
                         .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(1))
                         .sign(jwt::algorithm::hs256{"SECRET_KEY"});
        return token;
    }

    static jwt::decoded_jwt<jwt::traits::kazuho_picojson> verify(const std::string &token)
    {
        auto decoded = jwt::decode(token);

        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{"SECRET_KEY"})
            .with_issuer("cpp-backend")
            .verify(decoded);

        return decoded;
    }
};
