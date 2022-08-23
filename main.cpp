#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"
#include <fmt/core.h>

int main(int argc, char **argv) {
    Poco::MD5Engine md5;
    Poco::DigestOutputStream ds(md5);
    ds << "abcdefghijklmnopqrstuvwxyz";
    ds.close();
    fmt::print("{}\n", Poco::DigestEngine::digestToHex(md5.digest()));
    return 0;
}
