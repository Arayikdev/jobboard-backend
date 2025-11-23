#include "models/Application.hpp"
#include <cassert>
#include <iostream>

int main() {
  // Test case 1: Valid JSON
  json validJson = {{"jobId", "123"},
                    {"userId", "456"},
                    {"companyId", "789"},
                    {"message", "Hello"},
                    {"resumeUrl", "http://example.com/resume.pdf"},
                    {"status", "pending"}};

  try {
    Application app(validJson);
    std::cout << "Test 1 Passed: Valid JSON accepted" << std::endl;

    json outputJson = app.toJson();
    assert(outputJson["jobId"] == "123");
    assert(outputJson["userId"] == "456");
    std::cout << "Test 2 Passed: JSON serialization works" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "Test 1 Failed: " << e.what() << std::endl;
    return 1;
  }

  // Test case 2: Invalid JSON (missing field)
  json invalidJson = {{"jobId", "123"},
                      // Missing userId
                      {"companyId", "789"},
                      {"message", "Hello"},
                      {"resumeUrl", "http://example.com/resume.pdf"},
                      {"status", "pending"}};

  try {
    Application app(invalidJson);
    std::cerr << "Test 3 Failed: Invalid JSON should have thrown exception"
              << std::endl;
    return 1;
  } catch (const std::runtime_error &e) {
    std::cout << "Test 3 Passed: Invalid JSON threw exception as expected"
              << std::endl;
  }

  return 0;
}
