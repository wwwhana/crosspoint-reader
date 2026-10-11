#pragma once

#include <cctype>
#include <cstdlib>
#include <cstring>

// Release version strings: upstream major.minor.patch, then this fork's
// "-x<mod>" build number (1.6.5-x2), then "-rc..." on release-candidate builds.
namespace firmware_version {

struct Parts {
  int major = 0;
  int minor = 0;
  int patch = 0;
  int mod = 0;  // 0 when there is no "-x<mod>"
};

inline Parts parse(const char* version) {
  if (version == nullptr) return Parts{};
  Parts parts;
  int* const fields[] = {&parts.major, &parts.minor, &parts.patch};
  const char* p = version;
  for (int i = 0; i < 3; ++i) {
    if (!std::isdigit(static_cast<unsigned char>(*p))) return Parts{};
    char* end = nullptr;
    *fields[i] = static_cast<int>(std::strtol(p, &end, 10));
    p = end;
    if (i < 2) {
      if (*p != '.') return Parts{};
      ++p;
    }
  }
  // Only directly after the upstream version, and only digits: dev builds
  // append board names such as "-x4pro", which must not read as a mod number.
  if (p[0] == '-' && p[1] == 'x' && std::isdigit(static_cast<unsigned char>(p[2]))) {
    char* end = nullptr;
    const long mod = std::strtol(p + 2, &end, 10);
    if (!std::isalpha(static_cast<unsigned char>(*end))) parts.mod = static_cast<int>(mod);
  }
  return parts;
}

// True when `latest` should replace `current`: a higher upstream version, a
// higher mod number on the same upstream version, or the final release of
// the same version when `current` is a release candidate.
inline bool isNewer(const char* latest, const char* current) {
  const Parts l = parse(latest);
  const Parts c = parse(current);
  if (l.major != c.major) return l.major > c.major;
  if (l.minor != c.minor) return l.minor > c.minor;
  if (l.patch != c.patch) return l.patch > c.patch;
  if (l.mod != c.mod) return l.mod > c.mod;
  return current != nullptr && std::strstr(current, "-rc") != nullptr;
}

}  // namespace firmware_version
